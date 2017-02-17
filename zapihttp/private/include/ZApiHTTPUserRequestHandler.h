/*
 * File:   ZApiHTTPUserRequestHandler.h
 * Author: huuhoa
 *
 * Created on November 2, 2015, 4:00 PM
 */

#ifndef ZAPIHTTPUSERREQUESTHANDLER_H
#define	ZAPIHTTPUSERREQUESTHANDLER_H

#include <memory>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Object.h>
#include <Poco/Dynamic/Var.h>
#include <zcommon/ErrorCode.h>

#include <zapihttp/ZApiHTTPRequestBaseHandler.h>

class ZUserInfo;
class ZApiHTTPUserRequestHandler : public ZApiHTTPRequestBaseHandler {
public:
    ZApiHTTPUserRequestHandler(const std::string& path);
    virtual ~ZApiHTTPUserRequestHandler();
public:
    static bool CanHandleRequest(const std::string& path, const std::string& method);
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
private:
    void handleUserRegister(Poco::Net::HTTPServerRequest& request, Poco::JSON::Object::Ptr& responseData);
    void handleUserLogin(Poco::Net::HTTPServerRequest& request, Poco::JSON::Object::Ptr& responseData);
    void handleUserPassword(Poco::Net::HTTPServerRequest& request, Poco::JSON::Object::Ptr& responseData);
    void handleUserResetPassword(Poco::Net::HTTPServerRequest& request, Poco::JSON::Object::Ptr& responseData);
    void handleUserRemove(Poco::Net::HTTPServerRequest& request, Poco::JSON::Object::Ptr& responseData);
private:
    void fillJson(const std::shared_ptr<ZUserInfo> &userInfo, Poco::JSON::Object::Ptr& responseData);
    std::string generateAccountClaims(const std::shared_ptr<ZUserInfo> &userInfo, Poco::JSON::Object::Ptr& responseData) const;
};

#endif	/* ZAPIHTTPUSERREQUESTHANDLER_H */

