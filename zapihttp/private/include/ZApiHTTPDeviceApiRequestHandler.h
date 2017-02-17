/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ZApiHTTPDeviceApiRequestHandler.h
 * Author: tamvh
 *
 * Created on December 14, 2016, 1:53 PM
 */

#ifndef ZAPIHTTPDEVICEAPIREQUESTHANDLER_H
#define ZAPIHTTPDEVICEAPIREQUESTHANDLER_H

#include <memory>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>
#include <zdevice/ZDeviceInfo.h>
#include <zdevice/ZVarLastestValue.h>
#include <zdevice/ZVarValue.h>
#include <zdevice/ZDevice.h>
#include <zdevice/ZVarInfo.h>
#include <zapihttp/ZApiHTTPRequestBaseHandler.h>
class ZApiHTTPDeviceApiRequestHandler : public ZApiHTTPRequestBaseHandler{
public:
    ZApiHTTPDeviceApiRequestHandler(const std::string& requestPath);
    virtual ~ZApiHTTPDeviceApiRequestHandler();
public:
    static bool CanHandleRequest(const std::string& path, const std::string& method);
    void handleRequest(
            Poco::Net::HTTPServerRequest& request, 
            Poco::Net::HTTPServerResponse& response);    
private:
    Poco::Dynamic::Var handleRegisterDevice(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handleUpdateStatusDevice(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handleDevicePushData(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handleQueryDevice(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handleDeviceControl(Poco::Net::HTTPServerRequest& request);
private:
    void fillJsonStatusDeviceUpdate( 
            const std::shared_ptr<ZDevice> &device,
            Poco::JSON::Object::Ptr & responseData);
    void fillJsonAllDevices(
            const std::shared_ptr<ZDeviceInfo> &device, 
            ZVarInfo::Ptr& varInfo, 
            const std::string& localTime, 
            Poco::JSON::Object::Ptr& responseData);
};

#endif /* ZAPIHTTPDEVICEAPIREQUESTHANDLER_H */

