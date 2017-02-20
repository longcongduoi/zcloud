/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ZAPIHttpClient.cpp
 * Author: tamvh
 * 
 * Created on February 20, 2017, 1:23 PM
 */
#include "zapihttp.client/ZAPIHttpClient.h"
#include "zapihttp.client/ClientWorker.h"
#include <zworker/ZWorker.h>
#include <zworker/ZCloudReceiveMessageEvent.h>
using namespace Poco;
using namespace Net;
using namespace std;
using Poco::Util::Application;
using Poco::Util::LayeredConfiguration;
class ZAPIHttpClient::Impl {
public:
    Poco::SharedPtr<ClientWorker> httpClient;
};

bool ZAPIHttpClient::initialize() {
    LayeredConfiguration& config = Application::instance().config();
    std::string host = config.getString("api.http.client.host", " http://iotlc.stats.vng.com.vn");
    std::string api = config.getString("api.http.client.api", "api/datapoints");
    d_ptr->httpClient = new ClientWorker(host, api);
    ZWorker *worker = ZServiceLocator::instance()->get<ZWorker>(ZServiceLocator::ServiceId::Worker);   
    worker->receiveMessageEvent += Poco::delegate(this, &ZAPIHttpClient::handleRecvMessage);
    return true;
}

bool ZAPIHttpClient::start() {
    return true;
}

bool ZAPIHttpClient::stop() {
    return true;
}

bool ZAPIHttpClient::cleanup() {
    return true;
}

ZAPIHttpClient::ZAPIHttpClient() : d_ptr(new Impl) {
}

ZAPIHttpClient::~ZAPIHttpClient() {
}

void ZAPIHttpClient::handleRecvMessage(const void *pSender, const ZCloudReceiveMessageEvent &event) {
    std::string msg = event.msg();
    
    Poco::JSON::Object::Ptr obj_body = new Poco::JSON::Object;
    Poco::JSON::Object::Ptr obj_content = new Poco::JSON::Object;
    Poco::JSON::Object::Ptr obj_data = new Poco::JSON::Object;
    Poco::JSON::Parser parser_msg;
    Poco::JSON::Parser parser_content;
    Poco::JSON::Parser parser_data;
    Poco::Dynamic::Var result;
    try {
        if (!Poco::trim(msg).empty()) {
            result = parser_msg.parse(msg);
            obj_body = result.extract<Poco::JSON::Object::Ptr>();
            std::string content = obj_body->optValue("content", std::string());            
            if (!Poco::trim(content).empty()) {                                              
                result = parser_content.parse(content);
                obj_content = result.extract<Poco::JSON::Object::Ptr>();
                std::string data = obj_content->optValue("data", std::string());
                if (!Poco::trim(data).empty()) {
                    result = parser_data.parse(data);
                    obj_data = result.extract<Poco::JSON::Object::Ptr>();
                    std::string deviceId = obj_data->optValue("deviceid", std::string());
                    std::string value = obj_data->optValue("value", std::string());
                    std::string localTime = obj_data->optValue("localTime", std::string());
                    d_ptr->httpClient->setParam("name", deviceId);
                    d_ptr->httpClient->setParam("value", value);
                    d_ptr->httpClient->setParam("timestamp", localTime);
                    d_ptr->httpClient->setMethod("POST");
                    d_ptr->httpClient->sendRequest();
                    std::cout << "response data: " << d_ptr->httpClient->responseData() << std::endl;
                }
            }
        }
    } catch (Poco::Exception &ex) {
        Application& app = Application::instance();
        app.logger().error("Exception while processing message: %s", ex.displayText());
    }   
}


