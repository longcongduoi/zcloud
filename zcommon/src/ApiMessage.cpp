/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ApiMessage.cpp
 * Author: tamvh
 * 
 * Created on December 12, 2016, 3:51 PM
 */

#include "zcommon/ApiMessage.h"

ApiMessage::ApiMessage(int32_t error, const std::string& msg) {
    this->_error = error;
    this->_msg = msg;
}

ApiMessage::~ApiMessage() {
}

void ApiMessage::setErrorCode(const int32_t value) {
    this->_error = value;
}

int32_t ApiMessage::getErrorCode() const {
    return this->_error;
}
    
void ApiMessage::setMsg(const std::string& value) {
    this->_msg = value;
}

std::string ApiMessage::getMsg() const {
    return this->_msg;
}

