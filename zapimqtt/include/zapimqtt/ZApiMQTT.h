/*
 * File:   ZApiMQTT.h
 * Author: huuhoa
 *
 * Created on October 24, 2015, 3:17 PM
 */

#ifndef ZAPIMQTT_H
#define ZAPIMQTT_H

#include <vector>
#include <iostream>
#include <memory>
#include <Poco/JSON/Object.h>

class ZDeviceDataUpdatedEvent;
class ZCloudReceiveMessageEvent;
class ZApiMQTT : public ZServiceInterface {
public:
    ZApiMQTT();
    virtual ~ZApiMQTT();

public:
    virtual bool initialize();
    virtual bool start();
    virtual bool stop();
    virtual bool cleanup();
public:
    void publishData(const std::string& topic, const std::string& msg);
    void handlePublishMessage(const void *pSender, const ZDeviceDataUpdatedEvent &event);
    void handleReceiveMessage(const void *pSender, const ZCloudReceiveMessageEvent &event);
    Poco::Dynamic::Var handleChangeValue(const void *pSender, const ZDeviceDataUpdatedEvent &event);
    Poco::Dynamic::Var handleChangeState(const void *pSender, const ZDeviceDataUpdatedEvent &event);
    Poco::Dynamic::Var handleReceiveMessage_ChannelRequest(const void *pSender, const ZCloudReceiveMessageEvent &event, Poco::JSON::Object::Ptr& bodyData, std::vector<std::string> params);
    void handleReceiveMessage_ChannelResponse(const void *pSender, const ZCloudReceiveMessageEvent &event, Poco::JSON::Object::Ptr& bodyData, std::vector<std::string> params);
    std::vector<std::string> split(const std::string& str, char delimiter);
private:
    class Impl;
    std::shared_ptr<Impl> d_ptr;
};

#endif /* ZAPIMQTT_H */

