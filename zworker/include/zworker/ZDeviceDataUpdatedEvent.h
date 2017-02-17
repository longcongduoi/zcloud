/*
 * File:   ZDeviceDataUpdatedEvent.h
 * Author: huuhoa
 *
 * Created on February 17, 2015, 9:33 AM
 */

#ifndef __ZDEVICEDATAUPDATEDEVENT_H__
#define __ZDEVICEDATAUPDATEDEVENT_H__

#include <memory>
#include <sys/types.h>
class ZDevice;

class ZDeviceDataUpdatedEvent {
public:
    ZDeviceDataUpdatedEvent(
            std::shared_ptr<ZDevice> device, 
            const std::string& varValue, 
            const std::string& userApiKey, 
            const std::string& apiName,
            int64_t timeStamp,
            int32_t battery)
        : device_(device),
        variableValue_(varValue),
        userApiKey_(userApiKey),
        apiName_(apiName),
        timeStamp_(timeStamp),
        battery_(battery){
    }

public:
    std::shared_ptr<ZDevice> device() const { return device_; }
    std::string varialbleValue() const { return variableValue_; }
    std::string userApiKey() const { return userApiKey_; }
    std::string apiName() const { return apiName_; }
    int64_t timeStamp() const { return timeStamp_; }
    int32_t battery() const { return battery_; }
private:
    std::shared_ptr<ZDevice> device_;
    std::string variableValue_;
    std::string userApiKey_;
    std::string apiName_;
    int64_t timeStamp_;
    int32_t battery_;
};

#endif // __ZDEVICEDATAUPDATEDEVENT_H__
