/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ZApiHTTPDeviceApiRequestHandler.cpp
 * Author: tamvh
 * 
 * Created on December 14, 2016, 1:53 PM
 */
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
#include <zuser/ZUserManager.h>
#include <zuser/ZUserInfo.h>

#include "ZApiHTTPDeviceApiRequestHandler.h"

using namespace Poco::JSON;
using namespace Poco::Dynamic;
using Poco::DynamicStruct;
using Poco::Util::Application;
ZApiHTTPDeviceApiRequestHandler::ZApiHTTPDeviceApiRequestHandler(const std::string& requestPath) :
ZApiHTTPRequestBaseHandler(requestPath){
}

ZApiHTTPDeviceApiRequestHandler::~ZApiHTTPDeviceApiRequestHandler() {
}

bool ZApiHTTPDeviceApiRequestHandler::CanHandleRequest(const std::string& path, const std::string& method) {
    if (path.compare("/user/device/api/register") == 0 && 
            method.compare("POST") == 0) {
        return true;
    }

    if (path.compare("/user/device/api/update") == 0 && 
            method.compare("POST") == 0) {
        return true;
    }

    if (path.compare("/user/device/api/query") == 0 && 
            method.compare("GET") == 0) {
        return true;
    }
    
    if (path.compare("/user/device/data/push") == 0 &&
            method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
        return true;
    }
    
    if (path.compare("/user/device/control") == 0 &&
            method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
        return true;
    }
    return false;
}

void ZApiHTTPDeviceApiRequestHandler::handleRequest(
        Poco::Net::HTTPServerRequest& request, 
        Poco::Net::HTTPServerResponse& response) {
    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    Poco::Dynamic::Var responseData;
    const std::string& method = request.getMethod();
    if (requestPath().compare("/user/device/api/register") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
            responseData = handleRegisterDevice(request);
        }
    } else if (requestPath().compare("/user/device/api/update") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
            responseData = handleUpdateStatusDevice(request);
        }
    } else if (requestPath().compare("/user/device/api/query") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
            responseData = handleQueryDevice(request);
        }
    } else if (requestPath().compare("/user/device/data/push") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
            responseData = handleDevicePushData(request);
        }
    } else if (requestPath().compare("/user/device/control") == 0) {
        if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
            responseData = handleDeviceControl(request);
        }
    }

    std::ostream& ostr = response.send();
    Poco::JSON::Stringifier::stringify(responseData, ostr);
}


//Authen required
Poco::Dynamic::Var ZApiHTTPDeviceApiRequestHandler::handleDeviceControl(Poco::Net::HTTPServerRequest& request) {
    Application& app = Application::instance();
    app.logger().information("control device");

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
        int32_t sensor = object->optValue("sensor", 0);
        int32_t battery = object->optValue("battery", 0);
        std::string address = object->optValue("address", std::string());
        std::string value = object->optValue("value", std::string());
        std::string apikey = object->optValue("apikey", std::string());
        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult worker_result = worker->getSensorDevice(address, sensor);
        if (worker_result.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (worker_result.errorCode()), responseData);
        }
        std::shared_ptr<ZDevice> device = worker_result.extract<std::shared_ptr < ZDevice >> ();
        //update status device
        app.logger().information("About to update device data");
        ZWorkerResult workerResultUpdateSttDevice = worker->updateStatusDevice(
                apikey,
                device->info()->deviceId(),
                device->info()->deviceType(),
                address,
                value,
                battery);
        if (workerResultUpdateSttDevice.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (workerResultUpdateSttDevice.errorCode()), responseData);
            return responseData;
        }
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
    }
    return responseData;
}

Poco::Dynamic::Var ZApiHTTPDeviceApiRequestHandler::handleRegisterDevice(Poco::Net::HTTPServerRequest& request) {
    Application& app = Application::instance();
    app.logger().information("Requested to update device data");
    Object::Ptr responseData = new Object;
    try {
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return responseData;
        }

        Object::Ptr object = result.extract<Object::Ptr>();
        std::string address = object->optValue("address", std::string());
        int32_t sensor = object->optValue("sensor", 0);
        int32_t battery = object->optValue("battery", 0);
        std::string value = object->optValue("value", std::string());
        std::string apikey = object->optValue("apikey", std::string());
        std::string deviceName = "no name";
        std::string groupName = "Default Group";
        if (address.empty() || apikey.empty() || sensor < 1 || value.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }
        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult wDevice = worker->getSensorDevice(address, sensor);
        if (wDevice.failed()) {
            wDevice = worker->addSensorDevice(
                    apikey,
                    deviceName,
                    address,
                    groupName,
                    sensor,
                    value);
        }
        std::shared_ptr<ZDevice> device = wDevice.extract<std::shared_ptr < ZDevice >> ();
        //update status device
        app.logger().information("About to update device data");
        ZWorkerResult wUpdateStt = worker->updateStatusDevice(
                apikey,
                device->info()->deviceId(),
                device->info()->deviceType(),
                address,
                value, 
                battery);
        if (wUpdateStt.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (wUpdateStt.errorCode()), responseData);
            return responseData;
        }
        app.logger().information("Update finished");
        fillJsonStatusDeviceUpdate(device, responseData);
        return responseData;

    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
    }
    return responseData;
}

Poco::Dynamic::Var ZApiHTTPDeviceApiRequestHandler::handleUpdateStatusDevice(Poco::Net::HTTPServerRequest& request) {
    Application& app = Application::instance();
    app.logger().information("Requested to update device data");
    Object::Ptr responseData = new Object;
    try {
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return responseData;
        }

        Object::Ptr object = result.extract<Object::Ptr>();
        std::string address = object->optValue("address", std::string());
        int32_t sensor = object->optValue("sensor", 0);
        int32_t battery = object->optValue("battery", -1);
        std::string value = object->optValue("value", std::string());
        std::string apikey = object->optValue("apikey", std::string());
        std::string deviceName = "no name";
        std::string groupName = "Default Group";
        if (address.empty() || apikey.empty() || sensor < 1 || value.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }
        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult worker_result = worker->getSensorDevice(address, sensor);
        if (worker_result.failed()) {
            worker_result = worker->addSensorDevice(
                    apikey,
                    deviceName,
                    address,
                    groupName,
                    sensor,
                    value);
        }
        std::shared_ptr<ZDevice> device = worker_result.extract<std::shared_ptr < ZDevice >> ();
        //update status device
        app.logger().information("About to update device data");
        ZWorkerResult wUpdateStt = worker->updateStatusDevice(
                apikey,
                device->info()->deviceId(),
                device->info()->deviceType(),
                address,
                value, 
                battery);
        if (wUpdateStt.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (wUpdateStt.errorCode()), responseData);
            return responseData;
        }
        app.logger().information("Update finished");
        fillJsonStatusDeviceUpdate(device, responseData);
        return responseData;

    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
    }
    return responseData;
}

Poco::Dynamic::Var ZApiHTTPDeviceApiRequestHandler::handleDevicePushData(Poco::Net::HTTPServerRequest& request) {
    Application& app = Application::instance();
    app.logger().information("Requested to update device data");
    Object::Ptr responseData = new Object;
    try {
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return responseData;
        }

        Object::Ptr object = result.extract<Object::Ptr>();
        std::string address = object->optValue("address", std::string());
        int32_t sensor = object->optValue("sensor", 0);
        int32_t battery = object->optValue("battery", -1);
        std::string value = object->optValue("value", std::string());
        std::string apikey = object->optValue("apikey", std::string());
        std::string deviceName = "no name";
        std::string groupName = "Default Group";
        if (address.empty() || apikey.empty() || sensor < 1 || value.empty()) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }
        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult worker_result = worker->getSensorDevice(address, sensor);
        if (worker_result.failed()) {
            worker_result = worker->addSensorDevice(
                    apikey,
                    deviceName,
                    address,
                    groupName,
                    sensor,
                    value);
        }
        std::shared_ptr<ZDevice> device = worker_result.extract<std::shared_ptr < ZDevice >> ();
        //update status device
        app.logger().information("About to update device data");
        ZWorkerResult wUpdateStt = worker->updateStatusDevice(
                apikey,
                device->info()->deviceId(),
                device->info()->deviceType(),
                address,
                value,
                battery);
        if (wUpdateStt.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (wUpdateStt.errorCode()), responseData);
            return responseData;
        }
        app.logger().information("Update finished");
        fillJsonStatusDeviceUpdate(device, responseData);
        return responseData;

    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
    }
    return responseData;
}

Poco::Dynamic::Var ZApiHTTPDeviceApiRequestHandler::handleQueryDevice(Poco::Net::HTTPServerRequest& request) {
    Application& app = Application::instance();
    app.logger().information("Query device");
    Object::Ptr responseData = new Object;
    try {               
        Var result = parseServerRequest(request, responseData);
        if (result.isEmpty()) {
            return responseData;
        }
        Object::Ptr object = result.extract<Object::Ptr>();        
        std::string apiKey = object->optValue("apikey", std::string());
        // query device
        ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult wResult = worker->getAllDevicesWithUserApikey(apiKey);
        if (wResult.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (wResult.errorCode()), responseData);
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
            fillJsonAllDevices(device, varInfo, localTime, item);
            array_device->add(item);
        }
        return array_device;
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
    }
    return responseData;
}


void ZApiHTTPDeviceApiRequestHandler::fillJsonStatusDeviceUpdate(
        const std::shared_ptr<ZDevice> &device,
        Poco::JSON::Object::Ptr & responseData) {
    int32_t error_code = 0;
    responseData->set("error", error_code);
    responseData->set("deviceid", device->info()->deviceId());
    responseData->set("apikey", device->info()->apiKey());
}

void ZApiHTTPDeviceApiRequestHandler::fillJsonAllDevices(
        const std::shared_ptr<ZDeviceInfo> &device,
        ZVarInfo::Ptr& varInfo,
        const std::string& localTime,
        Poco::JSON::Object::Ptr& responseData) {
    responseData->set("deviceid", device->deviceId());
    responseData->set("name", device->deviceName());
    responseData->set("address", device->deviceAddress());
    responseData->set("group", device->deviceGroup());
    responseData->set("type", device->deviceType());
    responseData->set("apikey", device->apiKey());
    responseData->set("createdAt", static_cast<Poco::Int64> (device->createdAt()));
    responseData->set("localTime", localTime);
    responseData->set("value", varInfo->varValue());
}