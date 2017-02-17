/*
 * File:   ZApiHTTPDeviceRequestHandler.h
 * Author: huuhoa
 *
 * Created on November 8, 2015, 9:09 AM
 */

#ifndef ZAPIHTTPDEVICEREQUESTHANDLER_H
#define	ZAPIHTTPDEVICEREQUESTHANDLER_H

#include <memory>
#include <Poco/Dynamic/Var.h>
#include <zdevice/ZDeviceInfo.h>
#include <zdevice/ZVarLastestValue.h>
#include <zdevice/ZVarValue.h>
#include <zdevice/ZDevice.h>
#include <zdevice/ZVarInfo.h>

#include <zapihttp/ZApiHTTPRequestBaseHandler.h>

class ZApiHTTPDeviceRequestHandler : public ZApiHTTPRequestBaseHandler {
public:
    ZApiHTTPDeviceRequestHandler(const std::string& requestPath);
    virtual ~ZApiHTTPDeviceRequestHandler();
public:
    static bool CanHandleRequest(const std::string& path, const std::string& method);
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
private:
    Poco::Dynamic::Var handleAddDIYDevice(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handleGetAllUserDevices(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handleAddIndieDevice(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handleUpdateDeviceWithId(Poco::Net::HTTPServerRequest& request, const std::string& deviceId);
    Poco::Dynamic::Var handleGetDeviceWithId(Poco::Net::HTTPServerRequest& request, const std::string& deviceId);
    Poco::Dynamic::Var handleDeleteDeviceWithId(Poco::Net::HTTPServerRequest& request, const std::string& deviceId);
    Poco::Dynamic::Var handleAPIHttpRegister(Poco::JSON::Object::Ptr& params, const std::string& token);
    Poco::Dynamic::Var handleAPIHttpUpdate(Poco::JSON::Object::Ptr& params, const std::string& token);
    Poco::Dynamic::Var handleAPIHttpQuery(Poco::JSON::Object::Ptr& params, const std::string& token);
    Poco::Dynamic::Var handleDeviceHistory(Poco::Net::HTTPServerRequest& request, const std::string& deviceid, const std::string& count);
    Poco::Dynamic::Var handleDeleteDeviceHistory(Poco::Net::HTTPServerRequest& request, const std::string& deviceid);
private:
    void fillJson(const std::shared_ptr<ZDeviceInfo> &device, Poco::JSON::Object::Ptr& responseData);
    void fillJsonAllDevices(
            const std::shared_ptr<ZDeviceInfo> &device, 
            ZVarInfo::Ptr& varInfo, 
            const std::string& localTime, 
            const std::string& battery, 
            Poco::JSON::Object::Ptr& responseData);
    void fillJsonVarLastestValue(const std::shared_ptr<ZVarLastestValue> &varLastestValue, Poco::JSON::Object::Ptr& responseData);
    void fillJsonVarValue(const std::shared_ptr<ZVarValue> &varValue, Poco::JSON::Object::Ptr& responseData);
    void fillJsonVarValue(const std::string& value, Poco::JSON::Object::Ptr& responseData);
    void fillJsonVarValue(const std::string& value, const std::string& timestamp, Poco::JSON::Object::Ptr & responseData);
    void fillJsonVarInfo(const std::shared_ptr<ZVarLastestValue> &varLastestValue, const std::string& varName, Poco::JSON::Object::Ptr& responseData);
    void fillJsonDetailDevice(const std::shared_ptr<ZDevice> &device, ZVarInfo::Ptr & varInfo, const std::string localTime, Poco::JSON::Object::Ptr& responseData);
    void fillJsonHttpUpdate(const std::shared_ptr<ZDevice> &device, Poco::JSON::Object::Ptr& responseData);
    void fillJsonHttpQuery(const std::shared_ptr<ZDevice> &device, Poco::JSON::Array::Ptr & data, Poco::JSON::Object::Ptr& responseData);
};

#endif	/* ZAPIHTTPDEVICEREQUESTHANDLER_H */

