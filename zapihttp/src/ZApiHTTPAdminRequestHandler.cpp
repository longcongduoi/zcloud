/* 
 * File:   ZApiHTTPAdminRequestHandler.cpp
 * Author: tamvh
 * 
 * Created on November 25, 2015, 5:05 PM
 */

#include <stdint.h>
#include <sstream>
#include <string>

#include <Poco/Util/Application.h>
#include <Poco/Dynamic/Struct.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/RegularExpression.h>

#include <zcommon/ZServiceLocator.h>
#include <zsession/ZSessionService.h>
#include <zworker/ZWorker.h>
#include <zdevice/ZDeviceManager.h>
#include <zdevice/ZDeviceInfo.h>
#include <zdevice/ZDevice.h>
#include <zadmin/ZAdminInfo.h>
#include <zadmin/ZAdminManager.h>
#include <zuser/ZUserInfo.h>
#include <zuser/ZUserManager.h>

#include "ZApiHTTPAdminRequestHandler.h"



using namespace Poco::JSON;
using namespace Poco::Dynamic;
using Poco::DynamicStruct;


using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

using Poco::Util::Application;

ZApiHTTPAdminRequestHandler::ZApiHTTPAdminRequestHandler(const std::string& requestPath) :
ZApiHTTPRequestBaseHandler(requestPath) {

}

ZApiHTTPAdminRequestHandler::~ZApiHTTPAdminRequestHandler() {
}

bool ZApiHTTPAdminRequestHandler::CanHandleRequest(const std::string& path, const std::string& method) {

    if (path.compare("/admin/register") == 0 &&
            method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
        return true;
    }

    if (path.compare("/admin/login") == 0 &&
            method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
        return true;
    }

    if (path.compare("/admin/users") == 0 &&
            method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
        return true;
    }

    Poco::RegularExpression regexAdminUser("/admin/users(/[0-9a-fA-F]+)\\-?");
    Poco::RegularExpression::MatchVec matchVecAdminUser;
    int numMatches = regexAdminUser.match(path, 0, matchVecAdminUser);
    if (numMatches > 1 &&
            ((method.compare(Poco::Net::HTTPRequest::HTTP_DELETE) == 0) ||
            (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0))) {
        return true;
    }
    if (path.compare("/admin/devices") == 0 &&
            method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
        return true;
    }

    Poco::RegularExpression regexAdminDevices("/admin/devices(/[0-9a-zA-F]+)?");
    if (regexAdminDevices == path &&
            method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
        return true;
    }
    if (path.compare("/admin/factorydevices") == 0 &&
            method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
        return true;
    }
    if (path.compare("/admin/factorydevices/create") == 0 &&
            method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
        return true;
    }
    return false;
}

void ZApiHTTPAdminRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    Application& app = Application::instance();
    app.logger().information("Request from " + request.clientAddress().toString());

    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    Poco::Dynamic::Var responseData;
    const std::string& method = request.getMethod();
    Poco::RegularExpression regexAdminUserWApiKey("/admin/users(/[0-9a-fA-F]+)\\-?");
    Poco::RegularExpression regexAdminDevices("/admin/devices(/[0-9a-fA-F]+)?");
    Poco::RegularExpression::MatchVec matchVecUserWApiKey;
    Poco::RegularExpression::MatchVec matchVecDevices;
    int numMatchesUserWApiKey = regexAdminUserWApiKey.match(requestPath(), 0, matchVecUserWApiKey);
    int numMatchesDevices = regexAdminDevices.match(requestPath(), 0, matchVecDevices);

    if (requestPath().compare("/admin/register") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
            responseData = handlePostAdminRegister(request);
        }
    } else if (requestPath().compare("/admin/login") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
            responseData = handlePostAdminLogin(request);
        }
    } else if (requestPath().compare("/admin/users") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
            responseData = handleGetAdminUser(request);
        }
    } else if (numMatchesUserWApiKey > 1) {
        std::string apikey = requestPath().substr(matchVecUserWApiKey[1].offset + 1);
        app.logger().information("Api key: %s", apikey);
        if (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
            responseData = handleGetAdminUserApikey(request, apikey);
        } else if (method.compare(Poco::Net::HTTPRequest::HTTP_DELETE) == 0) {
            responseData = handleDeleteAdminUserApikey(request, apikey);
        }
    } else if (requestPath().compare("/admin/devices") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
            responseData = handleGetAdminDevices(request);
        }
    } else if (regexAdminDevices == requestPath()) {
        if (numMatchesDevices > 1) {
            std::string deviceId = requestPath().substr(matchVecDevices[1].offset + 1);
            app.logger().information("Device Id: %s", deviceId);
            if (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
                responseData = handleGetAdminDevicesDeviceId(request, deviceId);
            }
        }
    } else if (requestPath().compare("/admin/factorydevices") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
            responseData = handleGetAdminFactoryDevices(request);
        }
    } else if (requestPath().compare("/admin/factorydevices/create") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
            responseData = handlePostAdminFactoryDevicesCreate(request);
        }
    }

    std::ostream& ostr = response.send();
    Poco::JSON::Stringifier::stringify(responseData, ostr);
}

Poco::Dynamic::Var ZApiHTTPAdminRequestHandler::handlePostAdminRegister(Poco::Net::HTTPServerRequest& request) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;
    Object::Ptr responseData = new Object;
    try {
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return responseData;
        }

        Object::Ptr object = result.extract<Object::Ptr>();

        std::string email = object->optValue("email", std::string());
        std::string password = object->optValue("password", std::string());

        if (email.empty() || password.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }

        ZAdminManager* userManager = ZServiceLocator::instance()->get<ZAdminManager>(ZServiceLocator::ServiceId::AdminManager);
        int32_t userId = userManager->create(email, password, email, "no_avatar");
        ZAdminInfo::Ptr adminInfo = userManager->get(userId);
        std::string jwt = generateAccountClaims(adminInfo, responseData);
        responseData->set("jwt", jwt);

        fillJson(adminInfo, responseData);
        return responseData;
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
        return responseData;
    }
}

Poco::Dynamic::Var ZApiHTTPAdminRequestHandler::handlePostAdminLogin(Poco::Net::HTTPServerRequest& request) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;
    Object::Ptr responseData = new Object;
    try {
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return responseData;
        }

        Object::Ptr object = result.extract<Object::Ptr>();

        std::string email = object->optValue("email", std::string());
        std::string password = object->optValue("password", std::string());

        if (email.empty() || password.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }



        ZAdminManager* userManager = ZServiceLocator::instance()->get<ZAdminManager>(ZServiceLocator::ServiceId::AdminManager);
        ZAdminInfo::Ptr userInfo = userManager->get(email);
        if (!userInfo || userInfo->password().compare(password) != 0) {
            ReportError::reportTo(static_cast<HttpApiError> (ZErrorCode::Authentication_InvalidUserNameOrPassword), responseData);
            return responseData;
        }

        // create user session
        std::string jwt = generateAccountClaims(userInfo, responseData);
        if (jwt.empty()) {
            return responseData;
        }
        responseData->set("jwt", jwt);
        fillJson(userInfo, responseData);
        return responseData;
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
        return responseData;
    }
}

//Get the list of registered users on IoTgo 

Poco::Dynamic::Var ZApiHTTPAdminRequestHandler::handleGetAdminUser(Poco::Net::HTTPServerRequest& request) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    try {
        Object::Ptr responseData = new Object;
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }
        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult result = worker->getAllUser(userToken);
        ZUserInfo::Map userInfo = result.extract<ZUserInfo::Map>();
        Poco::JSON::Array::Ptr data = new Poco::JSON::Array;
        for (auto iter : userInfo) {
            ZUserInfo::Ptr user = iter.second;
            Object::Ptr item = new Object;
            fillJsonUserInfo(user, item);
            data->add(item);
        }
        return data;
    } catch (Poco::Exception &ex) {
        Object::Ptr responseData = new Object;
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);

        return responseData;
    }
}

//Get detailed user information by using GET request

Poco::Dynamic::Var ZApiHTTPAdminRequestHandler::handleGetAdminUserApikey(Poco::Net::HTTPServerRequest& request, std::string apikey) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    try {
        Object::Ptr responseData = new Object;
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }

        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult result = worker->getUserWithApikey(userToken, apikey);
        ZErrorCode errorCode = result.errorCode();

        if (errorCode == ZErrorCode::User_NotFound) {
            ReportError::reportTo(static_cast<HttpApiError> (errorCode), responseData);
            return responseData;
        }
        ZUserInfo::Ptr user = result.extract<ZUserInfo::Ptr>();
        Object::Ptr data = new Object;
        fillJsonUserInfo(user, data);
        return data;
    } catch (Poco::Exception &ex) {
        Object::Ptr responseData = new Object;
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);

        return responseData;
    }
}

// delete user and related devices by using DELETE request

Poco::Dynamic::Var ZApiHTTPAdminRequestHandler::handleDeleteAdminUserApikey(Poco::Net::HTTPServerRequest& request, std::string apikey) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    try {
        Object::Ptr responseData = new Object;
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }

        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult result = worker->getUserWithApikey(userToken, apikey);
        ZErrorCode errorCode = result.errorCode();
        Object::Ptr data = new Object;
        if (errorCode == ZErrorCode::User_NotFound) {
            ReportError::reportTo(static_cast<HttpApiError> (errorCode), responseData);
            return responseData;
        }
        ZUserInfo::Ptr user = result.extract<ZUserInfo::Ptr>();
        ZUserManager* userManager = ZServiceLocator::instance()->get<ZUserManager>(ZServiceLocator::ServiceId::UserManager);
        int32_t userId = user->userId();
        userManager->remove(userId);
        fillJsonUserInfo(user, data);
        return data;
    } catch (Poco::Exception &ex) {
        Object::Ptr responseData = new Object;
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);

        return responseData;
    }
}

//Get the list of created/added devices on IoTgo

Poco::Dynamic::Var ZApiHTTPAdminRequestHandler::handleGetAdminDevices(Poco::Net::HTTPServerRequest& request) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    try {
        Object::Ptr responseData = new Object;
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }
        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult result = worker->getAllDevicesInIoT(userToken);
        ZDeviceInfo::Map devices = result.extract<ZDeviceInfo::Map>();
        Poco::JSON::Array::Ptr data = new Poco::JSON::Array;

        for (auto iter : devices) {
            ZDeviceInfo::Ptr device = iter.second;
            Object::Ptr item = new Object;
            fillJsonDeviceInfo(device, item);
            data->add(item);
        }
        return data;
    } catch (Poco::Exception &ex) {
        Object::Ptr responseData = new Object;
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);

        return responseData;
    }
}

//Get detailed device information

Poco::Dynamic::Var ZApiHTTPAdminRequestHandler::handleGetAdminDevicesDeviceId(Poco::Net::HTTPServerRequest& request, std::string deviceId) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    Object::Ptr responseData = new Object;
    try {
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }
        int32_t deviceid = Poco::NumberParser::parse(deviceId);

        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResult = worker->getDevice(userToken, deviceid);

        if (workerResult.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResult.errorCode()), responseData);
            return responseData;
        }

        std::shared_ptr<ZDevice> device = workerResult.extract<std::shared_ptr < ZDevice >> ();

        ZWorkerResult workerResultVarlastestValues = worker->mGetVarLastestValues(userToken, deviceid);
        if (workerResultVarlastestValues.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResultVarlastestValues.errorCode()), responseData);
            return responseData;
        }
        ZVarLastestValue::Map varValues = workerResultVarlastestValues.extract<ZVarLastestValue::Map>();
        Poco::JSON::Array::Ptr data = new Poco::JSON::Array;
        for (auto iter : varValues) {
            ZVarLastestValue::Ptr varValue = iter.second;
            Object::Ptr item = new Object;
            fillJsonVarLastestValue(varValue, item);
            data->add(item);
        }

        fillJsonDetailDeviceInfo(device->info(), data, responseData);
        return responseData;
    } catch (Poco::Exception &ex) {

        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);

    }

    return responseData;
}


//Get issued licenses (for licensing indie devices) on IoTgo

Poco::Dynamic::Var ZApiHTTPAdminRequestHandler::handleGetAdminFactoryDevices(Poco::Net::HTTPServerRequest& request) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    Object::Ptr responseData = new Object;
    try {
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }

        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult zresult = worker->getAllFactoryDevices(userToken);
        ZDeviceInfo::Map zdevices = zresult.extract<ZDeviceInfo::Map>();
        Poco::JSON::Array::Ptr data = new Poco::JSON::Array;
        for (auto iter : zdevices) {
            ZDeviceInfo::Ptr device = iter.second;
            Object::Ptr item = new Object;
            fillJsonFactoryDeviceInfo(device, item);
            data->add(item);
        }
        return data;


    } catch (Poco::Exception &ex) {

        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);

    }

    return responseData;
}


//Generate new licenses for indie devices

Poco::Dynamic::Var ZApiHTTPAdminRequestHandler::handlePostAdminFactoryDevicesCreate(Poco::Net::HTTPServerRequest& request) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    Object::Ptr responseData = new Object;
    try {
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return responseData;
        }

        Object::Ptr object = result.extract<Object::Ptr>();

        std::string name = object->optValue("name", std::string());
        int32_t type = object->optValue("type", -1);
        int32_t qt = object->optValue("qt", -1);

        std::string group = "";
        int32_t deviceid = -1;


        if (name.empty() || type <= 0 || qt <= 0) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }


        ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
        Poco::JSON::Array::Ptr data = new Poco::JSON::Array;
        for (int i = 0; i < qt; i++) {
            deviceid = deviceManager->createFactoryDeviceInfo(name, type, group);
            ZDeviceInfo::Ptr device = deviceManager->getDeviceInfo(deviceid);
            Object::Ptr item = new Object;
            fillJsonFactoryDeviceInfo(device, item);
            data->add(item);
        }



        return data;
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
    }

    return responseData;
}

void ZApiHTTPAdminRequestHandler::fillJsonFactoryDeviceInfo(
        const std::shared_ptr<ZDeviceInfo> &device,
        Poco::JSON::Object::Ptr& responseData) {
    responseData->set("deviceid", device->deviceId());
    responseData->set("name", device->deviceName());
    responseData->set("type", device->deviceType());
    responseData->set("apikey", device->apiKey());
    responseData->set("createdAt", static_cast<Poco::Int64> (device->createdAt()));
}

void ZApiHTTPAdminRequestHandler::fillJsonDeviceInfo(
        const std::shared_ptr<ZDeviceInfo> &device,
        Poco::JSON::Object::Ptr& responseData) {
    responseData->set("deviceid", device->deviceId());
    responseData->set("name", device->deviceName());
    responseData->set("group", device->deviceGroup());
    responseData->set("type", device->deviceType());
    responseData->set("apikey", device->apiKey());
    responseData->set("createdAt", static_cast<Poco::Int64> (device->createdAt()));
    responseData->set("lastModified", static_cast<Poco::Int64> (device->updatedAt()));
}

void ZApiHTTPAdminRequestHandler::fillJsonDetailDeviceInfo(
        const std::shared_ptr<ZDeviceInfo> &device,
        Poco::JSON::Array::Ptr & data,
        Poco::JSON::Object::Ptr& responseData) {
    responseData->set("deviceid", device->deviceId());
    responseData->set("name", device->deviceName());
    responseData->set("group", device->deviceGroup());
    responseData->set("type", device->deviceType());
    responseData->set("apikey", device->apiKey());
    responseData->set("createdAt", static_cast<Poco::Int64> (device->createdAt()));
    responseData->set("lastModified", static_cast<Poco::Int64> (device->updatedAt()));
    responseData->set("params", data);
}

void ZApiHTTPAdminRequestHandler::fillJsonVarLastestValue(
        const std::shared_ptr<ZVarLastestValue> &varLastestValue,
        Poco::JSON::Object::Ptr & responseData) {
    responseData->set("varId", varLastestValue->varId());
    responseData->set("value", varLastestValue->value());
}

void ZApiHTTPAdminRequestHandler::fillJsonUserInfo(const std::shared_ptr<ZUserInfo> &userInfo, Poco::JSON::Object::Ptr& responseData) {
    responseData->set("email", userInfo->userName());
    responseData->set("apikey", userInfo->apiKey());
    responseData->set("createdAt", userInfo->createdAt());
}

void ZApiHTTPAdminRequestHandler::fillJson(const std::shared_ptr<ZAdminInfo> &adminInfo, Poco::JSON::Object::Ptr& responseData) {
    Poco::JSON::Object::Ptr juser = new Poco::JSON::Object();
    juser->set("email", adminInfo->userName());
    juser->set("isAdmin", adminInfo->isAdmin());
    responseData->set("user", juser);
}

std::string ZApiHTTPAdminRequestHandler::generateAccountClaims(const std::shared_ptr<ZAdminInfo> &user, Poco::JSON::Object::Ptr& responseData) const {
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

