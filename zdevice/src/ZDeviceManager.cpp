/*
 * File:   ZDeviceManager.cpp
 * Author: huuhoa
 *
 * Created on October 24, 2015, 3:20 PM
 */

#include <sstream>

#include <Poco/Types.h>
#include <Poco/NumberParser.h>
#include <Poco/JSON/Parser.h>

#include <zcommon/ZServiceLocator.h>
#include <zcommon/ZMemProxy.h>
#include <zcommon/ZDBKey.h>
#include <zdb/ZDBProxy.h>
#include <zdb/ZIdGenerator.h>

#include <zdevice/ZDeviceManager.h>


ZDeviceManager::ZDeviceManager() {
}

ZDeviceManager::~ZDeviceManager() {
}

bool ZDeviceManager::initialize() {
    return true;
}

bool ZDeviceManager::start() {
    return true;
}

bool ZDeviceManager::stop() {
    return true;
}

bool ZDeviceManager::cleanup() {
    return true;
}

int32_t ZDeviceManager::createDeviceInfo(
        const std::string& deviceName, 
        const std::string& deviceAddress,
        int32_t deviceType, 
        const std::string& deviceGroup) {

    ZDeviceInfo deviceInfo;
    deviceInfo.setDeviceName(deviceName);
    deviceInfo.setDeviceAddress(deviceAddress);
    deviceInfo.setDeviceType(deviceType);
    deviceInfo.setDeviceGroup(deviceGroup);
    deviceInfo.setCreatedAt(time(NULL));
    deviceInfo.setUpdatedAt(time(NULL));

    ZIdGenerator* generator = ZServiceLocator::instance()->get<ZIdGenerator>(ZServiceLocator::ServiceId::IDGenerator);
    int32_t deviceId = generator->getNext(ZDBKey::generatorDevices());
    deviceInfo.setDeviceId(deviceId);

    std::string key = ZDBKey::DeviceEntry(deviceId);

    saveDeviceInfoToDB(deviceInfo);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    // create apikey for new device
    std::string uuid_str = generator->createUuid();    
    dbProxy->HSET(key, ZDBKey::apiKey(), uuid_str);
    dbProxy->HSET(ZDBKey::DevicesApiKey(), uuid_str, key);
    dbProxy->SADD(ZDBKey::DevicesSet(), {key});
    key = ZDBKey::relationDeviceVars(deviceAddress, deviceType);
    dbProxy->HSET(key, ZDBKey::addressKey(), std::to_string(deviceId));
    return deviceId;
}

int32_t ZDeviceManager::createFactoryDeviceInfo(const std::string& deviceName, int32_t deviceType, const std::string& deviceGroup) {
    ZDeviceInfo deviceInfo;
    deviceInfo.setDeviceName(deviceName);
    deviceInfo.setDeviceType(deviceType);
    deviceInfo.setDeviceGroup(deviceGroup);
    deviceInfo.setCreatedAt(time(NULL));
    deviceInfo.setUpdatedAt(time(NULL));

    ZIdGenerator* generator = ZServiceLocator::instance()->get<ZIdGenerator>(ZServiceLocator::ServiceId::IDGenerator);
    int32_t deviceId = generator->getNext(ZDBKey::generatorDevices());
    deviceInfo.setDeviceId(deviceId);

    std::string key = ZDBKey::DeviceEntry(deviceId);

    saveDeviceInfoToDB(deviceInfo);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    // create apikey for new device
    std::string uuid_str = generator->createUuid();
    dbProxy->HSET(key, ZDBKey::apiKey(), uuid_str);
    dbProxy->HSET(ZDBKey::DevicesApiKey(), uuid_str, key);
    dbProxy->SADD(ZDBKey::DevicesSet(), {key});
    dbProxy->SADD(ZDBKey::DevicesFactory(), {key});
    return deviceId;
}

void ZDeviceManager::saveVarInfoToDB(ZVarInfo& varInfo, const std::string& key) {
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    dbProxy->HMSET(key,{
        { "varId", std::to_string(varInfo.varId())},
        { "deviceId", std::to_string(varInfo.deviceId())},
        { "varName", varInfo.varName()}, 
        { "varValue", varInfo.varValue()}
    });
}

void ZDeviceManager::saveVarValueToDB(ZVarValue& varValue, int32_t deviceId) {
    std::string key = ZDBKey::VarValueEntry(deviceId, varValue.timeStamp());

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    dbProxy->HMSET(key,{
        { "varId", std::to_string(varValue.varId())},
        { "timeStamp", std::to_string(varValue.timeStamp())},
        { "value", varValue.value()}
    });
    
    dbProxy->SADD(ZDBKey::VarValueList(deviceId), {
        key
    });
    
    key = ZDBKey::VarValueArr(deviceId);
    dbProxy->LPUSH(key, varValue.toString());
    dbProxy->LTRIM(key, 0, 19);
    
    // push to history store
    
            
    key = ZDBKey::VarLastestEntry(deviceId);
    dbProxy->HSET(key, ZDBKey::VarLastestEntry(), std::to_string(varValue.timeStamp()));    
}

void ZDeviceManager::saveVarLastetValueToDB(ZVarLastestValue& varLastestValue) {
    std::string key = ZDBKey::VarLastestEntry(varLastestValue.varId());

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    dbProxy->SET(key, varLastestValue.value());
}

int32_t ZDeviceManager::createVarInfo(ZVarInfo varInfo, const std::string& key) {
    int32_t varId = varInfo.varId();
    saveVarInfoToDB(varInfo, key);
    return varId;
}

int32_t ZDeviceManager::createVarValue(ZVarValue varValue, int32_t deviceId) {
    int32_t varId = varValue.varId();
    saveVarValueToDB(varValue, deviceId);
    return varId;
}
int32_t ZDeviceManager::createBatteryValue(int32_t deviceId, int32_t battery) {
    std::string key = ZDBKey::VarLastestBatteryEntry(deviceId);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    dbProxy->HSET(key, ZDBKey::VarLastestBatteryEntry(), std::to_string(battery));    
    return 0;
}

int32_t ZDeviceManager::createVarLastestValue(ZVarLastestValue varLastestValue) {
    int32_t varId = varLastestValue.varId();
    saveVarLastetValueToDB(varLastestValue);
    return varId;
}

ZVarLastestValue::Ptr ZDeviceManager::getVarLastestValue(int32_t varId) {
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    std::string key = ZDBKey::VarLastestEntry(varId);
    if (!dbProxy->EXISTS(key)) {
        ZVarLastestValue::Ptr empty;
        return empty;
    }

    ZVarLastestValue::Ptr varLastestValue(new ZVarLastestValue);
    varLastestValue->setVarId(varId);
    varLastestValue->setValue(dbProxy->GET(key));
    return varLastestValue;
}

ZVarInfo::Ptr ZDeviceManager::getVarInfo(const std::string& key) {
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    if (dbProxy->HLEN(key) == 0) {
        ZVarInfo::Ptr empty;
        return empty;
    }
    ZVarInfo::Ptr varInfo(new ZVarInfo);
    ZDBProxy::StringList vals = dbProxy->HMGET(key,{"varId", "deviceId", "varName", "varValue"});

    Poco::Int32 i32Value;
    if (Poco::NumberParser::tryParse(vals[0], i32Value)) {
        varInfo->setVarId(i32Value);
    }
    if (Poco::NumberParser::tryParse(vals[1], i32Value)) {
        varInfo->setDeviceId(i32Value);
    }
    varInfo->setVarName(vals[2]);
    varInfo->setVarValue(vals[3]);
    return varInfo;
}

ZVarValue::Ptr ZDeviceManager::getVarValue(const std::string& key) {
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    if (dbProxy->HLEN(key) == 0) {
        ZVarValue::Ptr empty;
        return empty;
    }

    ZVarValue::Ptr varValue(new ZVarValue);
    ZDBProxy::StringList vals = dbProxy->HMGET(key,{"varId", "timeStamp", "value"});

    Poco::Int32 i32Value;
    if (Poco::NumberParser::tryParse(vals[0], i32Value)) {
        varValue->setVarId(i32Value);
    }
    Poco::UInt64 i64Value;
    if (Poco::NumberParser::tryParseUnsigned64(vals[1], i64Value)) {
        varValue->setTimeStamp(i64Value);
    }

    varValue->setValue(vals[2]);
    return varValue;
}

ZVarLastestValue::Map ZDeviceManager::mGetVarLastestInfo(const ZVarLastestValue::KeyList& keyList) {
    ZVarLastestValue::Map result;
    for (int32_t varId : keyList) {
        ZVarLastestValue::Ptr device = getVarLastestValue(varId);
        if (!device) {
            continue;
        }

        result[varId] = device;
    }
    return result;
}

ZVarValue::MapString ZDeviceManager::mGetVarValue(const ZVarValue::KeyListString& keyListString) {
    ZVarValue::MapString result;
    int count = 0;
    for (std::string key : keyListString) {
        count = count + 1;
        ZVarValue::Ptr varValue = getVarValue(key);
        if (!varValue) {
            continue;
        }

        result[key] = varValue;
        if(count > 50)
            break;
    }
    return result;
}

ZVarValue::ListString ZDeviceManager::mGetVarValue(const std::string& key, int32_t start, int32_t end) {
    ZVarValue::ListString result;
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    result = dbProxy->LRANGE(key, start, end);
    return result;
}

void ZDeviceManager::saveDeviceInfoToDB(ZDeviceInfo& deviceInfo) {
    std::string key = ZDBKey::DeviceEntry(deviceInfo.deviceId());

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    dbProxy->HMSET(key,{
        { "id", std::to_string(deviceInfo.deviceId())},
        { "name", deviceInfo.deviceName()},
        { "address", deviceInfo.deviceAddress()},
        { "type", std::to_string(deviceInfo.deviceType())},
        { "group", deviceInfo.deviceGroup()},
        { "createdAt", std::to_string(deviceInfo.createdAt())},
        { "updatedAt", std::to_string(deviceInfo.updatedAt())},
    });
}

/// Lấy thông tin về một tài khoản có sẵn

ZDeviceInfo::Ptr ZDeviceManager::getDeviceInfo(int32_t deviceId) {
    std::string key = ZDBKey::DeviceEntry(deviceId);
    return getDeviceInfo(key);
}

ZDeviceInfo::Ptr ZDeviceManager::getDeviceInfo(const std::string& address, int32_t sensor) {
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    Poco::Int32 i32Value;
    int deviceId = 0;
    std::string key = ZDBKey::relationDeviceVars(address, sensor);
    std::string value = dbProxy->HGET(key, ZDBKey::addressKey());
    if (Poco::NumberParser::tryParse(value, i32Value)) {
        deviceId = i32Value;
    }
    key = ZDBKey::DeviceEntry(deviceId);
    return getDeviceInfo(key);
}

ZDeviceInfo::Ptr ZDeviceManager::getDeviceInfo(const std::string& deviceKey) {
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    if (dbProxy->HLEN(deviceKey) == 0) {
        ZDeviceInfo::Ptr empty;
        return empty;
    }

    ZDeviceInfo::Ptr device(new ZDeviceInfo);
    ZDBProxy::StringList vals = dbProxy->HMGET(deviceKey,{"name", "createdAt", "updatedAt", "id", "type", "group", "apikey", "address"});
    device->setDeviceName(vals[0]);

    Poco::UInt64 i64Value;
    if (Poco::NumberParser::tryParseUnsigned64(vals[1], i64Value)) {
        device->setCreatedAt(i64Value);
    }
    if (Poco::NumberParser::tryParseUnsigned64(vals[2], i64Value)) {
        device->setUpdatedAt(i64Value);
    }
    Poco::Int32 i32Value;
    if (Poco::NumberParser::tryParse(vals[3], i32Value)) {
        device->setDeviceId(i32Value);
    }
    if (Poco::NumberParser::tryParse(vals[4], i32Value)) {
        device->setDeviceType(i32Value);
    }
    device->setDeviceGroup(vals[5]);
    device->setApiKey(vals[6]);
    device->setDeviceAddress(vals[7]);
    return device;
}

/// Lấy thông tin một số device theo danh sách khoá

ZDeviceInfo::Map ZDeviceManager::mGetDeviceInfo(const ZDeviceInfo::KeyList& keyList) {
    ZDeviceInfo::Map result;
    for (int32_t deviceId : keyList) {
        ZDeviceInfo::Ptr device = getDeviceInfo(deviceId);
        if (!device) {
            continue;
        }

        result[deviceId] = device;
    }

    return result;
}

ZDeviceInfo::Map ZDeviceManager::mGetDeviceInfo(const ZDeviceInfo::KeyListString& keyList) {
    ZDeviceInfo::Map result;
    for (std::string key : keyList) {
        ZDeviceInfo::Ptr device = getDeviceInfo(key);
        if (!device) {
            continue;
        }

        result[device->deviceId()] = device;
    }

    return result;
}

/// Lấy thông tin một số device theo danh sách tuần tự

ZDeviceInfo::Map ZDeviceManager::list(int32_t start, int32_t count) {
    ZDeviceInfo::Map result;

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    ZDBProxy::StringList allkeys = dbProxy->SMEMBERS(ZDBKey::DevicesSet());

    for (auto key : allkeys) {
        ZDeviceInfo::Ptr device = getDeviceInfo(key);
        if (!device) {
            continue;
        }

        result[device->deviceId()] = device;
    }
    return result;
}

/// Xoá device khỏi DB

ZDeviceManager::ErrorCode ZDeviceManager::removeDeviceInfo(int32_t deviceId) {
    ZDeviceInfo::Ptr deviceInfo = getDeviceInfo(deviceId);
    if (!deviceInfo) {
        return ErrorCode::NotFound;
    }

    std::string key = ZDBKey::DeviceEntry(deviceId);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string uuid = dbProxy->HGET(key, ZDBKey::apiKey());
    dbProxy->DEL(key);

    dbProxy->SREM(ZDBKey::DevicesSet(), {
        key});
    dbProxy->HDEL(ZDBKey::DevicesApiKey(), uuid);

    return ErrorCode::OK;
}

ZDeviceManager::ErrorCode ZDeviceManager::removeVarInfo(int32_t varId) {
    std::string key = ZDBKey::VarInfoEntry(varId);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    dbProxy->DEL(key);
    return ErrorCode::OK;
}

ZDeviceManager::ErrorCode ZDeviceManager::removeVarLastestValue(int32_t varId) {
    std::string key = ZDBKey::VarLastestEntry(varId);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    dbProxy->DEL(key);
    return ErrorCode::OK;
}

ZDeviceManager::ErrorCode ZDeviceManager::removeVarValue(int32_t deviceId) {
    //Xóa log ghi lại lịch sử update status của thiết bị
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key = ZDBKey::VarValueList(deviceId);
    ZDBProxy::StringList varValueKeys = dbProxy->SMEMBERS(key);
    for (std::string varValueKey : varValueKeys) {
        dbProxy->DEL(varValueKey);
    }
    dbProxy->DEL(key);

    return ErrorCode::OK;
}

/// Cập nhật thông tin về tài khoản
/// Trả về mã lỗi nếu không thành công

ZDeviceManager::ErrorCode ZDeviceManager::updateDeviceInfo(ZDeviceInfo::Ptr deviceInfo) {
    deviceInfo->setUpdatedAt(time(NULL));
    saveDeviceInfoToDB(*deviceInfo.get());
    return ErrorCode::OK;
}

