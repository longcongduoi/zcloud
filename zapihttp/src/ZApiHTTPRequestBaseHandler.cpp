/*
 * File:   ZApiHTTPRequestBaseHandler.cpp
 * Author: huuhoa
 *
 * Created on November 8, 2015, 9:11 AM
 */

#include <stdint.h>
#include <sstream>

#include <Poco/Util/Application.h>
#include <Poco/Dynamic/Struct.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>

#include <zcommon/ZServiceLocator.h>
#include <zsession/ZSessionService.h>

#include <zapihttp/ZApiHTTPRequestBaseHandler.h>

using namespace Poco::JSON;
using namespace Poco::Dynamic;

using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

using Poco::Util::Application;

ZApiHTTPRequestBaseHandler::ZApiHTTPRequestBaseHandler(const std::string& requestPath)
: _requestPath(requestPath) {
}

ZApiHTTPRequestBaseHandler::~ZApiHTTPRequestBaseHandler() {
}

Poco::Dynamic::Var ZApiHTTPRequestBaseHandler::parseServerRequest(Poco::Net::HTTPServerRequest& request,
        Poco::JSON::Object::Ptr& responseData) {

    std::istream& istr = request.stream();

    std::string requestBody = std::string(std::istreambuf_iterator<char>(istr), {});

    Poco::Dynamic::Var emptyResult;

    if (requestBody.empty()) {
        ReportError::reportTo(HttpApiError::InvalidRequest, responseData);
        return emptyResult;
    }

    Parser parser;
    try {
        Var result = parser.parse(requestBody);
        if (result.type() != typeid (Object::Ptr)) {
            ReportError::reportTo(HttpApiError::InvalidRequest, responseData);
            return emptyResult;
        }

        Object::Ptr object = result.extract<Object::Ptr>();
        if (object->size() == 0) {
            ReportError::reportTo(HttpApiError::InvalidRequest, responseData);
            return emptyResult;
        }

        return result;
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
        return emptyResult;
    }
}

//
//
////! Derived class will populate @data member
//Poco::Dynamic::Var DoorMessageRequest::toJSON() const
//{
//    Poco::JSON::Object::Ptr inner;
//    inner = new Poco::JSON::Object;
//    inner->set("command", command_);
//    inner->set("session", session_);
//
//    if (doorId_ > 0) {
//        inner->set("id", doorId_);
//    }
//    if (!doorName_.empty()) {
//        inner->set("name", doorName_);
//    }
//    if (!doorDescription_.empty()) {
//        inner->set("description", doorDescription_);
//    }
//
//    if (!beaconUuid_.empty()) {
//        inner->set("beaconUuid", beaconUuid_);
//    }
//    if (beaconMajor_ >= 0) {
//        inner->set("beaconMajor", beaconMajor_);
//    }
//    if (beaconMinor_ >= 0) {
//        inner->set("beaconMinor", beaconMinor_);
//    }
//    if (beaconPosition_ >= 0) {
//        inner->set("beaconMinor", beaconPosition_);
//    }
//    if (beaconDistance_ >= 0) {
//        inner->set("beaconMinor", beaconDistance_);
//    }
//
//    if (start_ >= 0) {
//        inner->set("start", start_);
//    }
//    if (count_ >= 0) {
//        inner->set("count", count_);
//    }
//    return inner;
//}
//
////! Derived class will extract properties from @data member
//bool DoorMessageRequest::fromJSON(const Poco::Dynamic::Var &data)
//{
//    if (data.isEmpty()) {
//        return false;
//    }
//
//    Object::Ptr object = data.extract<Object::Ptr>();
//    if (!object) {
//        return false;
//    }
//    const DynamicStruct& ds = *object;
//    try
//    {
//        if (!tryGet<std::string>(ds, "command", command_)) {
//            return false;
//        }
//        if (!tryGet<std::string>(ds, "session", session_)) {
//            return false;
//        }
//        if (!tryGet<int32_t>(ds, "id", doorId_)) {
//            doorId_ = 0;
//        }
//        if (!tryGet<std::string>(ds, "name", doorName_)) {
//            doorName_ = "";
//        }
//        if (!tryGet<std::string>(ds, "description", doorDescription_)) {
//            doorDescription_ = "";
//        }
//        if (!tryGet<std::string>(ds, "beaconUuid", beaconUuid_)) {
//            beaconUuid_ = "";
//        }
//        if (!tryGet<int32_t>(ds, "beaconMajor", beaconMajor_)) {
//            beaconMajor_ = -1;
//        }
//        if (!tryGet<int32_t>(ds, "beaconMinor", beaconMinor_)) {
//            beaconMinor_ = -1;
//        }
//        if (!tryGet<int32_t>(ds, "beaconPosition", beaconPosition_)) {
//            beaconPosition_ = -1;
//        }
//        if (!tryGet<int32_t>(ds, "beaconDistance", beaconDistance_)) {
//            beaconDistance_ = -1;
//        }
//        if (!tryGet<int32_t>(ds, "start", start_)) {
//            start_ = -1;
//        }
//        if (!tryGet<int32_t>(ds, "count", count_)) {
//            count_ = 0;
//        }
//        return true;
//    }
//    catch (Poco::Exception ex)
//    {
//        std::cerr << "error while parsing data: " << ex.displayText() << std::endl;
//        return false;
//    }
//}

//std::string ZApiHTTPUserRequestHandler::toJSONString() {
//    Poco::JSON::Object::Ptr root = new Poco::JSON::Object;
//    root->set("cmd", _command);
//    root->set("token", (Poco::UInt64)_token);
//    root->set("type", MessageTypeToString(_type));
//    if (_data) {
//        root->set("data", _data->toJSON());
//    }
//    root->set("error_code", _errorCode);
//    if (!_errorMessage.empty()) {
//        root->set("error_message", _errorMessage);
//    }
//
//    std::stringstream oss;
//    Poco::JSON::Stringifier::stringify(root, oss);
//    return oss.str();
//}

int32_t ZApiHTTPRequestBaseHandler::getAuthenticatedUser(Poco::Net::HTTPServerRequest& request, Poco::JSON::Object::Ptr& responseData) const {
    std::string authToken = request.get("Authorization", std::string());
    if (authToken.empty()) {
        ReportError::reportTo(HttpApiError::AuthenticationMissing, responseData);
        return -1;
    }
    const std::string bearer("Bearer ");
    if (!(authToken.compare(0, bearer.length(), bearer) == 0)) {
        ReportError::reportTo(HttpApiError::AuthenticationMissing, responseData);
        return -1;
    }
    authToken = authToken.substr(bearer.length());
    ZSessionService* sessionService = ZServiceLocator::instance()->
            get<ZSessionService>(ZServiceLocator::ServiceId::SessionService);
    int32_t userId;
    ZErrorCode errorCode = sessionService->validate_jwt(authToken, userId);
    if (errorCode != ZErrorCode::OK) {
        ReportError::reportTo(static_cast<HttpApiError> (errorCode), responseData);
        return -1;
    }
    return userId;
}

std::string ZApiHTTPRequestBaseHandler::getAuthenticationToken(Poco::Net::HTTPServerRequest& request, Poco::JSON::Object::Ptr& responseData) const {
    std::string authToken = request.get("Authorization", std::string());
    if (authToken.empty()) {
        ReportError::reportTo(HttpApiError::AuthenticationMissing, responseData);
        return std::string();
    }
    const std::string bearer("Bearer ");
    if (!(authToken.compare(0, bearer.length(), bearer) == 0)) {
        ReportError::reportTo(HttpApiError::AuthenticationMissing, responseData);
        return std::string();
    }
    authToken = authToken.substr(bearer.length());
    return authToken;
}

