/*
 * File:   ZUserManager.cpp
 * Author: huuhoa
 *
 * Created on October 23, 2015, 2:05 PM
 */
#include <sstream>
#include <algorithm>
#include <string>

#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>
#include <Poco/NumberParser.h>

#include <zcommon/ZServiceLocator.h>
#include <zcommon/ZDBKey.h>
#include <zdb/ZDBProxy.h>
#include <zdb/ZIdGenerator.h>

#include <zuser/ZUserInfo.h>

#include "zuser/ZUserManager.h"

ZUserManager::ZUserManager() {
}

ZUserManager::~ZUserManager() {
}

bool ZUserManager::initialize() {
    return true;
}

bool ZUserManager::start() {
    return true;
}

bool ZUserManager::stop() {
    return true;
}

bool ZUserManager::cleanup() {
    return true;
}

int32_t ZUserManager::create(const std::string& userName,
        const std::string& password,
        const std::string& displayName,
        const std::string& avatar) {

    ZUserInfo userInfo;
    userInfo.setUserName(userName);
    userInfo.setPassword(password);
    userInfo.setDisplayName(displayName);
    userInfo.setAvatar(avatar);
    userInfo.setCreatedAt(time(NULL));
    userInfo.setUpdatedAt(time(NULL));

    ZIdGenerator* generator = ZServiceLocator::instance()->get<ZIdGenerator>(ZServiceLocator::ServiceId::IDGenerator);
    int32_t userId = generator->getNext(ZDBKey::generatorUsers());
    userInfo.setUserId(userId);
    std::string key = ZDBKey::UserEntry(userId);

    saveToDB(userInfo);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);

    // create apikey for new user
    std::string uuid_str = generator->createUuid();
    dbProxy->HSET(key, ZDBKey::apiKey(), uuid_str);
    dbProxy->HSET(ZDBKey::UserApiKey(), uuid_str, key);
    dbProxy->HSET(ZDBKey::UserEmail(), userName, std::to_string(userId));
    dbProxy->SADD(ZDBKey::UserSet(),{key});
    
    key = ZDBKey::UserEntry(uuid_str);
    dbProxy->HSET(key, ZDBKey::UserApiKey(), std::to_string(userId));        
    return userId;
}

void ZUserManager::saveToDB(ZUserInfo& userInfo) {
    std::string key = ZDBKey::UserEntry(userInfo.userId());

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    dbProxy->HMSET(key,{
        { "userId", std::to_string(userInfo.userId())},
        { "userName", userInfo.userName()},
        { "displayName", userInfo.displayName()},
        { "avatar", userInfo.avatar()},
        { "password", userInfo.password()},
        { "createdAt", std::to_string(userInfo.createdAt())},
        { "updatedAt", std::to_string(userInfo.updatedAt())},
    });
}

/// Lấy thông tin về một tài khoản có sẵn

ZUserInfo::Ptr ZUserManager::get(int32_t userId) {
    std::string userkey = ZDBKey::UserEntry(userId);

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    if (dbProxy->HLEN(userkey) == 0) {
        ZUserInfo::Ptr empty;
        return empty;
    }

    ZUserInfo::Ptr user(new ZUserInfo);

    ZDBProxy::StringList vals = dbProxy->HMGET(userkey, {
        "userId", "userName", "displayName", "avatar", "password", "createdAt", "updatedAt"
    });

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

    user->setApiKey(dbProxy->HGET(userkey, ZDBKey::apiKey()));
    return user;
}

ZUserInfo::Ptr ZUserManager::getWithUserKey(const std::string& userkey) {
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    if (dbProxy->HLEN(userkey) == 0) {
        ZUserInfo::Ptr empty;
        return empty;
    }

    ZUserInfo::Ptr user(new ZUserInfo);

    ZDBProxy::StringList vals = dbProxy->HMGET(userkey,{
        "userId", "userName", "displayName", "avatar", "password", "createdAt", "updatedAt"
    });

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

    user->setApiKey(dbProxy->HGET(userkey, ZDBKey::apiKey()));
    return user;
}

ZUserInfo::Ptr ZUserManager::get(const std::string& userName) {
    int32_t userId = -1;
    ZUserInfo::Ptr user;
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string userId_Str = dbProxy->HGET(ZDBKey::UserEmail(), userName);
    if (userId_Str.empty()) {
        return user;
    }
    if (!Poco::NumberParser::tryParse(userId_Str, userId)) {
        return user;
    }

    return get(userId);
}

/// Lấy thông tin một số tài khoản theo danh sách khoá

ZUserInfo::Map ZUserManager::multiGet(const ZUserInfo::KeyList& keyList) {
    ZUserInfo::Map result;
    for (int32_t userId : keyList) {
        ZUserInfo::Ptr user = get(userId);
        if (!user) {
            continue;
        }

        result[userId] = user;
    }

    return result;
}

/// Lấy thông tin một số tài khoản theo danh sách tuần tự

ZUserInfo::Map ZUserManager::list(int32_t start, int32_t count) {
    ZUserInfo::Map result;

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    ZDBProxy::StringList allkeys = dbProxy->SMEMBERS(ZDBKey::UserSet());

    for (auto key : allkeys) {
        ZUserInfo::Ptr user = getWithUserKey(key);
        if (!user) {
            continue;
        }

        result[user->userId()] = user;
    }
    return result;
}

/// Xoá tài khoản khỏi DB

ZUserManager::ErrorCode ZUserManager::remove(int32_t userId) {
    ZUserInfo::Ptr userInfo = get(userId);
    if (!userInfo) {
        return ErrorCode::NotFound;
    }

    std::string key = ZDBKey::UserEntry(userId);

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string uuid = dbProxy->HGET(key, ZDBKey::apiKey());
    dbProxy->DEL(key);
    dbProxy->SREM(ZDBKey::UserSet(),{key});
    dbProxy->HDEL(ZDBKey::UserApiKey(), uuid);
    dbProxy->HDEL(ZDBKey::UserEmail(), userInfo->userName());

    return ErrorCode::OK;
}

/// Cập nhật thông tin về tài khoản
/// Trả về mã lỗi nếu không thành công

ZUserManager::ErrorCode ZUserManager::update(ZUserInfo::Ptr userInfo) {
    userInfo->setUpdatedAt(time(NULL));
    saveToDB(*userInfo.get());
    return ErrorCode::OK;
}


