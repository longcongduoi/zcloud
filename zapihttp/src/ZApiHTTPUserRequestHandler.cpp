/*
 * File:   ZApiHTTPUserRequestHandler.cpp
 * Author: huuhoa
 *
 * Created on November 2, 2015, 4:00 PM
 */

#include <stdint.h>
#include <sstream>

#include <Poco/Util/Application.h>
#include <Poco/Dynamic/Struct.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/NumberParser.h>

#include <zuser/ZUserInfo.h>
#include <zuser/ZUserManager.h>
#include <zcommon/ErrorCode.h>
#include <zcommon/ZServiceLocator.h>
#include <zcommon/ZEmailService.h>
#include <zsession/ZSessionService.h>
#include <zworker/ZWorker.h>
#include <Poco/RegularExpression.h>
#include "ZApiHTTPUserRequestHandler.h"

using namespace Poco::JSON;
using namespace Poco::Dynamic;
using Poco::DynamicStruct;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Util::Application;

void ReportError::reportTo(HttpApiError error, Poco::JSON::Object::Ptr& responseData) {
    responseData->set("error", static_cast<int> (error));
}


ZApiHTTPUserRequestHandler::ZApiHTTPUserRequestHandler(const std::string& path) :
ZApiHTTPRequestBaseHandler(path){

}

ZApiHTTPUserRequestHandler::~ZApiHTTPUserRequestHandler() {
}

bool ZApiHTTPUserRequestHandler::CanHandleRequest(const std::string& path, const std::string& method) {
    if (path.compare("/user/register") == 0 && method.compare("POST") == 0) {
        return true;
    }

    if (path.compare("/user/login") == 0 && method.compare("POST") == 0) {
        return true;
    }

    if (path.compare("/user/password") == 0 && method.compare("POST") == 0) {
        return true;
    }

    if (path.compare("/user/password/reset") == 0 && method.compare("POST") == 0) {
        return true;
    }
    
    if (path.compare("/user/remove") == 0 && method.compare("POST") == 0) {
        return true;
    }

    return false;
}

void ZApiHTTPUserRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    Application& app = Application::instance();
    app.logger().information("Request from " + request.clientAddress().toString());

    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    Poco::JSON::Object::Ptr responseData = new Poco::JSON::Object;
    const std::string& method = request.getMethod();
    if (requestPath().compare("/user/register") == 0) {
        if (method.compare("POST") == 0) {
            handleUserRegister(request, responseData);
        }
    } else if (requestPath().compare("/user/login") == 0) {
        if (method.compare("POST") == 0) {
            handleUserLogin(request, responseData);
        }
    } else if (requestPath().compare("/user/password") == 0) {
        if (method.compare("POST") == 0) {
            handleUserPassword(request, responseData);
        }
    } else if (requestPath().compare("/user/password/reset") == 0) {
        if (method.compare("POST") == 0) {
            handleUserResetPassword(request, responseData);
        }
    } else if (requestPath().compare("/user/remove") == 0) {
        if (method.compare("POST") == 0) {
            handleUserRemove(request, responseData);
        }
    }

    std::ostream& ostr = response.send();
    Poco::JSON::Stringifier::stringify(responseData, ostr);
}

void ZApiHTTPUserRequestHandler::fillJson(const std::shared_ptr<ZUserInfo> &userInfo, Poco::JSON::Object::Ptr& responseData) {
    Poco::JSON::Object::Ptr juser = new Poco::JSON::Object();
    juser->set("email", userInfo->userName());
    juser->set("createdAt", (Poco::UInt64)userInfo->createdAt());
    juser->set("apikey", userInfo->apiKey());
    responseData->set("user", juser);
}


/// Request body
/// @code
/// {
///     "email":"iot@zenuous.com",
///     "password":"123456"
/// }
/// @endcode
///
/// Response body
/// @code
/// {
///     "session":"hh",
///     "user": {
///         "email":"iot@zenuous.com",
///         "createdAt":122314555,
///         "apikey":"807d56db-db06-4775-900a-04d08826d2cf"
///     }
/// }
///
/// {
///     "error":1,
///     "message":"user is existed"
/// }
/// @endcode

void ZApiHTTPUserRequestHandler::handleUserRegister(HTTPServerRequest& request, Object::Ptr& responseData) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    try {
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return;
        }

        Object::Ptr object = result.extract<Object::Ptr>();

        std::string email = object->optValue("email", std::string());
        std::string password = object->optValue("password", std::string());

        if (email.empty() || password.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return;
        }

        Poco::RegularExpression regexEmail("^[a-z0-9_\\+-]+(\\.[a-z0-9_\\+-]+)*@[a-z0-9-]+(\\.[a-z0-9-]+)*\\.([a-z]{2,4})$");
        Poco::RegularExpression::MatchVec matchVecEmail;
        int numMatchEmail = regexEmail.match(email, 0, matchVecEmail);
        if (numMatchEmail <= 0) {
            ReportError::reportTo(static_cast<HttpApiError> (ZErrorCode::User_InvalidEmailAddress), responseData);
            return;
        }

        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResultUserInfoWithEmail = worker->getUserInfo(email);
        if (!workerResultUserInfoWithEmail.failed()) {
            //user nay da ton tai
            ReportError::reportTo(static_cast<HttpApiError> (ZErrorCode::User_Exists), responseData);
            return;
        }
        ZWorkerResult workerResultCreateUser = worker->createNewUser(email, password, email, "no_avatar");
        if (workerResultCreateUser.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResultCreateUser.errorCode()), responseData);
            return;
        }
        int32_t userId = workerResultCreateUser.extract<int32_t>();

        ZWorkerResult workerResultUserInfo = worker->getUserInfo(userId);
        if (workerResultUserInfo.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResultUserInfo.errorCode()), responseData);
            return;
        }
        ZUserInfo::Ptr user = workerResultUserInfo.extract<ZUserInfo::Ptr>();
        std::string jwt = generateAccountClaims(user, responseData);
        if (jwt.empty()) {
            return;
        }
        responseData->set("jwt", jwt);
        fillJson(user, responseData);
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
        return;
    }
}

void ZApiHTTPUserRequestHandler::handleUserLogin(HTTPServerRequest& request, Object::Ptr& responseData) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    try {
        Application& app = Application::instance();
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            ReportError::reportTo(HttpApiError::InvalidRequest, responseData);
            return;
        }

        Object::Ptr object = result.extract<Object::Ptr>();

        std::string email = object->optValue("email", std::string());
        std::string password = object->optValue("password", std::string());

        if (email.empty() || password.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return;
        }

        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResultUserInfo = worker->getUserInfo(email);
        if (workerResultUserInfo.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResultUserInfo.errorCode()), responseData);
            return;
        }
        ZUserInfo::Ptr user = workerResultUserInfo.extract<ZUserInfo::Ptr>();
        if (!user) {
            ReportError::reportTo(static_cast<HttpApiError> (ZErrorCode::Authentication_InvalidUserNameOrPassword), responseData);
            app.logger().information("Cannot find user: %s", email);
            return;
        }

        if (!user || user->password().compare(password) != 0) {
            ReportError::reportTo(static_cast<HttpApiError> (ZErrorCode::Authentication_InvalidUserNameOrPassword), responseData);
            return;
        }

        // create user session
        std::string jwt = generateAccountClaims(user, responseData);
        if (jwt.empty()) {
            ReportError::reportTo(static_cast<HttpApiError> (ZErrorCode::Authentication_FailedToGenerateToken), responseData);
            return;
        }
        responseData->set("jwt", jwt);
        fillJson(user, responseData);
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
        return;
    }
}

void ZApiHTTPUserRequestHandler::handleUserPassword(HTTPServerRequest& request, Object::Ptr& responseData) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    try {
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return;
        }

        Object::Ptr object = result.extract<Object::Ptr>();

        std::string oldPassword = object->optValue("oldPassword", std::string());
        std::string newPassword = object->optValue("newPassword", std::string());

        if (oldPassword.empty() || newPassword.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return;
        }

        std::string authToken = request.get("Authorization", std::string());
        if (authToken.empty()) {
            ReportError::reportTo(HttpApiError::AuthenticationMissing, responseData);
            return;
        }

        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResult = worker->changeUserPassword(authToken, oldPassword, newPassword);
        if (workerResult.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResult.errorCode()), responseData);
            return;
        }
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
        return;
    }
}

void ZApiHTTPUserRequestHandler::handleUserResetPassword(Poco::Net::HTTPServerRequest& request, Poco::JSON::Object::Ptr& responseData) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    try {
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return;
        }

        Object::Ptr object = result.extract<Object::Ptr>();

        std::string email = object->optValue("email", std::string());
        std::string newPassword = object->optValue("newPassword", std::string());

        if (email.empty() || newPassword.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return;
        }
        
        
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
        return;
    }
}

void ZApiHTTPUserRequestHandler::handleUserRemove(HTTPServerRequest& request, Object::Ptr& responseData) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    try {
        std::string authToken = request.get("Authorization", std::string());
        if (authToken.empty()) {
            ReportError::reportTo(HttpApiError::AuthenticationMissing, responseData);
            return;
        }

        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResult = worker->removeUser(authToken);
        if (workerResult.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResult.errorCode()), responseData);
            return;
        }

        // return empty {}
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
        return;
    }
}

std::string ZApiHTTPUserRequestHandler::generateAccountClaims(const std::shared_ptr<ZUserInfo> &user, Poco::JSON::Object::Ptr& responseData) const {
    std::string claims = "{\"isActivated\":1}";
    // create user session
    ZSessionService* sessionService = ZServiceLocator::instance()->
            get<ZSessionService>(ZServiceLocator::ServiceId::SessionService);
    std::string jwt;
    ZErrorCode errorCode = sessionService->create_jwt(user->userId(), claims, jwt);
    if (errorCode != ZErrorCode::OK) {
        ReportError::reportTo(static_cast<HttpApiError> (errorCode), responseData);
        return std::string();
    }

    return jwt;
}
