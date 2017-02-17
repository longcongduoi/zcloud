/*
 * File:   ZWorker.cpp
 * Author: huuhoa
 *
 * Created on November 30, 2015, 9:33 AM
 */

#include <memory>
#include <stdlib.h>
#include <sstream>
#include <Poco/Util/Application.h>
#include <Poco/RegularExpression.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Object.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Parser.h>
#include <zcommon/ZServiceLocator.h>
#include <zcommon/StringUtil.h>
#include <zcommon/ZDBKey.h>
#include <zdevice/ZDeviceInfo.h>
#include <zdevice/ZVarInfo.h>
#include <zdevice/ZVarValue.h>
#include <zdevice/ZVarLastestValue.h>
#include <zdevice/ZDevice.h>
#include <zdevice/ZDeviceManager.h>
#include <zapihttp.web/ZHTTPWebManager.h>
#include <zuser/ZUserInfo.h>
#include <zadmin/ZAdminInfo.h>
#include <zadmin/ZAdminManager.h>
#include <zsession/ZSessionService.h>
#include <zdb/ZDBProxy.h>
#include <zdb/ZIdGenerator.h>
#include <zapimqtt/ZApiMQTT.h>

#include "zworker/ZDeviceDataUpdatedEvent.h"
#include "zworker/ZDeviceDataDeletedEvent.h"
#include "zworker/ZDeviceDataCreatedEvent.h"
#include "zworker/ZDeviceDataChangeInformationEvent.h"
#include "zworker/ZCloudReceiveMessageEvent.h"
#include "zworker/ZWorker.h"


using namespace Poco::JSON;
using namespace Poco::Dynamic;
using Poco::Util::Application;
using Poco::JSON::Object;
using Poco::JSON::Array;
using Poco::Dynamic::Var;

class ZWorker::Impl {
public:
    std::shared_ptr<ZHTTPWebManager> _widgetMrg;
    std::shared_ptr<ZApiMQTT> _mqtt;
};

ZWorker::ZWorker(): d_ptr(new Impl) {
}

ZWorker::~ZWorker() {
}

bool ZWorker::initialize() {
    return true;
}

bool ZWorker::start() {
    return true;
}

bool ZWorker::stop() {
    return true;
}

bool ZWorker::cleanup() {
    return true;
}

ZWorkerResult ZWorker::validateUserAuthentication(const std::string& token) {
    ZSessionService* sessionService = ZServiceLocator::instance()->
            get<ZSessionService>(ZServiceLocator::ServiceId::SessionService);
    int32_t userId;
    ZErrorCode errorCode = sessionService->validate_jwt(token, userId);
    if (errorCode != ZErrorCode::OK) {
        return errorCode;
    }

    return ZWorkerResult(Poco::Dynamic::Var(userId));
}

ZWorkerResult ZWorker::addDIYDevice(
        const std::string& userToken,
        const std::string& deviceName,
        const std::string& deviceAddress,
        const std::string& deviceGroup,
        int32_t deviceType) {

    Application& app = Application::instance();
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    // User tạo ra device mới
    // User luôn tồn tại và hợp lệ nếu authentication token đã được chứng minh hợp lệ
    int32_t userId = authResult.extract<int32_t>();

    // Tạo ra device info mới
    ZIdGenerator* generator = ZServiceLocator::instance()->get<ZIdGenerator>(ZServiceLocator::ServiceId::IDGenerator);
    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);    

    int deviceId = deviceManager->createDeviceInfo(deviceName, deviceAddress, deviceType, deviceGroup);
    ZDeviceInfo::Ptr deviceInfo = deviceManager->getDeviceInfo(deviceId);
    ZVarInfo varInfo;
    int32_t varId = generator->getNext("varId");
    std::string defaultValue = "0";
    std::string varName = std::to_string(deviceType);
    varInfo.setVarId(varId);
    varInfo.setDeviceId(deviceId);
    varInfo.setVarName(varName);
    varInfo.setVarValue(defaultValue);
    std::string key = ZDBKey::relationDeviceVars(deviceAddress, deviceType);
    deviceManager->createVarInfo(varInfo, key);

    //  write log (create var value)
    ZVarValue varValue;
    varValue.setVarId(varInfo.varId());
    varValue.setTimeStamp(time(NULL));
    varValue.setValue(defaultValue);
    deviceManager->createVarValue(varValue, deviceId);

    // Thực hiện gắn device mới với user request.

    std::string key_relation_user_devices = ZDBKey::relationUserDevices(userId);

    dbProxy->SADD(key_relation_user_devices,{
        std::to_string(deviceInfo->deviceId())
    });
    
    
    //add widget
    addWidget(userId, deviceInfo);
    
    std::shared_ptr<ZDevice> device(new ZDevice);
    device->setInfo(deviceInfo);
    ZDeviceDataCreatedEvent event(device, varId, defaultValue);
    deviceDataCreated(this, event);

    Poco::Dynamic::Var var(deviceInfo);
    ZWorkerResult result(var);
    return result;
}


ZWorkerResult ZWorker::addSensorDevice(
        const std::string& apiKey,
        const std::string& deviceName,
        const std::string& deviceAddress,
        const std::string& deviceGroup,
        int32_t deviceType,
        const std::string& value) {
    Application& app = Application::instance();
    ZIdGenerator* generator = ZServiceLocator::instance()->get<ZIdGenerator>(ZServiceLocator::ServiceId::IDGenerator);
    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key = ZDBKey::UserEntry(apiKey);
    int32_t userId = 0;
    Poco::Int32 i32Value;
    if (Poco::NumberParser::tryParse(dbProxy->HGET(key, ZDBKey::UserApiKey()), i32Value)) {
        userId = i32Value;
    }
    else {
        return ZWorkerResult(ZErrorCode::User_NotFound);
    }

    // Tạo ra device info mới
    int deviceId = deviceManager->createDeviceInfo(deviceName, deviceAddress, deviceType, deviceGroup);
    ZDeviceInfo::Ptr deviceInfo = deviceManager->getDeviceInfo(deviceId);
    ZVarInfo varInfo;
    int32_t varId = generator->getNext("varId");
    std::string varName = std::to_string(deviceType);
    varInfo.setVarId(varId);
    varInfo.setDeviceId(deviceId);
    varInfo.setVarName(varName);
    varInfo.setVarValue(value);
    key = ZDBKey::relationDeviceVars(deviceAddress, deviceType);
    deviceManager->createVarInfo(varInfo, key);


    // Thực hiện gắn device mới với user request.

    std::string key_relation_user_devices = ZDBKey::relationUserDevices(userId);

    dbProxy->SADD(key_relation_user_devices,{
        std::to_string(deviceInfo->deviceId())
    });
    
    //add widget
    addWidget(i32Value, deviceInfo);
    std::shared_ptr<ZDevice> device(new ZDevice);
    device->setInfo(deviceInfo);
    ZDeviceDataCreatedEvent event(device, varId, value);
    deviceDataCreated(this, event);

//    Poco::Dynamic::Var var(deviceInfo);
    ZWorkerResult result(device);
    return result;
}

ZWorkerResult ZWorker::addIndieDevice(
        const std::string& userToken,
        const std::string& deviceName,
        const std::string& deviceGroup,
        int32_t deviceId,
        const std::string & deviceApiKey) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    // User luôn tồn tại và hợp lệ nếu authentication token đã được chứng minh hợp lệ
    int32_t userId = authResult.extract<int32_t>();

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->
            get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    //    int deviceId = deviceManager->create(name, type, group);
    ZDeviceInfo::Ptr device = deviceManager->getDeviceInfo(deviceId);
    if (!device) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }

    if (device->apiKey().compare(deviceApiKey) != 0) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }

    device->setDeviceName(deviceName);
    device->setDeviceGroup(deviceGroup);
    device->setCreatedAt(time(NULL));
    deviceManager->updateDeviceInfo(device);

    // Thực hiện gắn device mới với user request.
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key_relation_user_devices = ZDBKey::relationUserDevices(userId);

    dbProxy->SADD(key_relation_user_devices,{
        std::to_string(device->deviceId())
    });


    std::string keyfdevice = ZDBKey::DevicesFactory();
    std::string skey = ZDBKey::DeviceEntry(device->deviceId());
    dbProxy->SREM(keyfdevice,{
        skey
    });   
    
    ZWorkerResult result(device);
    return result;
}

ZWorkerResult ZWorker::getAllDevicesWithUser(const std::string & userToken) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    // User luôn tồn tại và hợp lệ nếu authentication token đã được chứng minh hợp lệ
    int32_t userId = authResult.extract<int32_t>();

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key_relation_user_devices = ZDBKey::relationUserDevices(userId);
    ZDBProxy::StringList deviceKeys = dbProxy->SMEMBERS(key_relation_user_devices);
    if (deviceKeys.empty()) {
        // return empty result
        return ZWorkerResult(ZDeviceInfo::Map());
    }

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Map devices = deviceManager->mGetDeviceInfo(StringUtil::convert(deviceKeys));
    return ZWorkerResult(devices);
}

ZWorkerResult ZWorker::getAllDevicesWithUserApikey(const std::string & userApikey) { 
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key = ZDBKey::UserEntry(userApikey);
    std::string userId_str = dbProxy->HGET(key, ZDBKey::UserApiKey());
    int32_t userId = 0;
    Poco::Int32 i32Value;
    if (Poco::NumberParser::tryParse(userId_str, i32Value)) {
        userId = i32Value;
    }
    else {
        return ZWorkerResult(ZErrorCode::User_NotFound);
    }
    
    
    std::string key_relation_user_devices = ZDBKey::relationUserDevices(userId);
    ZDBProxy::StringList deviceKeys = dbProxy->SMEMBERS(key_relation_user_devices);
    if (deviceKeys.empty()) {
        // return empty result
        return ZWorkerResult(ZDeviceInfo::Map());
    }

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Map devices = deviceManager->mGetDeviceInfo(StringUtil::convert(deviceKeys));
    return ZWorkerResult(devices);
}

ZWorkerResult ZWorker::getAllDevicesInIoT(const std::string& userToken) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    std::string key = ZDBKey::DevicesSet();
    ZDBProxy::StringList deviceKeys = dbProxy->SMEMBERS(key);
    if (deviceKeys.empty()) {
        // return empty result
        return ZWorkerResult(ZDeviceInfo::Map());
    }

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Map devices = deviceManager->mGetDeviceInfo(deviceKeys);
    return ZWorkerResult(devices);
}

ZWorkerResult ZWorker::getAllFactoryDevices(const std::string& userToken) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    std::string key = ZDBKey::DevicesFactory();
    ZDBProxy::StringList deviceKeys = dbProxy->SMEMBERS(key);
    if (deviceKeys.empty()) {
        // return empty result
        return ZWorkerResult(ZDeviceInfo::Map());
    }

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Map devices = deviceManager->mGetDeviceInfo(deviceKeys);
    return ZWorkerResult(devices);
}

ZWorkerResult ZWorker::mGetVarLastestValues(const std::string& userToken, int32_t deviceId) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    std::string key = ZDBKey::relationDeviceVars(deviceId);
    ZDBProxy::StringList deviceKeys = dbProxy->SMEMBERS(key);
    if (deviceKeys.empty()) {
        // return empty result
        return ZWorkerResult(ZVarLastestValue::Map());
    }
    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZVarLastestValue::Map varLastestValues = deviceManager->mGetVarLastestInfo(StringUtil::convert(deviceKeys));
    return ZWorkerResult(varLastestValues);
}

ZWorkerResult ZWorker::getDevice(const std::string& userToken, int32_t deviceId) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Ptr deviceInfo = deviceManager->getDeviceInfo(deviceId);
    if (deviceInfo == NULL) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }

    std::shared_ptr<ZDevice> device(new ZDevice);
    device->setInfo(deviceInfo);

    return ZWorkerResult(device);
}

ZWorkerResult ZWorker::getDevice(const std::string& userToken, const std::string& address, int sensor) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Ptr deviceInfo = deviceManager->getDeviceInfo(address, sensor);
    if (deviceInfo == NULL) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }

    std::shared_ptr<ZDevice> device(new ZDevice);
    device->setInfo(deviceInfo);

    return ZWorkerResult(device);
}

ZWorkerResult ZWorker::getSensorDevice(const std::string& address, int sensor) {
    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Ptr deviceInfo = deviceManager->getDeviceInfo(address, sensor);
    if (deviceInfo == NULL) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }
    
    

    std::shared_ptr<ZDevice> device(new ZDevice);
    device->setInfo(deviceInfo);

    return ZWorkerResult(device);
}

ZWorkerResult ZWorker::getDIYDevice(const std::string& userToken, int32_t deviceId) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }
    int32_t userId = authResult.extract<int32_t>();

    //kiem tra xem device da dc gắn với user tương ứng chưa.
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key_relation_user_devices = ZDBKey::relationUserDevices(userId);
    if (!dbProxy->SISMEMBER(key_relation_user_devices, std::to_string(deviceId))) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Ptr deviceInfo = deviceManager->getDeviceInfo(deviceId);
    if (deviceInfo == NULL) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }

    std::shared_ptr<ZDevice> device(new ZDevice);
    device->setInfo(deviceInfo);

    return ZWorkerResult(device);
}

ZWorkerResult ZWorker::deleteDevice(const std::string& userToken, int32_t deviceId) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    // User luôn tồn tại và hợp lệ nếu authentication token đã được chứng minh hợp lệ
    int32_t userId = authResult.extract<int32_t>();

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Ptr deviceInfo = deviceManager->getDeviceInfo(deviceId);
    //deleteInfo
    deviceManager->removeDeviceInfo(deviceId);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    // delete relation user-device
    std::string key_relation_user_devices = ZDBKey::relationUserDevices(userId);

    dbProxy->SREM(key_relation_user_devices,{
        std::to_string(deviceId)
    });

    //delete var info - var lastest device - var value
    std::string key_relation_device_vars = ZDBKey::relationDeviceVars(deviceId);
    ZDBProxy::StringList varKeys = dbProxy->SMEMBERS(key_relation_device_vars);
    ZVarInfo::KeyList keyList = StringUtil::convert(varKeys);
    for (int32_t varKey : keyList) {
        deviceManager->removeVarInfo(varKey);
        deviceManager->removeVarLastestValue(varKey);
        deviceManager->removeVarValue(varKey);

        dbProxy->SREM(key_relation_device_vars,{
            std::to_string(varKey)
        });
    }
    
    // delete widget;
    d_ptr->_widgetMrg->removeWidget(userId, deviceId);
    
    // broadcast device data delete
    ZDeviceDataDeletedEvent event(std::to_string(deviceId), deviceInfo->apiKey());
    deviceDataDeleted(this, event);

    return ZWorkerResult(ZErrorCode::OK);
}

//update deviceName, deviceGroup

ZWorkerResult ZWorker::updateDevice(const std::string &userToken, int32_t deviceId, const std::string &deviceName, const std::string & deviceGroup) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    // User luôn tồn tại và hợp lệ nếu authentication token đã được chứng minh hợp lệ
    int32_t userId = authResult.extract<int32_t>();

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key_relation_user_devices = ZDBKey::relationUserDevices(userId);
    if (!dbProxy->SISMEMBER(key_relation_user_devices, std::to_string(deviceId))) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Ptr deviceInfo = deviceManager->getDeviceInfo(deviceId);
    if (!deviceInfo) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }

    deviceInfo->setDeviceName(deviceName);
    deviceInfo->setDeviceGroup(deviceGroup);
    deviceManager->updateDeviceInfo(deviceInfo);

    std::shared_ptr<ZDevice> device(new ZDevice);
    device->setInfo(deviceInfo);

    return ZWorkerResult(device);
}

ZWorkerResult ZWorker::initStatusDevice(
        const std::string& userToken,
        int32_t deviceId,
        int32_t varId,
        const std::string& value) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Ptr deviceInfo = deviceManager->getDeviceInfo(deviceId);
    if (!deviceInfo) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }

    std::shared_ptr<ZDevice> device(new ZDevice);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    std::string key_relation_device_vars = ZDBKey::relationDeviceVars(deviceId);
    std::string member = std::to_string(varId);
    if (!dbProxy->SISMEMBER(key_relation_device_vars, member)) {
        return ZWorkerResult(ZErrorCode::Unknown);
    }

    //create lastest value
    ZVarLastestValue varLastestValue;
    varLastestValue.setVarId(varId);
    varLastestValue.setValue(value);
    deviceManager->createVarLastestValue(varLastestValue);
    device->setInfo(deviceInfo);
    return ZWorkerResult(device);
}

//UPDATE STATUS DEVICE

ZWorkerResult ZWorker::updateStatusDevice(
        const std::string& userApikey,
        int32_t deviceId,
        int32_t deviceType,
        const std::string& deviceAddress,
        const std::string& value,
        int32_t battery) {
    Application& app = Application::instance();
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key = ZDBKey::UserEntry(userApikey);
    std::string userId_str = dbProxy->HGET(key, ZDBKey::UserApiKey());
    int32_t userId = 0;
    Poco::Int32 i32Value;
    if (Poco::NumberParser::tryParse(userId_str, i32Value)) {
        userId = i32Value;
    }
    else {
        return ZWorkerResult(ZErrorCode::User_NotFound);
    }

    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Ptr deviceInfo = deviceManager->getDeviceInfo(deviceAddress, deviceType);
    if (!deviceInfo) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }

    std::shared_ptr<ZDevice> device(new ZDevice);
    device->setInfo(deviceInfo);

    std::string key_relation_device_vars = ZDBKey::relationDeviceVars(deviceAddress, deviceType);
    ZVarInfo::Ptr varInfo = deviceManager->getVarInfo(key_relation_device_vars);
    varInfo->setVarValue(value);

    deviceManager->createVarInfo(*varInfo, key_relation_device_vars);
    
    //  write log (create var value)
    ZVarValue varValue;
    varValue.setVarId(varInfo->varId());
    varValue.setTimeStamp(time(NULL));
    varValue.setValue(value);
    deviceManager->createVarValue(varValue, deviceId);
    deviceManager->createBatteryValue(deviceId, battery);
        
    // broadcast device data update
    app.logger().information("about to broadcast to WS");
    ZWorker* worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
    ZWorkerResult workerResultUserInfo = worker->getUserInfo(userId);
    ZUserInfo::Ptr user = workerResultUserInfo.extract<ZUserInfo::Ptr>();
    if (!user) {
        return ZWorkerResult(ZErrorCode::User_NotFound);
    }
    std::string userApiKey = user->apiKey();
    std::string apiName = "changeValue";
    ZDeviceDataUpdatedEvent event(device, value, userApiKey, apiName, varValue.timeStamp(), battery);
    deviceDataUpdated(this, event);
    
    
    return ZWorkerResult(device);
}

ZWorkerResult ZWorker::updateDataDevice(const std::string& userApiKey, int32_t deviceId, int32_t varId, const std::string& value) {
    Application& app = Application::instance();
    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZDeviceInfo::Ptr deviceInfo = deviceManager->getDeviceInfo(deviceId);
    if (!deviceInfo) {
        return ZWorkerResult(ZErrorCode::Device_NotFound);
    }

    std::shared_ptr<ZDevice> device(new ZDevice);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    std::string key_relation_device_vars = ZDBKey::relationDeviceVars(deviceId);
    std::string member = std::to_string(varId);
    if (!dbProxy->SISMEMBER(key_relation_device_vars, member)) {
        return ZWorkerResult(ZErrorCode::Unknown);
    }

    //create lastest value
    ZVarLastestValue varLastestValue;
    varLastestValue.setVarId(varId);
    varLastestValue.setValue(value);
    deviceManager->createVarLastestValue(varLastestValue);
    device->setInfo(deviceInfo);

    app.logger().information("varId: %d", varId);
    app.logger().information("value: %s", value);

    // broadcast device data update
    app.logger().information("about to broadcast to WS");
    std::string apiName = "changeValue";
    int32_t battery = 0;
    ZDeviceDataUpdatedEvent event(device, value, userApiKey, apiName, time(NULL), battery);
    deviceDataUpdated(this, event);

    return ZWorkerResult(device);
}

//get varInfo

ZWorkerResult ZWorker::getVarInfo(const std::string& key) {
    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZVarInfo::Ptr varInfo = deviceManager->getVarInfo(key);
    if (!varInfo) {
        return ZWorkerResult(ZErrorCode::VarInfo_NotFound);
    }
    return ZWorkerResult(varInfo);
}

void ZWorker::deleteDevice(const std::string& deviceKey) {
    ZDeviceManager* deviceManager = ZServiceLocator::instance()->
            get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);

    int32_t deviceId = Poco::NumberParser::parse(deviceKey);
    //deleteInfo
    deviceManager->removeDeviceInfo(deviceId);

    //delete var info - var lastest device - var value
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key_relation_device_vars = ZDBKey::relationDeviceVars(deviceId);
    ZDBProxy::StringList varKeys = dbProxy->SMEMBERS(key_relation_device_vars);
    ZVarInfo::KeyList keyList = StringUtil::convert(varKeys);
    for (int32_t varKey : keyList) {
        deviceManager->removeVarInfo(varKey);
        deviceManager->removeVarLastestValue(varKey);
        deviceManager->removeVarValue(varKey);
    }
    dbProxy->DEL(key_relation_device_vars);
}

ZWorkerResult ZWorker::getLog(const std::string& userToken, int32_t deviceId, int32_t count) {
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }
    std::string key = ZDBKey::VarValueList(deviceId);
    ZDBProxy::StringList deviceKeys = dbProxy->SMEMBERS(key);
    if (deviceKeys.empty()) {
        // return empty result
        return ZWorkerResult(ZErrorCode::Unknown);
    }
    
    key = ZDBKey::VarValueArr(deviceId);
    ZVarValue::ListString listVarValue = deviceManager->mGetVarValue(key, 0, count);
    return ZWorkerResult(listVarValue);
}

ZWorkerResult ZWorker::removeLog(const std::string& userToken, int32_t deviceId) {
    ZDeviceManager* deviceManager = ZServiceLocator::instance()->get<ZDeviceManager>(ZServiceLocator::ServiceId::DeviceManager);
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }      
    ZDeviceManager::ErrorCode error_code = deviceManager->removeVarValue(deviceId);
    return ZWorkerResult(error_code);
}


ZWorkerResult ZWorker::getLocalTime(int32_t deviceId) {
    std::string localTime = "";
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key = ZDBKey::VarLastestEntry(deviceId);
    localTime = dbProxy->HGET(key, ZDBKey::VarLastestEntry());
    return ZWorkerResult(localTime);
}

ZWorkerResult ZWorker::getBattery(int32_t deviceId) {
    std::string battery = "";
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key = ZDBKey::VarLastestBatteryEntry(deviceId);
    battery = dbProxy->HGET(key, ZDBKey::VarLastestBatteryEntry());
    return ZWorkerResult(battery);
}

ZWorkerResult ZWorker::addWidget(
            int32_t userId, 
            const std::shared_ptr<ZDeviceInfo> &device) {
    bool result = d_ptr->_widgetMrg->addWidget(userId, device); 
    return ZWorkerResult(result);
}