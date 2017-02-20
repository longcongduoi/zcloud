/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   newClass.h
 * Author: tamvh
 *
 * Created on February 20, 2017, 1:23 PM
 */

#ifndef NEWCLASS_H
#define NEWCLASS_H
#include <memory>
#include <string>
#include <iostream>
#include <system_error>
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
#include <Poco/Util/Application.h>
#include <Poco/Util/LayeredConfiguration.h>
#include <Poco/RegularExpression.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/Delegate.h>
#include <Poco/StringTokenizer.h>
#include <zcommon/ZServiceInterface.h>
#include <zcommon/ZServiceLocator.h>
class ZCloudReceiveMessageEvent;
class ZAPIHttpClient : public ZServiceInterface{
public:
    virtual bool initialize();
    virtual bool start();
    virtual bool stop();
    virtual bool cleanup();
public:
    ZAPIHttpClient();
    virtual ~ZAPIHttpClient();
public:
    void handleRecvMessage(const void *pSender, const ZCloudReceiveMessageEvent &event);
private:
    class Impl;
    Poco::SharedPtr<Impl> d_ptr;
};

#endif /* NEWCLASS_H */

