/*
 * File:   ZadminManager.cpp
 * Author: tamvh
 *
 * Created on November 27, 2015, 12:04 PM
 */

#include <sstream>
#include <algorithm>
#include <string>

#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/NumberParser.h>

#include <zcommon/ZServiceLocator.h>
#include <zcommon/ZDBKey.h>
#include <zadmin/ZAdminInfo.h>
#include <zdb/ZDBProxy.h>
#include <zdb/ZIdGenerator.h>

#include <zadmin/ZAdminManager.h>

ZAdminManager::ZAdminManager() {
}

ZAdminManager::~ZAdminManager() {
}

bool ZAdminManager::initialize() {
    return true;
}

bool ZAdminManager::start() {
    return true;
}

bool ZAdminManager::stop() {
    return true;
}

bool ZAdminManager::cleanup() {
    return true;
}

int32_t ZAdminManager::create(const std::string& userName,
        const std::string& password,
        const std::string& displayName,
        const std::string& avatar) {
    ZAdminInfo adminInfo;
    adminInfo.setUserName(userName);
    adminInfo.setPassword(password);
    adminInfo.setDisplayName(displayName);
    adminInfo.setAvatar(avatar);
    adminInfo.setCreatedAt(time(NULL));
    adminInfo.setUpdatedAt(time(NULL));
    adminInfo.setIsAdmin(true);

    ZIdGenerator* generator = ZServiceLocator::instance()->get<ZIdGenerator>(ZServiceLocator::ServiceId::IDGenerator);
    int32_t adminId = generator->getNext(ZDBKey::generatorAdmin());
    adminInfo.setUserId(adminId);

    std::string key = ZDBKey::AdminEntry(adminId);

    saveToDB(adminInfo);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    // create apikey for new user admin
    std::string uuid_str = generator->createUuid();
    dbProxy->HSET(key, ZDBKey::apiKey(), uuid_str);
    dbProxy->HSET(ZDBKey::AdminApiKey(), uuid_str, key);
    dbProxy->HSET(ZDBKey::AdminEmail(), userName, std::to_string(adminId));

    dbProxy->SADD(ZDBKey::AdminSet(), {
        key});
    return adminId;
}

void ZAdminManager::saveToDB(ZAdminInfo& adminInfo) {
    std::string key = ZDBKey::AdminEntry(adminInfo.userId());

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    dbProxy->HMSET(key,{
        { "userId", std::to_string(adminInfo.userId())},
        { "userName", adminInfo.userName()},
        { "displayName", adminInfo.displayName()},
        { "avatar", adminInfo.avatar()},
        { "password", adminInfo.password()},
        { "createdAt", std::to_string(adminInfo.createdAt())},
        { "updatedAt", std::to_string(adminInfo.updatedAt())},
        { "isAdmin", std::to_string(adminInfo.isAdmin())},
    });
}

ZAdminInfo::Ptr ZAdminManager::get(int32_t userId) {
    std::string key = ZDBKey::AdminEntry(userId);

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    if (dbProxy->HLEN(key) == 0) {
        ZAdminInfo::Ptr empty;
        return empty;
    }

    ZAdminInfo::Ptr user(new ZAdminInfo);
    ZDBProxy::StringList vals = dbProxy->HMGET(key,{"userId", "userName", "displayName", "avatar", "password", "createdAt", "updatedAt", "isAdmin"});

    Poco::Int64 i64Value;
    Poco::Int32 i32Value;

    if (Poco::NumberParser::tryParse(vals[0], i32Value)) {
        user->setUserId(i32Value);
    }
    user->setUserName(vals[1]);
    user->setDisplayName(vals[2]);
    user->setAvatar(vals[3]);
    user->setPassword(vals[4]);
    if (Poco::NumberParser::tryParse64(vals[5], i64Value)) {
        user->setCreatedAt(i64Value);
    }

    if (Poco::NumberParser::tryParse64(vals[6], i64Value)) {
        user->setUpdatedAt(i64Value);
    }

    user->setIsAdmin(Poco::NumberParser::parseBool(vals[7]));
    user->setApiKey(dbProxy->HGET(key, ZDBKey::apiKey()));
    return user;
}

ZAdminInfo::Ptr ZAdminManager::get(const std::string& userName) {
    std::string userId_Str;
    int32_t userId = -1;
    ZAdminInfo::Ptr admin;
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    userId_Str = dbProxy->HGET(ZDBKey::AdminEmail(), userName);

    if (userId_Str.empty()) {
        return admin;
    }
    userId = Poco::NumberParser::parse(userId_Str);
    return get(userId);
}

ZAdminInfo::Map ZAdminManager::multiGet(const ZAdminInfo::KeyList& keyList) {
    ZAdminInfo::Map result;
    for (int32_t userId : keyList) {
        ZAdminInfo::Ptr admin = get(userId);
        if (!admin) {
            continue;
        }

        result[userId] = admin;
    }

    return result;
}

ZAdminInfo::Map ZAdminManager::list(int32_t start, int32_t count) {
    ZAdminInfo::Map result;

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    ZDBProxy::StringList allkeys = dbProxy->SMEMBERS(ZDBKey::AdminSet());

    for (auto key : allkeys) {
        ZAdminInfo::Ptr user = get(key);
        if (!user) {
            continue;
        }

        result[user->userId()] = user;
    }
    return result;
}

ZAdminManager::ErrorCode ZAdminManager::remove(int32_t userId) {
    ZAdminInfo::Ptr adminInfo = get(userId);
    if (!adminInfo) {
        return ErrorCode::NotFound;
    }
    std::string key = ZDBKey::AdminEntry(userId);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string uuid = dbProxy->HGET(key, ZDBKey::apiKey());
    dbProxy->DEL(key);

    dbProxy->SREM(ZDBKey::AdminSet(), {
        key});
    dbProxy->HDEL(ZDBKey::AdminApiKey(), uuid);

    return ErrorCode::OK;
}

ZAdminManager::ErrorCode ZAdminManager::update(ZAdminInfo::Ptr adminInfo) {
    adminInfo->setUpdatedAt(time(NULL));
    saveToDB(*adminInfo.get());
    return ErrorCode::OK;
}






