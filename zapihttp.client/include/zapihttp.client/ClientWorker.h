/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ClientWorker.h
 * Author: tamvh
 *
 * Created on February 20, 2017, 4:00 PM
 */

#ifndef CLIENTWORKER_H
#define CLIENTWORKER_H
#include <memory>
#include <string>
#include <iostream>
#include <stdio.h>  
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
class ClientWorker {
public:
    ClientWorker(const std::string& server,
        const std::string& api);
    virtual ~ClientWorker();
public:
    void setMethod(const std::string method);
    void setParam(const std::string key, const std::string value);
    void sendRequest();
    std::string responseData();
    void doGet();
    void doPost();
private:
    class Impl;
    Poco::SharedPtr<Impl> d_ptr;
};

#endif /* CLIENTWORKER_H */

