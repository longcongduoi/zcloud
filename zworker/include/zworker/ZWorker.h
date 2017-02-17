/*
 * File:   ZWorker.h
 * Author: huuhoa
 *
 * Created on November 30, 2015, 9:33 AM
 */

#ifndef ZWORKER_H
#define ZWORKER_H
#include <string>
#include <memory>
#include <Poco/Dynamic/Var.h>
#include <Poco/BasicEvent.h>
#include <zcommon/ZServiceInterface.h>
#include <zcommon/ErrorCode.h>

class ZVarLastestValue;
class ZDevice;
class ZDeviceInfo;
class ZHTTPWebManager;
class ZDeviceDataUpdatedEvent;
class ZDeviceDataDeletedEvent;
class ZDeviceDataCreatedEvent;
class ZDeviceDataChangeInformationEvent;
class ZCloudReceiveMessageEvent;
class ZApiMQTT;

class ZWorkerResult {
public:

    ZWorkerResult(ZErrorCode error) : error_(error) {
    }

    ZWorkerResult(Poco::Dynamic::Var obj) : obj_(obj), error_(ZErrorCode::OK) {
    }
public:

    ZErrorCode errorCode() const {
        return error_;
    }

    bool succeeded() const {
        return errorCode() == ZErrorCode::OK;
    }

    bool failed() const {
        return errorCode() != ZErrorCode::OK;
    }

    template <typename T>
    const T& extract() const {
        return obj_.extract<T>();
    }
private:
    Poco::Dynamic::Var obj_;
    ZErrorCode error_;
};

class ZWorker : public ZServiceInterface {
public:
    ZWorker();
    virtual ~ZWorker();

public:
    virtual bool initialize();
    virtual bool start();
    virtual bool stop();
    virtual bool cleanup();

public:
    ZWorkerResult addDIYDevice(
            const std::string& userToken,
            const std::string& deviceName,
            const std::string& deviceAddress,
            const std::string& deviceGroup,
            int32_t deviceType);
    
    ZWorkerResult addSensorDevice(
            const std::string& apiKey,
            const std::string& deviceName,
            const std::string& deviceAddress,
            const std::string& deviceGroup,
            int32_t deviceType,
            const std::string& value);
    
    ZWorkerResult addIndieDevice(
            const std::string& userToken,
            const std::string& deviceName,
            const std::string& deviceGroup,
            int32_t deviceId,
            const std::string& deviceApiKey);    
    ZWorkerResult getAllDevicesWithUser(const std::string& userToken);
    ZWorkerResult getAllDevicesWithUserApikey(const std::string & userApikey);
    ZWorkerResult getAllDevicesInIoT(const std::string& userToken);
    ZWorkerResult getAllFactoryDevices(const std::string& userToken);
    ZWorkerResult getDevice(const std::string& userToken, int32_t deviceId);
    ZWorkerResult getDevice(const std::string& userToken, const std::string& address, int sensor);
    ZWorkerResult getSensorDevice(const std::string& address, int sensor);
    ZWorkerResult getDIYDevice(const std::string& userToken, int32_t deviceId);
    ZWorkerResult mGetVarLastestValues(const std::string& userToken, int32_t deviceId);
    ZWorkerResult deleteDevice(const std::string& userToken, int32_t deviceId);
    ZWorkerResult initStatusDevice(
            const std::string& userToken,
            int32_t deviceId,
            int32_t varId,
            const std::string& value);     
      
    ZWorkerResult updateStatusDevice(
            const std::string& userApikey,
            int32_t deviceId,
            int32_t deviceType,
            const std::string& deviceAddress,
            const std::string& value,            
            int32_t battery);
    
    ZWorkerResult updateDataDevice(
            const std::string& userApiKey,
            int32_t deviceId,
            int32_t varId,
            const std::string& value);
    ZWorkerResult getVarInfo(const std::string& key);
    ZWorkerResult updateDevice(
            const std::string& userToken,
            int32_t deviceId,
            const std::string& deviceName,
            const std::string& deviceGroup);
    ZWorkerResult getAllUser(const std::string& userToken); //support for admin
    ZWorkerResult deleteUser(const std::string& userToken, int32_t userId); //support for admin
    ZWorkerResult getUserWithApikey(const std::string& userToken, const std::string& apiKey);
    ZWorkerResult getLog(const std::string& userToken, int32_t deviceId, int32_t count);
    ZWorkerResult removeLog(const std::string& userToken, int32_t deviceId);
    ZWorkerResult getLocalTime(int32_t deviceId);
    ZWorkerResult getBattery(int32_t deviceId);
    
    ZWorkerResult createNewUser(
            const std::string& email,
            const std::string& password,
            const std::string& displayName,
            const std::string& avatar);
    ZWorkerResult removeUser(const std::string& userToken); // for user unregister account him/herself
    ZWorkerResult getUserInfo(int32_t userId);
    ZWorkerResult getUserInfo(const std::string& email);
    ZWorkerResult changeUserPassword(
            const std::string& userToken,
            const std::string& oldPassword,
            const std::string& newPassword);
    ZWorkerResult addWidget(
            int32_t userId, 
            const std::shared_ptr<ZDeviceInfo> &device);
    ZWorkerResult validateUserAuthentication(const std::string& token);
private:
    //    void broadcastEvent(std::shared_ptr<ZDevice> device, ZVarLastestValue* value);
    void deleteDevice(const std::string& deviceKey);
    bool doesUserExist(int32_t userId);

public:
    //! @session Events
    //! All event broadcasted by worker such as device data update, device added, device deleted, etc.
    Poco::BasicEvent<const ZDeviceDataUpdatedEvent> deviceDataUpdated;
    Poco::BasicEvent<const ZDeviceDataDeletedEvent> deviceDataDeleted;
    Poco::BasicEvent<const ZDeviceDataCreatedEvent> deviceDataCreated;
    Poco::BasicEvent<const ZDeviceDataChangeInformationEvent> deviceDataChangedInformation;  
    Poco::BasicEvent<const ZCloudReceiveMessageEvent> receiveMessageEvent;
private:
    class Impl;
    std::shared_ptr<Impl> d_ptr;
};

#endif /* ZWORKER_H */

