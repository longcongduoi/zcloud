/*
 * File:   ZApiHTTPWebRequestHandler.h
 * Author: hoanh
 *
 * Created on February 20, 2016
 */

#ifndef __ZAPIHTTPWEBREQUESTHANDLER_H__
#define __ZAPIHTTPWEBREQUESTHANDLER_H__

#include <memory>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>

#include <zapihttp/ZApiHTTPRequestBaseHandler.h>

class ZHTTPWebManager;

class ZApiHTTPWebRequestHandler : public ZApiHTTPRequestBaseHandler {
public:
    ZApiHTTPWebRequestHandler(const std::string& requestPath, ZHTTPWebManager* webManager);
    virtual ~ZApiHTTPWebRequestHandler();
public:
    static bool CanHandleRequest(const std::string& path, const std::string& method);
public:
    void handleRequest(Poco::Net::HTTPServerRequest& request, Poco::Net::HTTPServerResponse& response);
private:
    //! Add new widget to dashboard.
    //! HTTP POST /api/web/widgets
    //!
    //! POST body request
    //! {
    //!     "action": "widgets.add",
    //!     "type": 1,
    //!     "device_id": 12,
    //!     "variable_id": 1
    //! }
    //!
    //! POST response
    //! {
    //!     "data": {
    //!         "id": 11,
    //!         "type": 1,
    //!         "device_id": 12,
    //!         "variable_id": 1
    //!     }
    //! }
    Poco::Dynamic::Var handleAddWidget(Poco::JSON::Object::Ptr requestData, const std::string& userToken);

    //! Get a widget from dashboard.
    //! HTTP GET /api/web/widgets/11
    //!
    //! GET response
    //! {
    //!     "data": {
    //!         "id": 11,
    //!         "type": 1,
    //!         "device_id": 12,
    //!         "variable_id": 1
    //!     }
    //! }
    Poco::Dynamic::Var handleGetWidget(const std::string& userToken, int32_t widgetId);

    //! Remove a widget from dashboard.
    //! HTTP DELETE /api/web/widgets/11
    //!
    //! DELETE body request
    //! {
    //!     "action": "widgets.remove",
    //!     "id": 11
    //! }
    //!
    //! DELETE response
    //! {
    //! }
    Poco::Dynamic::Var handleRemoveWidget(const std::string& userToken, int32_t widgetId);

    //! Update widget's settings.
    //! HTTP POST /api/web/widgets/11
    //!
    //! POST body request
    //! {    
    //! }
    //!
    //! POST response
    //! {
    //! }
    Poco::Dynamic::Var handleUpdateWidget(Poco::JSON::Object::Ptr requestData, const std::string& userToken, int32_t widgetId);

    //! Get widgets from dashboard.
    //! HTTP GET /api/web/widgets
    //!
    //! GET response
    //! [
    //!     {
    //!         "id": 11,
    //!         "type": 1,
    //!         "device_id": 12,
    //!         "variable_id": 1
    //!     },{
    //!         "id": 12,
    //!         "type": 1,
    //!         "device_id": 12,
    //!         "variable_id": 1
    //!     },{
    //!         "id": 14,
    //!         "type": 1,
    //!         "device_id": 12,
    //!         "variable_id": 1
    //!     }
    //! ]
    Poco::Dynamic::Var handleGetAllWidgets(const std::string& userToken);

    //! Remove all widgets from dashboard.
    //! HTTP DELETE /api/web/widgets/all
    //! DELETE body request
    //! {
    //!     "action": "widgets.removeall"
    //! }
    //!
    //! DELETE response
    //! {
    //! }
    Poco::Dynamic::Var handleRemoveAllWidgets(Poco::JSON::Object::Ptr requestData, const std::string& userToken);
private:
    ZHTTPWebManager* webManager_;
};

#endif // __ZAPIHTTPWEBREQUESTHANDLER_H__
