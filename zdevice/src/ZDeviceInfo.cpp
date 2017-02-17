/*
 * File:   ZDeviceInfo.cpp
 * Author: huuhoa
 *
 * Created on October 27, 2015, 5:21 PM
 */

#include <zdevice/ZDeviceInfo.h>

ZDeviceInfo::ZDeviceInfo() :
    deviceId_(0),
    deviceType_(0),
    createdAt_(0),
    updatedAt_(0)
{
}

ZDeviceInfo::~ZDeviceInfo() {}

bool ZDeviceInfo::operator ==(const ZDeviceInfo& other) {
    return deviceId_ == other.deviceId_ &&
            deviceName_ == other.deviceName_ &&
            deviceType_ == other.deviceType_ &&
            deviceGroup_ == other.deviceGroup_ &&
            createdAt_ == other.createdAt_ &&
            apiKey_ == other.apiKey_;
}

bool ZDeviceInfo::operator !=(const ZDeviceInfo& other) {
    return deviceId_ != other.deviceId_ ||
            deviceName_ != other.deviceName_ ||
            deviceType_ != other.deviceType_ ||
            deviceGroup_ != other.deviceGroup_ ||
            createdAt_ != other.createdAt_ ||
            apiKey_ != other.apiKey_;
}

void ZDeviceInfo::setApiKey(const std::string& value) {
    this->apiKey_ = value;
}

std::string ZDeviceInfo::apiKey() const {
    return apiKey_;
}

void ZDeviceInfo::setUpdatedAt(const uint64_t value) {
    this->updatedAt_ = value;
}

uint64_t ZDeviceInfo::updatedAt() const {
    return updatedAt_;
}

void ZDeviceInfo::setCreatedAt(const uint64_t value) {
    this->createdAt_ = value;
}

uint64_t ZDeviceInfo::createdAt() const {
    return createdAt_;
}

void ZDeviceInfo::setDeviceGroup(const std::string& value) {
    this->deviceGroup_ = value;
}

std::string ZDeviceInfo::deviceGroup() const {
    return deviceGroup_;
}

void ZDeviceInfo::setDeviceType(const int32_t value) {
    this->deviceType_ = value;
}

int32_t ZDeviceInfo::deviceType() const {
    return deviceType_;
}

void ZDeviceInfo::setDeviceName(const std::string& value) {
    this->deviceName_ = value;
}

std::string ZDeviceInfo::deviceName() const {
    return deviceName_;
}

void ZDeviceInfo::setDeviceAddress(const std::string& value) {
    this->deviceAddress_ = value;
}

std::string ZDeviceInfo::deviceAddress() const {
    return deviceAddress_;
}

void ZDeviceInfo::setDeviceId(const int32_t value) {
    this->deviceId_ = value;
}

int32_t ZDeviceInfo::deviceId() const {
    return deviceId_;
}

void ZDeviceInfo::setSecurityNumber(const std::string& value) {
    this->securityNumber_ = value;
}

std::string ZDeviceInfo::securityNumber() {
    return securityNumber_;
}

