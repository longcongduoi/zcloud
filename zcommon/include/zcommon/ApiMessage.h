/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ApiMessage.h
 * Author: tamvh
 *
 * Created on December 12, 2016, 3:51 PM
 */

#ifndef APIMESSAGE_H
#define APIMESSAGE_H

#include <string>
#include <Poco/JSON/Object.h>
class ApiMessage {
public:
    ApiMessage(int32_t error, const std::string& msg);
    virtual ~ApiMessage();
    
    void setErrorCode(const int32_t value);
    int32_t getErrorCode() const;
    
    void setMsg(const std::string& value);
    std::string getMsg() const;
private:
    int32_t _error;
    std::string _msg;
};

#endif /* APIMESSAGE_H */

