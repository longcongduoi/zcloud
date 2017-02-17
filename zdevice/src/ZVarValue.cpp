/*
 * File:   ZVarValue.cpp
 * Author: tamvh
 *
 * Created on December 14, 2015, 11:06 AM
 */

#include "zdevice/ZVarValue.h"

ZVarValue::ZVarValue() {
}

ZVarValue::~ZVarValue() {
}

bool ZVarValue::operator==(const ZVarValue& other) {
    return varId_ == other.varId_ &&
            timeStamp_ == other.timeStamp_ &&
            value_ == other.value_;
}

bool ZVarValue::operator!=(const ZVarValue& other) {
    return varId_ == other.varId_ ||
            timeStamp_ == other.timeStamp_ ||
            value_ == other.value_;
}

void ZVarValue::setVarId(const int32_t value) {
    this->varId_ = value;
}

int32_t ZVarValue::varId() const {
    return varId_;
}

void ZVarValue::setTimeStamp(const uint64_t value) {
    this->timeStamp_ = value;
}

uint64_t ZVarValue::timeStamp() const {
    return timeStamp_;
}

void ZVarValue::setVarKey(const std::string& value) {
    this->varkey_ = value;
}

std::string ZVarValue::varKey() const {
    return varkey_;
}

void ZVarValue::setValue(const std::string& value) {
    this->value_ = value;
}

std::string ZVarValue::value() const {
    return value_;
}

std::string ZVarValue::toString() {
    return "{\"varId\": " + std::to_string(varId_) + 
            ", \"timeStamp\": \"" + std::to_string(timeStamp_) + 
            "\", \"value\": \"" + value_ + 
            "\"}";
}

