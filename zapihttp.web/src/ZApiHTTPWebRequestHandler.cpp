/*
 * File:   ZApiHTTPWebRequestHandler.cpp
 * Author: hoanh
 *
 * Created on February 20, 2016
 */

#include <Poco/RegularExpression.h>
#include <Poco/Util/Application.h>

#include <zcommon/ZServiceLocator.h>
#include <zworker/ZWorker.h>

#include <zapihttp.web/ZHTTPWebManager.h>
#include "ZHTTPWidget.h"
#include <zapihttp.web/ZApiHTTPWebRequestHandler.h>

using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Util::Application;
using Poco::JSON::Object;
using Poco::JSON::Array;
using Poco::Dynamic::Var;

namespace {

    Object::Ptr widgetToJson(std::shared_ptr<ZHTTPWidget> widget) {
        Object::Ptr result = new Object;
        if (!widget) {
            return result;
        }

        result->set("id", widget->widgetId());
        result->set("type", widget->widgetType());
        result->set("name", widget->widgetName());
        result->set("device_id", widget->deviceId());
        result->set("size", widget->widgetSize());

        return result;
    }

    enum class WebError {
        Start = (int) ZErrorCode::APIHTTP_Start + 100,
        CannotUpdateWidget,
        CannotRemoveWidget,
        WidgetNotFound,
        CannotAddWidget,
    };
}

ZApiHTTPWebRequestHandler::ZApiHTTPWebRequestHandler(const std::string& requestPath,
        ZHTTPWebManager* webManager) :
ZApiHTTPRequestBaseHandler(requestPath),
webManager_(webManager) {

}

ZApiHTTPWebRequestHandler::~ZApiHTTPWebRequestHandler() {

}

bool ZApiHTTPWebRequestHandler::CanHandleRequest(const std::string& path, const std::string& method) {
    // /web/widget[/:deviceId]

    if (path.compare("/web/widget") == 0 &&
            ((method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) ||
            (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0))) {
        return true;
    }

    if (path.compare("/web/widget/all") == 0 &&
            (method.compare(Poco::Net::HTTPRequest::HTTP_DELETE) == 0)) {
        return true;
    }

    Poco::RegularExpression regex("/web/widget/[0-9]+");
    if ((regex == path) &&
            ((method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) ||
            (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) ||
            (method.compare(Poco::Net::HTTPRequest::HTTP_DELETE) == 0))) {
        return true;
    }

    return false;
}

enum class ProcessingExceptionType {
    MissingAuthentication,
    InvalidAuthentication,
    InvalidDataRequest,

};

class ProcessingException : public std::exception {
public:

    ProcessingException(ProcessingExceptionType what) : cause_(what) {

    }

    ProcessingExceptionType cause() const {
        return cause_;
    }
    //    virtual const char* what() const _NOEXCEPT {

    virtual const char* what() {
        switch (cause_) {
            case ProcessingExceptionType::InvalidAuthentication:
                return "Invalid authentication";
            case ProcessingExceptionType::MissingAuthentication:
                return "Missing authentication token";
            case ProcessingExceptionType::InvalidDataRequest:
                return "Invalid data in HTTP request";
            default:
                return "Unknown error";
        }
    }
private:
    ProcessingExceptionType cause_;
};

void ZApiHTTPWebRequestHandler::handleRequest(HTTPServerRequest& request,
        HTTPServerResponse& response) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;

    Application& app = Application::instance();

    response.setChunkedTransferEncoding(true);
    response.setContentType("application/json");

    Poco::Dynamic::Var responseData;
    try {
        Poco::JSON::Object::Ptr data = new Poco::JSON::Object;
        std::string userToken = getAuthenticationToken(request, data);
        if (userToken.empty()) {
            app.logger().error("Missing authentication token");
            throw new ProcessingException(ProcessingExceptionType::MissingAuthentication);
        }

        const std::string& method = request.getMethod();
        Object::Ptr object;
        if (method.compare(Poco::Net::HTTPRequest::HTTP_GET) != 0 &&
                method.compare(Poco::Net::HTTPRequest::HTTP_DELETE) != 0) {
            Var result = parseServerRequest(request, data);
            if (result.isEmpty()) {
                app.logger().error("Invalid/Empty data");
                throw new ProcessingException(ProcessingExceptionType::InvalidDataRequest);
            }

            object = result.extract<Object::Ptr>();
        }

        if (requestPath().compare("/web/widget") == 0) {
            if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
                responseData = handleAddWidget(object, userToken);
            } else if (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
                responseData = handleGetAllWidgets(userToken);
            }
        } else if (requestPath().compare("/web/widget/all") == 0) {
            if (method.compare(Poco::Net::HTTPRequest::HTTP_DELETE) == 0) {
                responseData = handleRemoveAllWidgets(object, userToken);
            }
        } else {
            Poco::RegularExpression regex("/web/widget/([0-9]+)");
            Poco::RegularExpression::MatchVec matchVec;
            int numMatches = regex.match(requestPath(), 0, matchVec);
            if (numMatches >= 1) {
                Poco::RegularExpression::Match m = matchVec[1];
                int32_t widgetId = Poco::NumberParser::parse(requestPath().substr(m.offset, m.length));
                if (method.compare(Poco::Net::HTTPRequest::HTTP_POST) == 0) {
                    responseData = handleUpdateWidget(object, userToken, widgetId);
                } else if (method.compare(Poco::Net::HTTPRequest::HTTP_GET) == 0) {
                    responseData = handleGetWidget(userToken, widgetId);
                } else if (method.compare(Poco::Net::HTTPRequest::HTTP_DELETE) == 0) {
                    responseData = handleRemoveWidget(userToken, widgetId);
                }
            }
        }
        // validate user authentication
    } catch (Poco::Exception ex) {
        app.logger().error("Exception while processing message: %s", ex.displayText());
        Poco::JSON::Object::Ptr data = new Poco::JSON::Object;
        ReportError::reportTo(HttpApiError::Unknown, data);
        responseData = data;
    } catch (ProcessingException ex) {
        app.logger().error("Exception while processing message: %s", ex.what());
        Poco::JSON::Object::Ptr data = new Poco::JSON::Object;
        ReportError::reportTo(HttpApiError::ParameterMissing, data);
        responseData = data;
    }

    if (responseData.isEmpty()) {
        Poco::JSON::Object::Ptr unimplementedData = new Poco::JSON::Object;
        ReportError::reportTo(HttpApiError::NotImplemented, unimplementedData);
        responseData = unimplementedData;
    }

    std::ostream& ostr = response.send();
    Poco::JSON::Stringifier::stringify(responseData, ostr);
}

Poco::Dynamic::Var ZApiHTTPWebRequestHandler::handleAddWidget(
        Poco::JSON::Object::Ptr requestData,
        const std::string& userToken) {
    Object::Ptr responseData = new Object;
    std::string action = requestData->optValue("action", std::string());
    int32_t wtype = requestData->optValue("type", -1);
    int32_t deviceId = requestData->optValue("device_id", -1);
    int32_t wsize = requestData->optValue("size", -1);
    std::string name = requestData->optValue("name", std::string());

    if (action.empty() || action.compare("widgets.add") != 0 ||
            deviceId < 0 || wtype < 0) {
        ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
        return responseData;
    }

    ZWorker* worker = ZServiceLocator::instance()->
            get<ZWorker>(ZServiceLocator::ServiceId::Worker);
    ZWorkerResult authResult = worker->validateUserAuthentication(userToken);
    if (authResult.failed()) {
        ReportError::reportTo(static_cast<HttpApiError> (authResult.errorCode()), responseData);
        return responseData;
    }

    const int32_t &userId = authResult.extract<int32_t>();
    std::shared_ptr<ZHTTPWidget> widget(new ZHTTPWidget);
    widget->setWidgetType(wtype);
    widget->setWidgetName(name);
    widget->setDeviceId(deviceId);
    widget->setWidgetSize(wsize);

    if (!webManager_->addWidget(userId, widget)) {
        ReportError::reportTo(static_cast<HttpApiError> (WebError::CannotUpdateWidget), responseData);
        return responseData;
    }

    Object::Ptr data = widgetToJson(widget);
    responseData->set("data", data);
    // return '{ "data" : widget }'
    return responseData;
}

Poco::Dynamic::Var ZApiHTTPWebRequestHandler::handleGetWidget(
        const std::string& userToken,
        int32_t widgetId) {
    Object::Ptr responseData = new Object;

    ZWorker* worker = ZServiceLocator::instance()->
            get<ZWorker>(ZServiceLocator::ServiceId::Worker);
    ZWorkerResult authResult = worker->validateUserAuthentication(userToken);
    if (authResult.failed()) {
        ReportError::reportTo(static_cast<HttpApiError> (authResult.errorCode()), responseData);
        return responseData;
    }

    const int32_t &userId = authResult.extract<int32_t>();

    std::shared_ptr<ZHTTPWidget> widget = webManager_->getWidget(userId, widgetId);
    if (!widget) {
        ReportError::reportTo(static_cast<HttpApiError> (WebError::WidgetNotFound), responseData);
        return responseData;
    }

    Object::Ptr data = widgetToJson(widget);
    responseData->set("data", data);
    // return '{ "data": widget }'
    return responseData;
}

Poco::Dynamic::Var ZApiHTTPWebRequestHandler::handleRemoveWidget(
        const std::string& userToken,
        int32_t widgetId) {
    Object::Ptr responseData = new Object;

    if (widgetId < 0) {
        ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
        return responseData;
    }

    ZWorker* worker = ZServiceLocator::instance()->
            get<ZWorker>(ZServiceLocator::ServiceId::Worker);
    ZWorkerResult authResult = worker->validateUserAuthentication(userToken);
    if (authResult.failed()) {
        ReportError::reportTo(static_cast<HttpApiError> (authResult.errorCode()), responseData);
        return responseData;
    }

    const int32_t &userId = authResult.extract<int32_t>();

    if (!webManager_->removeWidget(userId, widgetId)) {
        ReportError::reportTo(static_cast<HttpApiError> (WebError::CannotRemoveWidget), responseData);
        return responseData;
    }

    // return '{}'
    return responseData;
}

Poco::Dynamic::Var ZApiHTTPWebRequestHandler::handleUpdateWidget(
        Poco::JSON::Object::Ptr requestData,
        const std::string& userToken,
        int32_t widgetId) {
    Object::Ptr responseData = new Object;
    std::string action = requestData->optValue("action", std::string());
    int32_t wid = requestData->optValue("id", -1);
    int32_t deviceId = requestData->optValue("device_id", -1);
    int32_t wsize = requestData->optValue("size", -1);
    std::string name = requestData->optValue("name", std::string());

    if (action.empty() || action.compare("widgets.update") != 0 ||
            wid < 0 || deviceId < 0 || 
            wid != widgetId) {
        ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
        return responseData;
    }

    ZWorker* worker = ZServiceLocator::instance()->
            get<ZWorker>(ZServiceLocator::ServiceId::Worker);
    ZWorkerResult authResult = worker->validateUserAuthentication(userToken);
    if (authResult.failed()) {
        ReportError::reportTo(static_cast<HttpApiError> (authResult.errorCode()), responseData);
        return responseData;
    }

    const int32_t &userId = authResult.extract<int32_t>();
    std::shared_ptr<ZHTTPWidget> widget(new ZHTTPWidget);
    widget->setWidgetId(widgetId);
    widget->setWidgetName(name);
    widget->setDeviceId(deviceId);
    widget->setWidgetSize(wsize);

    if (!webManager_->updateWidget(userId, widget)) {
        ReportError::reportTo(static_cast<HttpApiError> (WebError::CannotUpdateWidget), responseData);
        return responseData;
    }

    // return '{}'
    return responseData;
}

Poco::Dynamic::Var ZApiHTTPWebRequestHandler::handleGetAllWidgets(
        const std::string& userToken) {

    Object::Ptr responseData = new Object;

    ZWorker* worker = ZServiceLocator::instance()->
            get<ZWorker>(ZServiceLocator::ServiceId::Worker);
    ZWorkerResult authResult = worker->validateUserAuthentication(userToken);
    if (authResult.failed()) {
        ReportError::reportTo(static_cast<HttpApiError> (authResult.errorCode()), responseData);
        return responseData;
    }

    const int32_t &userId = authResult.extract<int32_t>();
    std::vector<std::shared_ptr < ZHTTPWidget>> widgets = webManager_->getAllWidgets(userId);
    Array::Ptr data = new Array;
    for (std::shared_ptr<ZHTTPWidget> widget : widgets) {
        data->add(widgetToJson(widget));
    }

    // return '[ {w1}, {w2}, ...]'
    return data;
}

Poco::Dynamic::Var ZApiHTTPWebRequestHandler::handleRemoveAllWidgets(
        Poco::JSON::Object::Ptr requestData,
        const std::string& userToken) {
    Object::Ptr responseData = new Object;
    try {
        std::string action = requestData->optValue("action", std::string());

        if (action.empty() || action.compare("widgets.removeall") != 0) {
            ReportError::reportTo(HttpApiError::ParameterMissing, responseData);
            return responseData;
        }

        ZWorker* worker = ZServiceLocator::instance()->
                get<ZWorker>(ZServiceLocator::ServiceId::Worker);
        ZWorkerResult authResult = worker->validateUserAuthentication(userToken);
        if (authResult.failed()) {
            ReportError::reportTo(static_cast<HttpApiError> (authResult.errorCode()), responseData);
            return responseData;
        }

        const int32_t &userId = authResult.extract<int32_t>();
        webManager_->removeAllWidgets(userId);
        // return "{}"
        return responseData;
    } catch (Poco::Exception &ex) {

        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
        ReportError::reportTo(HttpApiError::Unknown, responseData);
    }

    return responseData;
}
