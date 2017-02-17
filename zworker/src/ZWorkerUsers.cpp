#include <Poco/RegularExpression.h>

#include <zcommon/ZServiceLocator.h>
#include <zcommon/StringUtil.h>
#include <zcommon/ZDBKey.h>

#include <zdb/ZDBProxy.h>

#include <zuser/ZUserInfo.h>
#include <zuser/ZUserManager.h>

#include <zworker/ZWorker.h>

ZWorkerResult ZWorker::getUserWithApikey(const std::string& userToken, const std::string & apiKey) {
    ZUserInfo::Ptr user;
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    std::string key = dbProxy->HGET(ZDBKey::UserApiKey(), apiKey);
    if (key.empty()) {
        return ZWorkerResult(ZErrorCode::User_NotFound);
    }

    ZUserManager* userManager = ZServiceLocator::instance()->get<ZUserManager>(ZServiceLocator::ServiceId::UserManager);
    user = userManager->getWithUserKey(key);

    return ZWorkerResult(user);
}

//Get the list of registered users on ZCloud
ZWorkerResult ZWorker::getAllUser(const std::string & userToken) {
    //authen with token admin
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }
    ZUserManager* userManager = ZServiceLocator::instance()->get<ZUserManager>(ZServiceLocator::ServiceId::UserManager);
    ZUserInfo::Map users = userManager->list(0, 10);
    return ZWorkerResult(users);
}

ZWorkerResult ZWorker::deleteUser(const std::string& userToken, int32_t userId) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }
    ZUserManager* userManager = ZServiceLocator::instance()->get<ZUserManager>(ZServiceLocator::ServiceId::UserManager);
    ZUserInfo::Ptr user = userManager->get(userId);
    userManager->remove(userId);
    return ZWorkerResult(user);
}

ZWorkerResult ZWorker::removeUser(const std::string& userToken) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }
    int32_t userId = authResult.extract<int32_t>();
    ZUserManager* userManager = ZServiceLocator::instance()->get<ZUserManager>(ZServiceLocator::ServiceId::UserManager);
    ZUserInfo::Ptr user = userManager->get(userId);
    if (!user) {
        return ZWorkerResult(ZErrorCode::User_NotFound);
    }

    userManager->remove(userId);
    ZDBProxy* dbProxy = ZServiceLocator::instance()->get<ZDBProxy>(ZServiceLocator::ServiceId::DBProxy);
    dbProxy->DEL(ZDBKey::UserSession(userId));

    // remove all devices associated with user
    std::string key_relation_user_devices = ZDBKey::relationUserDevices(userId);
    ZDBProxy::StringList deviceKeys = dbProxy->SMEMBERS(key_relation_user_devices);
    if (deviceKeys.empty()) {
        // return empty result
        return ZWorkerResult(ZErrorCode::OK);
    }

    for (std::string key : deviceKeys) {
        deleteDevice(key);
    }

    // remove relationships
    dbProxy->DEL(key_relation_user_devices);

    return ZWorkerResult(ZErrorCode::OK);
}

ZWorkerResult ZWorker::createNewUser(
        const std::string& email,
        const std::string& password,
        const std::string& displayName,
        const std::string& avatar) {
    Poco::RegularExpression regex("^[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,}$", Poco::RegularExpression::RE_CASELESS);
    if (!regex.match(email)) {
        return ZWorkerResult(ZErrorCode::User_InvalidEmailAddress);
    }
    ZUserManager* userManager = ZServiceLocator::instance()->get<ZUserManager>(ZServiceLocator::ServiceId::UserManager);
    int32_t userId = userManager->create(email, password, displayName, avatar);
    return ZWorkerResult(userId);
}

ZWorkerResult ZWorker::getUserInfo(int32_t userId) {
    ZUserManager* userManager = ZServiceLocator::instance()->get<ZUserManager>(ZServiceLocator::ServiceId::UserManager);
    ZUserInfo::Ptr user = userManager->get(userId);
    if (!user) {
        return ZWorkerResult(ZErrorCode::User_NotFound);
    }
    return ZWorkerResult(user);
}

ZWorkerResult ZWorker::getUserInfo(const std::string& email) {
    ZUserManager* userManager = ZServiceLocator::instance()->get<ZUserManager>(ZServiceLocator::ServiceId::UserManager);
    ZUserInfo::Ptr user = userManager->get(email);
    if (!user) {
        return ZWorkerResult(ZErrorCode::User_NotFound);
    }
    return ZWorkerResult(user);
}

ZWorkerResult ZWorker::changeUserPassword(const std::string& userToken, const std::string& oldPassword, const std::string& newPassword) {
    ZWorkerResult authResult = validateUserAuthentication(userToken);
    if (authResult.failed()) {
        return authResult;
    }

    int32_t userId = authResult.extract<int32_t>();
    ZUserManager* userManager = ZServiceLocator::instance()->get<ZUserManager>(ZServiceLocator::ServiceId::UserManager);
    ZUserInfo::Ptr user = userManager->get(userId);
    if (!user) {
        return ZWorkerResult(ZErrorCode::User_NotFound);
    }
    if (user->password().compare(oldPassword) != 0) {
        return ZWorkerResult(ZErrorCode::Unknown);
    }
    user->setPassword(newPassword);
    userManager->update(user);
    return ZWorkerResult(ZErrorCode::OK);
}

bool ZWorker::doesUserExist(int32_t userId) {
    ZUserManager* userManager = ZServiceLocator::instance()->get<ZUserManager>(ZServiceLocator::ServiceId::UserManager);
    ZUserInfo::Ptr user = userManager->get(userId);
    if (!user) {
        return false;
    } else {
        return true;
    }
}



