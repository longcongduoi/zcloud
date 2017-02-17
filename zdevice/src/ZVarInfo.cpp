/* 
 * File:   ZVarInfo.cpp
 * Author: tamvh
 * 
 * Created on December 14, 2015, 10:15 AM
 */

#include "zdevice/ZVarInfo.h"

ZVarInfo::ZVarInfo() {
}

ZVarInfo::~ZVarInfo() {
}

bool ZVarInfo::operator==(const ZVarInfo& other) {
    return varId_ == other.varId_ &&
            deviceId_ == other.deviceId_ &&
            varName_ == other.varName_;
}

bool ZVarInfo::operator!=(const ZVarInfo& other) {
    return varId_ == other.varId_ ||
            deviceId_ == other.deviceId_ ||
            varName_ == other.varName_;
}

void ZVarInfo::setDeviceId(const int32_t value) {
    this->deviceId_ = value;
}

int32_t ZVarInfo::deviceId() const {
    return deviceId_;
}

void ZVarInfo::setVarId(const int32_t value) {
    this->varId_ = value;
}

int32_t ZVarInfo::varId() const {
    return varId_;
}

void ZVarInfo::setVarName(const std::string& value) {
    this->varName_ = value;
}

std::string ZVarInfo::varName() const {
    return varName_;
}

void ZVarInfo::setVarValue(const std::string& value) {
    this->varValue_ = value;
}

std::string ZVarInfo::varValue() const {
    return varValue_;
}

