/*
 * File:   ZApiHTTPAdminRequestHandler.h
 * Author: tamvh
 *
 * Created on November 25, 2015, 5:05 PM
 */

#ifndef ZAPIHTTPADMINREQUESTHANDLER_H
#define	ZAPIHTTPADMINREQUESTHANDLER_H

#include <memory>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Object.h>
#include <Poco/Dynamic/Var.h>

#include <zcommon/ErrorCode.h>
#include <zuser/ZUserInfo.h>
#include <zdevice/ZDeviceInfo.h>
#include <zdevice/ZVarLastestValue.h>

#include <zapihttp/ZApiHTTPRequestBaseHandler.h>

class ZAdminInfo;
class ZApiHTTPAdminRequestHandler : public ZApiHTTPRequestBaseHandler {
public:
    ZApiHTTPAdminRequestHandler(const std::string& requestPath);
    virtual ~ZApiHTTPAdminRequestHandler();
public:
    static bool CanHandleRequest(const std::string& path, const std::string& method);
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
private:
    Poco::Dynamic::Var handlePostAdminRegister(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handlePostAdminLogin(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handleGetAdminUser(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handleGetAdminUserApikey(Poco::Net::HTTPServerRequest& request, std::string apikey);
    Poco::Dynamic::Var handleDeleteAdminUserApikey(Poco::Net::HTTPServerRequest& request, std::string apikey);
    Poco::Dynamic::Var handleGetAdminDevices(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handleGetAdminDevicesDeviceId(Poco::Net::HTTPServerRequest& request, std::string deviceId);
    Poco::Dynamic::Var handleGetAdminFactoryDevices(Poco::Net::HTTPServerRequest& request);
    Poco::Dynamic::Var handlePostAdminFactoryDevicesCreate(Poco::Net::HTTPServerRequest& request);
private:
    void fillJsonFactoryDeviceInfo(const std::shared_ptr<ZDeviceInfo> &device, Poco::JSON::Object::Ptr& responseData);
    void fillJsonDeviceInfo(const std::shared_ptr<ZDeviceInfo> &device, Poco::JSON::Object::Ptr& responseData);
    void fillJsonDetailDeviceInfo(const std::shared_ptr<ZDeviceInfo> &device, Poco::JSON::Array::Ptr & data, Poco::JSON::Object::Ptr& responseData);
    void fillJsonVarLastestValue(const std::shared_ptr<ZVarLastestValue> &varLastestValue, Poco::JSON::Object::Ptr& responseData);
    void fillJsonUserInfo(const std::shared_ptr<ZUserInfo> &userInfo, Poco::JSON::Object::Ptr& responseData);
    void fillJson(const std::shared_ptr<ZAdminInfo> &adminInfo, Poco::JSON::Object::Ptr& responseData);
    std::string generateAccountClaims(const std::shared_ptr<ZAdminInfo> &adminInfo, Poco::JSON::Object::Ptr& responseData) const;

};

#endif	/* ZAPIHTTPADMINREQUESTHANDLER_H */

