/*
 * File:   ZApiHTTPDeviceRequestHandler.cpp
 * Author: huuhoa
 *
 * Created on November 8, 2015, 9:09 AM
 */

#include <stdint.h>
#include <sstream>
#include <string>
#include <stdbool.h>
#include <vector>

#include <Poco/Util/Application.h>
#include <Poco/Dynamic/Struct.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/RegularExpression.h>

#include <zcommon/ZServiceLocator.h>
#include <zcommon/ZDBKey.h>
#include <zsession/ZSessionService.h>
#include <zworker/ZWorker.h>
#include <zdevice/ZDeviceManager.h>
#include <zdevice/ZDeviceInfo.h>
#include <zdevice/ZVarLastestValue.h>
#include <zdevice/ZDevice.h>
#include <zuser/ZUserManager.h>
#include <zuser/ZUserInfo.h>



#include "ZApiHTTPDeviceRequestHandler.h"

using namespace Poco::JSON;
using namespace Poco::Dynamic;
using Poco::DynamicStruct;


using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;

using Poco::Util::Application;
class ZDBKey;

ZApiHTTPDeviceRequestHandler::ZApiHTTPDeviceRequestHandler(const std::string& requestPath) :
ZApiHTTPRequestBaseHandler(requestPath) {

}

ZApiHTTPDeviceRequestHandler::~ZApiHTTPDeviceRequestHandler() {
}

bool ZApiHTTPDeviceRequestHandler::CanHandleRequest(const std::string& path, const std::string& method) {

    // /user/device[/:deviceId]
    if (path.compare("/user/device") == 0 &&
            ((method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) ||
            (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0))) {
        return true;
    }

    Poco::RegularExpression regex("/user/device(/[0-9a-fA-F]+)?");
    if ((regex == path) &&
            ((method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) ||
            (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) ||
            (method.compare(Poco::Net::HTTPRequest::HTTP_DELETE) == 0))) {
        return true;
    }

    if (path.compare("/user/device/add") == 0 &&
            (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0)) {
        return true;
    }

    Poco::RegularExpression regex_history("/user/device/history(/[0-9a-fA-F]+)?(/[0-9,-]+)?");
    if ((regex_history == path) &&
            ((method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) ||
            (method.compare(Poco::Net::HTTPRequest::HTTP_DELETE) == 0))) {
        return true;
    }

    return false;
}

void ZApiHTTPDeviceRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response) {
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    Poco::Dynamic::Var responseData;
    int numMatches;
    Poco::RegularExpression regex("/user/device(/([0-9a-fA-F]+))?");
    Poco::RegularExpression regex_history("/user/device/history(/[0-9a-fA-F]+)?(/[0-9,-]+)?");
    const std::string& method = request.getMethod();
    if (requestPath().compare("/user/device") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
            responseData = handleAddDIYDevice(request);
        } else if (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
            responseData = handleGetAllUserDevices(request);
        }
    } else if (requestPath().compare("/user/device/add") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
            responseData = handleAddIndieDevice(request);
        }
    } else {
        if (regex == requestPath()) {
            Poco::RegularExpression::MatchVec matchVec;
            numMatches = regex.match(requestPath(), 0, matchVec);
            if (numMatches > 1) {
                Poco::RegularExpression::Match m = matchVec[2];
                std::string deviceId = requestPath().substr(m.offset, m.length);
                if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
                    responseData = handleUpdateDeviceWithId(request, deviceId);
                } else if (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
                    responseData = handleGetDeviceWithId(request, deviceId);
                } else if (method.compare(Poco::Net::HTTPRequest::HTTP_DELETE) == 0) {
                    responseData = handleDeleteDeviceWithId(request, deviceId);
                }
            }
        }
        if (regex_history == requestPath()) {
            std::vector<std::string> params;
            numMatches = regex_history.split(requestPath(), params, 0);
            if (numMatches > 1) {
                std::string deviceId = params[1].substr(1, params[1].length() - 1);
                std::string count = params[2].substr(1, params[1].length() - 1);
                if (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
                    responseData = handleDeviceHistory(request, deviceId, count);
                } else if (method.compare(Poco::Net::HTTPRequest::HTTP_DELETE) == 0) {
                    responseData = handleDeleteDeviceHistory(request, deviceId);
                }
            }
        }
    }

    std::ostream& ostr = response.send();
    Poco::JSON::Stringifier::stringify(responseData, ostr);
}

void ZApiHTTPDeviceRequestHandler::fillJson(
        const std::shared_ptr<ZDeviceInfo> &device,
        Poco::JSON::Object::Ptr & responseData) {
    responseData->set("deviceid", device->deviceId());
    responseData->set("name", device->deviceName());
    responseData->set("group", device->deviceGroup());
    responseData->set("type", device->deviceType());
    responseData->set("apikey", device->apiKey());
    responseData->set("createdAt", static_cast<Poco::Int64> (device->createdAt()));
}

void ZApiHTTPDeviceRequestHandler::fillJsonAllDevices(
        const std::shared_ptr<ZDeviceInfo> &device,
        ZVarInfo::Ptr& varInfo,
        const std::string& localTime,
        const std::string& battery,
        Poco::JSON::Object::Ptr& responseData) {
    responseData->set("deviceid", device->deviceId());
    responseData->set("name", device->deviceName());
    responseData->set("address", device->deviceAddress());
    responseData->set("group", device->deviceGroup());
    responseData->set("type", device->deviceType());
    responseData->set("apikey", device->apiKey());
    responseData->set("createdAt", static_cast<Poco::Int64> (device->createdAt()));
    responseData->set("localTime", localTime);
    responseData->set("battery", battery);
    responseData->set("value", varInfo->varValue());
}

void ZApiHTTPDeviceRequestHandler::fillJsonVarLastestValue(
        const std::shared_ptr<ZVarLastestValue> &varLastestValue,
        Poco::JSON::Object::Ptr & responseData) {
    responseData->set("varId", varLastestValue->varId());
    responseData->set("value", varLastestValue->value());
}

void ZApiHTTPDeviceRequestHandler::fillJsonVarValue(
        const std::shared_ptr<ZVarValue> &varValue,
        Poco::JSON::Object::Ptr & responseData) {
    responseData->set("varId", varValue->varId());
    responseData->set("localtime", varValue->timeStamp());
    responseData->set("value", varValue->value());
}

void ZApiHTTPDeviceRequestHandler::fillJsonVarValue(
        const std::string &varValue,
        Poco::JSON::Object::Ptr & responseData) {
    responseData->set("value", varValue);
}

void ZApiHTTPDeviceRequestHandler::fillJsonVarValue(
        const std::string& value,
        const std::string& timestamp,
        Poco::JSON::Object::Ptr & responseData) {
    responseData->set("value", value);
    responseData->set("localTime", timestamp);
}

void ZApiHTTPDeviceRequestHandler::fillJsonVarInfo(
        const std::shared_ptr<ZVarLastestValue> &varLastestValue,
        const std::string& varName,
        Poco::JSON::Object::Ptr& responseData) {
    responseData->set("varId", varLastestValue->varId());
    responseData->set("value", varLastestValue->value());
    responseData->set("varName", varName);
}

void ZApiHTTPDeviceRequestHandler::fillJsonDetailDevice(
        const std::shared_ptr<ZDevice> &device,
        ZVarInfo::Ptr & varInfo,
        const std::string localTime,
        Poco::JSON::Object::Ptr & responseData) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;
    responseData->set("deviceid", device->info()->deviceId());
    responseData->set("name", device->info()->deviceName());
    responseData->set("address", device->info()->deviceAddress());
    responseData->set("group", device->info()->deviceGroup());
    responseData->set("type", device->info()->deviceType());
    responseData->set("apikey", device->info()->apiKey());
    responseData->set("createdAt", static_cast<Poco::Int64> (device->info()->createdAt()));
    responseData->set("lastModified", static_cast<Poco::Int64> (device->info()->updatedAt()));
    responseData->set("localTime", localTime);
    responseData->set("value", varInfo->varValue());

}

void ZApiHTTPDeviceRequestHandler::fillJsonHttpUpdate(
        const std::shared_ptr<ZDevice> &device,
        Poco::JSON::Object::Ptr & responseData) {
    int32_t error_code = 0;
    responseData->set("error", error_code);
    responseData->set("deviceid", device->info()->deviceId());
    responseData->set("apikey", device->info()->apiKey());
}

void ZApiHTTPDeviceRequestHandler::fillJsonHttpQuery(
        const std::shared_ptr<ZDevice> &device,
        Poco::JSON::Array::Ptr & data,
        Poco::JSON::Object::Ptr & responseData) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;
    int32_t error_code = 0;
    responseData->set("error", error_code);
    responseData->set("deviceid", device->info()->deviceId());
    responseData->set("apikey", device->info()->apiKey());
    responseData->set("params", data);
}


/// Request body
/// @code
/// {name: "Light Bulb", type: "02", group: "Living Room"}
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

Poco::Dynamic::Var ZApiHTTPDeviceRequestHandler::handleDeviceHistory(Poco::Net::HTTPServerRequest& request, const std::string& deviceid, const std::string& count) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;
    Application& app = Application::instance();
    Object::Ptr responseData = new Object;
    
    try {
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }

        if (deviceid.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }

        Poco::Int32 i32DeviceId;
        Poco::Int32 i32Count;
        if (!Poco::NumberParser::tryParse(deviceid, i32DeviceId)) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }
        if (!Poco::NumberParser::tryParse(count, i32Count)) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }
        
        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResult = worker->getLog(userToken, i32DeviceId, i32Count);
        Poco::JSON::Array::Ptr data = new Poco::JSON::Array;
        if (workerResult.failed()) {
            return data;
        }
        ZVarValue::ListString historys = workerResult.extract<ZVarValue::ListString>();        
        for (auto iter : historys) {
            Parser parser;
            std::string requestBody = std::string(iter, {});
            if(requestBody.empty()) {
                break;
            }
            Var zVarValue = parser.parse(requestBody);
            if (zVarValue.type() != typeid (Object::Ptr)) {
                break;
            }
            Object::Ptr obj = zVarValue.extract<Object::Ptr>();
            if (obj->size() == 0) {
                break;
            }
            
            Object::Ptr object = zVarValue.extract<Object::Ptr>();
            std::string value = object->optValue("value", std::string());
            std::string timeStamp = object->optValue("timeStamp", std::string());
            Object::Ptr item = new Object;
            fillJsonVarValue(value, timeStamp, item);
            data->add(item);
        }

        return data;
    } catch (Poco::Exception &ex) {
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
    }

    return responseData;
}

Poco::Dynamic::Var ZApiHTTPDeviceRequestHandler::handleDeleteDeviceHistory(Poco::Net::HTTPServerRequest& request, const std::string& deviceid) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;
    Application& app = Application::instance();
    Object::Ptr responseData = new Object;
    try {
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }

        if (deviceid.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }

        Poco::Int32 i32Value;
        if (!Poco::NumberParser::tryParse(deviceid, i32Value)) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }
        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResult = worker->removeLog(userToken, i32Value);
        if (workerResult.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResult.errorCode()), responseData);
            return responseData;
        }
    } catch (Poco::Exception &ex) {
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
    }

    return responseData;
}

//Create new device by using POST request

Poco::Dynamic::Var ZApiHTTPDeviceRequestHandler::handleAddDIYDevice(HTTPServerRequest & request) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    Object::Ptr responseData = new Object;
    try {
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }

        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return responseData;
        }

        Object::Ptr object = result.extract<Object::Ptr>();
        std::string name = object->optValue("name", std::string());
        std::string address = object->optValue("address", std::string());
        int32_t type = object->optValue("type", -1);
        std::string group = object->optValue("group", std::string());

        if (name.empty() || address.empty() || type < 0 || group.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }

        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResult = worker->addDIYDevice(userToken, name, address, group, type);
        if (workerResult.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResult.errorCode()), responseData);
            return responseData;
        }
        ZDeviceInfo::Ptr device = workerResult.extract<ZDeviceInfo::Ptr>();
        fillJson(device, responseData);
        return responseData;
    } catch (Poco::Exception &ex) {

        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
    }

    return responseData;
}

//get the list of devices owned by user by using GET request

Poco::Dynamic::Var ZApiHTTPDeviceRequestHandler::handleGetAllUserDevices(HTTPServerRequest & request) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    try {
        Object::Ptr responseData = new Object;
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }
        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult result = worker->getAllDevicesWithUser(userToken);
        if (result.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (result.errorCode()), responseData);
            return responseData;
        }
        ZDeviceInfo::Map devices = result.extract<ZDeviceInfo::Map>();
        Poco::JSON::Array::Ptr array_device = new Poco::JSON::Array;
        for (auto iter : devices) {
            ZDeviceInfo::Ptr device = iter.second;
            Object::Ptr item = new Object;
            std::string key = ZDBKey::relationDeviceVars(device->deviceAddress(), device->deviceType());
            ZWorkerResult wVarInfo = worker->getVarInfo(key);
            if (wVarInfo.failed()) {
                ReportError::reportTo(static_cast<HttpApiError> (wVarInfo.errorCode()), responseData);
                return responseData;
            }
            ZVarInfo::Ptr varInfo = wVarInfo.extract<ZVarInfo::Ptr>();
            std::string localTime = "";
            
            ZWorkerResult wLocalTime = worker->getLocalTime(device->deviceId());
            if (wLocalTime.failed()) {
                ReportError::reportTo(static_cast<HttpApiError> (wLocalTime.errorCode()), responseData);
                return responseData;
            }
            localTime = wLocalTime.extract<std::string>();
            
            std::string battery = "";
            ZWorkerResult wBattery = worker->getBattery(device->deviceId());
            if (wBattery.failed()) {
                ReportError::reportTo(static_cast<HttpApiError> (wBattery.errorCode()), responseData);
                return responseData;
            }
            battery = wBattery.extract<std::string>();
            fillJsonAllDevices(device, varInfo, localTime, battery, item);
            array_device->add(item);
        }
        return array_device;
    } catch (Poco::Exception &ex) {
        Object::Ptr responseData = new Object;
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);

        return responseData;
    }
}

Poco::Dynamic::Var ZApiHTTPDeviceRequestHandler::handleAddIndieDevice(HTTPServerRequest & request) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    Object::Ptr responseData = new Object;
    try {
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }

        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return responseData;
        }

        Object::Ptr object = result.extract<Object::Ptr>();

        std::string name = object->optValue("name", std::string());
        std::string group = object->optValue("group", std::string());
        int32_t deviceid = object->optValue("deviceid", -1);
        std::string apikey = object->optValue("apikey", std::string());

        if (name.empty() || group.empty() || deviceid < 0) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }

        ZWorker* worker = ZServiceLocator::instance()->
                get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResult = worker->addIndieDevice(userToken, name, group, deviceid, apikey);

        if (workerResult.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResult.errorCode()), responseData);
            return responseData;
        }

        ZDeviceInfo::Ptr device = workerResult.extract<ZDeviceInfo::Ptr>();
        fillJson(device, responseData);
        return responseData;
    } catch (Poco::Exception &ex) {

        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);

    }

    return responseData;
}


//update device name and group by using POST request,

Poco::Dynamic::Var ZApiHTTPDeviceRequestHandler::handleUpdateDeviceWithId(HTTPServerRequest& request, const std::string & deviceId) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    Object::Ptr responseData = new Object;
    try {
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return responseData;
        }

        Object::Ptr object = result.extract<Object::Ptr>();
        int32_t deviceid = Poco::NumberParser::parse(deviceId);
        std::string name = object->optValue("name", std::string());
        std::string group = object->optValue("group", std::string());

        if (name.empty() || group.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }

        std::string authToken = getAuthenticationToken(request, responseData);
        if (authToken.empty()) {
            ReportError::reportTo(HttpApiError::AuthenticationMissing, responseData);
            return responseData;
        }

        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResultDeviceInfo = worker->getDIYDevice(authToken, deviceid);


        if (workerResultDeviceInfo.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResultDeviceInfo.errorCode()), responseData);
            return responseData;
        }


        ZWorkerResult workerResult = worker->updateDevice(authToken, deviceid, name, group);
        std::shared_ptr<ZDevice> device = workerResult.extract<std::shared_ptr < ZDevice >> ();


        std::string key = ZDBKey::relationDeviceVars(device->info()->deviceAddress(), device->info()->deviceType());
        ZWorkerResult wVarInfo = worker->getVarInfo(key);
        if (wVarInfo.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (wVarInfo.errorCode()), responseData);
            return responseData;
        }
        ZVarInfo::Ptr varInfo = wVarInfo.extract<ZVarInfo::Ptr>();
        std::string localTime = ""; //update
        fillJsonDetailDevice(device, varInfo, localTime, responseData);
        return responseData;
    } catch (Poco::Exception &ex) {

        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
    }

    return responseData;
}


//Get detailed device information by using GET reques

Poco::Dynamic::Var ZApiHTTPDeviceRequestHandler::handleGetDeviceWithId(HTTPServerRequest& request, const std::string & deviceId) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;
    Application& app = Application::instance();
    Object::Ptr responseData = new Object;
    try {
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }

        int32_t deviceid = Poco::NumberParser::parse(deviceId);

        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResultDeviceInfo = worker->getDIYDevice(userToken, deviceid);


        if (workerResultDeviceInfo.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResultDeviceInfo.errorCode()), responseData);
            return responseData;
        }

        std::shared_ptr<ZDevice> device = workerResultDeviceInfo.extract<std::shared_ptr < ZDevice >> ();

        std::string key = ZDBKey::relationDeviceVars(device->info()->deviceAddress(), device->info()->deviceType());
        ZWorkerResult wVarInfo = worker->getVarInfo(key);
        if (wVarInfo.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (wVarInfo.errorCode()), responseData);
            return responseData;
        }
        ZVarInfo::Ptr varInfo = wVarInfo.extract<ZVarInfo::Ptr>();

        ZWorkerResult wLocalTime = worker->getLocalTime(deviceid);
        std::string localTime = wLocalTime.extract<std::string>();
        fillJsonDetailDevice(device, varInfo, localTime, responseData);
        return responseData;
    } catch (Poco::Exception &ex) {


        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);

    }

    return responseData;
}


//delete device by using DELETE request

Poco::Dynamic::Var ZApiHTTPDeviceRequestHandler::handleDeleteDeviceWithId(HTTPServerRequest& request, const std::string & _deviceId) {
    Poco::JSON::Object::Ptr responseData = new Poco::JSON::Object;
    try {
        std::string userToken = getAuthenticationToken(request, responseData);
        if (userToken.empty()) {
            return responseData;
        }

        int32_t deviceid = Poco::NumberParser::parse(_deviceId);
        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult workerResultgetDevice = worker->getDIYDevice(userToken, deviceid);
        ZErrorCode errorCode = workerResultgetDevice.errorCode();
        if (errorCode == ZErrorCode::User_NotFound) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResultgetDevice.errorCode()), responseData);
            return responseData;
        }

        ZWorkerResult workerResultdelDevice = worker->deleteDevice(userToken, deviceid);
        if (workerResultdelDevice.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResultdelDevice.errorCode()), responseData);
            return responseData;
        }

        return responseData;
    } catch (Poco::Exception &ex) {

        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);

    }

    return responseData;
}

