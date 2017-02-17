/*
 * File:   ZApiMQTT.cpp
 * Author: huuhoa
 *
 * Created on October 24, 2015, 3:17 PM
 */
#include <sstream>

#include <Poco/Util/Application.h>
#include <Poco/Util/LayeredConfiguration.h>
#include <Poco/RegularExpression.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/Delegate.h>

#include <zcommon/ZServiceInterface.h>
#include <zcommon/ZServiceLocator.h>
#include <zcommon/ZDBKey.h>
#include <zcommon/ErrorCode.h>
#include <zdb/ZDBProxy.h>
#include <zworker/ZWorker.h>
#include <zworker/ZDeviceDataUpdatedEvent.h>
#include <zworker/ZCloudReceiveMessageEvent.h>
#include <zdevice/ZDevice.h>
#include <zdevice/ZDeviceInfo.h>
#include <zdevice/ZVarLastestValue.h>
#include <zdevice/ZDeviceManager.h>

#include "MqttClientWorker.h"
#include "MqttTopic.h"
#include "zapimqtt/ZApiMQTT.h"

#include "MQTTAsync.h"

using namespace std;
using Poco::Util::Application;
using Poco::Util::LayeredConfiguration;

class ZApiMQTT::Impl {
public:
    typedef std::map<int64_t, Poco::JSON::Object::Ptr> ObjectMap;
    ObjectMap list_object_request;
    Poco::SharedPtr<MqttClientWorker> mqttClientWorker;
};

ZApiMQTT::ZApiMQTT() : d_ptr(new Impl) {
}

ZApiMQTT::~ZApiMQTT() {
}

bool ZApiMQTT::initialize() {
    LayeredConfiguration& config = Application::instance().config();
    unsigned short port = (unsigned short) config.getInt("api.mqtt.port", 1883);
    std::string host = config.getString("api.mqtt.host", "localhost");
    std::string clientId = config.getString("api.mqtt.clientId", "");
    std::string topic = config.getString("api.mqtt.topic", "/u/#");

    d_ptr->mqttClientWorker = new MqttClientWorker(clientId, host, port);
    d_ptr->mqttClientWorker->preSubscribe(topic, 0);
    d_ptr->mqttClientWorker->autoReconnect(true);
    d_ptr->mqttClientWorker->beginConnect();
    d_ptr->mqttClientWorker->connect();

    ZWorker *worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);    
    worker->deviceDataUpdated += Poco::delegate(this, &ZApiMQTT::handlePublishMessage);
    worker->receiveMessageEvent += Poco::delegate(this, &ZApiMQTT::handleReceiveMessage);
    return true;
}

bool ZApiMQTT::start() {
    return true;
}

bool ZApiMQTT::stop() {
    return true;
}

bool ZApiMQTT::cleanup() {
    return true;
}

void ZApiMQTT::publishData(const std::string& topic, const std::string& msg) {    
    if (!topic.empty() && !msg.empty()) {
        d_ptr->mqttClientWorker->publish(topic, msg);
    }
}

//Server -> Device

void ZApiMQTT::handlePublishMessage(const void *pSender, const ZDeviceDataUpdatedEvent &event) {
    Poco::Dynamic::Var responseData;
    try {
        std::string apiName = event.apiName();
        if (apiName == "changeValue") {
            responseData = handleChangeValue(pSender, event);
        }
        if (apiName == "changeState") {
            responseData = handleChangeState(pSender, event);
        }
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
    }
}

Poco::Dynamic::Var ZApiMQTT::handleChangeValue(const void *pSender, const ZDeviceDataUpdatedEvent &event) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;
    Application& app = Application::instance();
    std::string msg_pub = "";
    std::string topic_request = "";
    Object::Ptr responseData = new Object;
    try {        
        Poco::JSON::Object::Ptr responseData = new Poco::JSON::Object;
        Poco::JSON::Object::Ptr item = new Poco::JSON::Object;
        Poco::JSON::Object::Ptr data = new Poco::JSON::Object;        
        data->set("deviceid", event.device()->info()->deviceId());
        data->set("name", event.device()->info()->deviceName());
        data->set("group", event.device()->info()->deviceGroup());
        data->set("type", event.device()->info()->deviceType());
        data->set("apikey", event.device()->info()->apiKey());
        data->set("createdAt", static_cast<Poco::Int64> (event.device()->info()->createdAt()));
        data->set("lastModified", static_cast<Poco::Int64> (event.device()->info()->updatedAt()));
        data->set("value", event.varialbleValue());
        data->set("localTime", event.timeStamp());
        data->set("battery", event.battery());
        item->set("action", "update");
        item->set("data", data);
        responseData->set("userApiKey", event.userApiKey());
        responseData->set("content", item);

        std::stringstream buffer;
        Poco::JSON::Stringifier::stringify(responseData, buffer);
        msg_pub = buffer.str();
        topic_request = MqttTopic::requestTopic(
                event.userApiKey(), 
                std::to_string(event.device()->info()->deviceId()),
                event.apiName());
        publishData(topic_request, msg_pub);
    } catch (Poco::Exception &ex) {        
        app.logger().error("Exception while processing message: %s", ex.displayText());
    }
    return responseData;
}

Poco::Dynamic::Var ZApiMQTT::handleChangeState(const void *pSender, const ZDeviceDataUpdatedEvent &event) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;
    std::string msg_pub = "";
    std::string topic_request = "";
    Object::Ptr responseData = new Object;
    Object::Ptr item = new Object;
    Object::Ptr objectData = new Object;
    try {
        std::string deviceId = std::to_string(event.device()->info()->deviceId());
        std::string userApiKey = event.userApiKey();
        int64_t requestId = time(NULL);
        std::string state = event.varialbleValue();
        item->set("requestId", requestId);
        item->set("state", state);
        std::stringstream buffer;
        Poco::JSON::Stringifier::stringify(item, buffer);
        msg_pub = buffer.str();
        topic_request = MqttTopic::requestTopic(userApiKey, deviceId, event.apiName());
        responseData->set("msg", msg_pub);
        responseData->set("topic", topic_request);

        objectData->set("deviceId", deviceId);
//        objectData->set("variableId", event.varialbleId());
        objectData->set("value", event.varialbleValue());
        objectData->set("userApiKey", userApiKey);
        d_ptr->list_object_request[requestId] = objectData;
//        std::string json_obj = "{deviceId:" + deviceId + ",requestId:" + std::to_string(requestId) + ", variableId:" + std::to_string(event.varialbleId()) + ",value:" + event.varialbleValue() + ",userApiKey:" + userApiKey + "}";
        std::string json_obj = "";
        std::cout << "json_obj: " << json_obj << endl;
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
    }
    return responseData;
}

//Device -> Server

void ZApiMQTT::handleReceiveMessage(const void *pSender, const ZCloudReceiveMessageEvent &event) {
    std::string msgReceive = event.msg();
    std::string topicReceive = event.topic();
    Poco::JSON::Object::Ptr bodyData = new Poco::JSON::Object;
    Poco::Dynamic::Var responseData;
    Poco::JSON::Parser parser;
    try {
        if (!Poco::trim(msgReceive).empty()) {
            std::vector<std::string> params = ZApiMQTT::split(topicReceive, '/');
            std::string topicType = params[6];
            Poco::Dynamic::Var result = parser.parse(msgReceive);
            bodyData = result.extract<Poco::JSON::Object::Ptr>();
            if (topicType == "request") {
                responseData = handleReceiveMessage_ChannelRequest(pSender, event, bodyData, params);
            }

            if (topicType == "response") {
                handleReceiveMessage_ChannelResponse(pSender, event, bodyData, params);
            }
        }
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
    }   
}

Poco::Dynamic::Var ZApiMQTT::handleReceiveMessage_ChannelRequest(
        const void *pSender,
        const ZCloudReceiveMessageEvent &event,
        Poco::JSON::Object::Ptr& bodyData,
        std::vector<std::string> params) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;
    int32_t error_response = 0;
    Object::Ptr responseData = new Object;
    int64_t requestId;
    int32_t variableId;
    std::string value;
    std::string userApiKey = params[2];
    int32_t deviceId = 0;
    Poco::Int32 i32Value;

    try {
        std::string apiName = params[5];
        /*
         * ----------CHANGE DATA (device->server)----------
         * json: {"requestId": 1, variableId": 1,"value": 30}
         * update DB
         * broadcast to ws
         * publish topic response
         */

        if (apiName == "updateData") {
            requestId = bodyData->optValue("requestId", -1);
            if (Poco::NumberParser::tryParse(params[4], i32Value)) {
                deviceId = i32Value;
            }
            variableId = bodyData->optValue("variableId", -1);
            value = bodyData->optValue("value", std::string());
            ZWorker *worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
            ZWorkerResult workerResultUpdateData = worker->updateDataDevice(userApiKey, deviceId, variableId, value);
            if (workerResultUpdateData.failed()) {
                error_response = static_cast<int32_t> (workerResultUpdateData.errorCode());
            }

            std::string topic = MqttTopic::responseTopic(userApiKey, std::to_string(deviceId), apiName);
            std::string msg_pub = "{\"requestId\":" + std::to_string(requestId) + ", \"error\":" + std::to_string(error_response) + "}";
            responseData->set("msg", msg_pub);
            responseData->set("topic", topic);
        }
        if (apiName == "updateState") {
            requestId = bodyData->optValue("requestId", -1);
            value = bodyData->optValue("state", std::string());
            //do something
        }
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
    }
    return responseData;
}

void ZApiMQTT::handleReceiveMessage_ChannelResponse(
        const void *pSender,
        const ZCloudReceiveMessageEvent &event,
        Poco::JSON::Object::Ptr& bodyData,
        std::vector<std::string> params) {
    using namespace Poco::JSON;
    using namespace Poco::Dynamic;
    Application& app = Application::instance();
    Object::Ptr item = new Object;
    int32_t error_response = 0;
    try {
        std::string apiName = params[5];
        int32_t error = bodyData->optValue("error", -1);
        int64_t requestId = bodyData->optValue("requestId", -1);
        ZApiMQTT::Impl::ObjectMap::iterator iLookup = d_ptr->list_object_request.find(requestId);
        if (iLookup != d_ptr->list_object_request.end() && error == 0) {
            item = d_ptr->list_object_request[requestId];
            if (apiName == "changeValue") {
                /*
                 * update BD
                 * broadcast ws
                 */
                std::string userApiKey = item->optValue("userApiKey", std::string());

                std::string value = item->optValue("value", std::string());
                int32_t deviceId = item->optValue("deviceId", -1);
                int32_t variableId = item->optValue("variableId", -1);
                std::cout << "{value: " << value << ", requestId: " << requestId << "}" << endl;
                if (userApiKey.empty() || value.empty() || deviceId <= 0 || variableId <= 0) {
                    error_response = static_cast<int32_t> (ZErrorCode::APIMQTT_ParameterMissing);
                    app.logger().error("Params Missing, Error: ", error_response);
                } else {
                    ZWorker *worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);
                    ZWorkerResult workerResultUpdateData = worker->updateDataDevice(userApiKey, deviceId, variableId, value);
                    if (workerResultUpdateData.failed()) {
                        error_response = static_cast<int32_t> (workerResultUpdateData.errorCode());
                        app.logger().error("Update data error, Error: ", error_response);
                    }
                }
            }
            if (apiName == "changeState") {

            }
        }

    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
    }
}

std::vector<std::string> ZApiMQTT::split(const std::string& str, char delimiter) {
    vector<string> internal;
    stringstream ss(str); // Turn the string into a stream.
    string tok;

    while (getline(ss, tok, delimiter)) {
        internal.push_back(tok);
    }

    return internal;
}

