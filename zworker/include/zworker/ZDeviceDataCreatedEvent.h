/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ZDeviceDataCreatedEvent.h
 * Author: tamvh
 *
 * Created on February 19, 2016, 11:16 AM
 */

#ifndef ZDEVICEDATACREATEDEVENT_H
#define ZDEVICEDATACREATEDEVENT_H

#include <memory>
class ZDevice;

class ZDeviceDataCreatedEvent {
public:
    ZDeviceDataCreatedEvent(std::shared_ptr<ZDevice> device, int32_t varId, const std::string& varValue)
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

#endif /* ZDEVICEDATACREATEDEVENT_H */

