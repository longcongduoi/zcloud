/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ZApiHTTPClient.h
 * Author: tamvh
 *
 * Created on February 6, 2017, 2:39 PM
 */

#ifndef ZAPIHTTPCLIENT_H
#define ZAPIHTTPCLIENT_H
#include <string>
#include <memory>
#include <iostream>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/StreamCopier.h>
#include <Poco/Path.h>
#include <Poco/URI.h>
#include <Poco/Exception.h>

class ZApiHTTPClient {
public:
    ZApiHTTPClient();
    virtual ~ZApiHTTPClient();
public:
    void setHost(const std::string host);
    void setPort(int16_t port);
    void setMethod(const std::string method);
    void setApi(const std::string api);
    void setParam(const std::string key, const std::string value);
    void setReqBody();
    void sendRequest();
    std::string responseData();
private:
    class Impl;
    std::shared_ptr<Impl> d_ptr;
};

#endif /* ZAPIHTTPCLIENT_H */

