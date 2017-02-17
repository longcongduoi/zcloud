/*
 * File:   ZApiHTTPRequestBaseHandler.h
 * Author: huuhoa
 *
 * Created on November 8, 2015, 9:11 AM
 */

#ifndef ZAPIHTTPREQUESTBASEHANDLER_H
#define	ZAPIHTTPREQUESTBASEHANDLER_H

#include <memory>

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/JSON/Object.h>
#include <Poco/Dynamic/Var.h>

#include <zcommon/ErrorCode.h>
#include <zcommon/ApiMessage.h>

enum class HttpApiError {
    InvalidRequest = (int) ZErrorCode::APIHTTP_Start,
    ParameterMissing,
    AuthenticationMissing,
    NotImplemented,
    Unknown = 1999
};

//class HttpErrorMessage{
//public:
//    static ApiMessage INVALID_REQUEST = new ApiMessage(HttpApiError::InvalidRequest, "Invalid request");
//    static ApiMessage PARAMETER_MISSING = new ApiMessage(HttpApiError::ParameterMissing, "Parameter Missing");    
//    static ApiMessage AUTHENTICATION_MISSING = new ApiMessage(HttpApiError::AuthenticationMissing, "Authentication Missing");
//    static ApiMessage NOT_IMPLEMENTED = new ApiMessage(HttpApiError::NotImplemented, "Not Implemented");
//    static ApiMessage UNKNOWN = new ApiMessage(HttpApiError::Unknown, "Unknown");
//};

class ReportError {
public:
    static void reportTo(HttpApiError error, Poco::JSON::Object::Ptr& responseData);
};

class ZApiHTTPRequestBaseHandler : public Poco::Net::HTTPRequestHandler {
public:
    ZApiHTTPRequestBaseHandler(const std::string& path);
    virtual ~ZApiHTTPRequestBaseHandler();
protected:
    Poco::Dynamic::Var parseServerRequest(Poco::Net::HTTPServerRequest& request, Poco::JSON::Object::Ptr& responseData);

    const std::string& requestPath() const {
        return _requestPath;
    }
    int32_t getAuthenticatedUser(Poco::Net::HTTPServerRequest& request, Poco::JSON::Object::Ptr& responseData) const;
    std::string getAuthenticationToken(Poco::Net::HTTPServerRequest& request, Poco::JSON::Object::Ptr& responseData) const;
    int32_t getUserDeviceId(const std::string& path) const;
    int32_t getAdminUserDeviceId(const std::string& path) const;
    std::string getAdminUserApikey(const std::string& path) const;
private:
    std::string _requestPath;
};

#endif	/* ZAPIHTTPREQUESTBASEHANDLER_H */

