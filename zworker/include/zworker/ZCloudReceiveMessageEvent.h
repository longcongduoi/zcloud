/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ZCloudReceiveMessageEvent.h
 * Author: tamvh
 *
 * Created on February 9, 2017, 3:57 PM
 */

#ifndef ZCLOUDRECEIVEMESSAGEEVENT_H
#define ZCLOUDRECEIVEMESSAGEEVENT_H

#include <memory>
class ZCloudReceiveMessageEvent {
public:
    ZCloudReceiveMessageEvent(
            const std::string& msg, 
            const std::string& topic)
        : 
        msg_(msg),
        topic_(topic) {
    }
public:
    std::string msg() const { return msg_; }
    std::string topic() const { return topic_; }   
private:   
    std::string msg_;
    std::string topic_;
};

#endif /* ZCLOUDRECEIVEMESSAGEEVENT_H */

