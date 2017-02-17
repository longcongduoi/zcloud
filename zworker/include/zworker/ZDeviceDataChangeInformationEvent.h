/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ZDeviceDataChangeInformationEvent.h
 * Author: tamvh
 *
 * Created on February 19, 2016, 3:27 PM
 */

#ifndef ZDEVICEDATACHANGEINFORMATIONEVENT_H
#define ZDEVICEDATACHANGEINFORMATIONEVENT_H

#include <memory>
class ZDevice;

class ZDeviceDataChangeInformationEvent {
public:
    ZDeviceDataChangeInformationEvent(std::shared_ptr<ZDevice> device, int32_t varId, const std::string& varValue)
        : device_(device),
        variableId_(varId),
        variableValue_(varValue) {
    }

public:
    std::shared_ptr<ZDevice> device() const { return device_; }
    int32_t varialbleId() const { return variableId_; }
    std::string varialbleValue() const { return variableValue_; }
private:
    std::shared_ptr<ZDevice> device_;
    int32_t variableId_;
    std::string variableValue_;
};

#endif /* ZDEVICEDATACHANGEINFORMATIONEVENT_H */

