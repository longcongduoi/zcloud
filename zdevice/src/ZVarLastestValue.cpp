/* 
 * File:   ZVarLastestValue.cpp
 * Author: tamvh
 * 
 * Created on December 14, 2015, 11:27 AM
 */

#include "zdevice/ZVarLastestValue.h"

ZVarLastestValue::ZVarLastestValue() {
}

ZVarLastestValue::~ZVarLastestValue() {
}

bool ZVarLastestValue::operator==(const ZVarLastestValue& other) {
    return varId_ == other.varId_ &&
            value_ == other.value_;
}

bool ZVarLastestValue::operator!=(const ZVarLastestValue& other) {
    return varId_ == other.varId_ ||
            value_ == other.value_;
}

void ZVarLastestValue::setVarId(const int32_t value) {
    this->varId_ = value;
}

int32_t ZVarLastestValue::varId() const {
    return varId_;
}

void ZVarLastestValue::setValue(const std::string& value) {
    this->value_ = value;
}

std::string ZVarLastestValue::value() const {
    return value_;
}
