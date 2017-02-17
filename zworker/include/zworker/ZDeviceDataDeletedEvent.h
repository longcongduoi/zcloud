/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   ZDeviceDataDeleteEvent.h
 * Author: tamvh
 *
 * Created on February 19, 2016, 9:48 AM
 */

#ifndef ZDEVICEDATADELETEEVENT_H
#define ZDEVICEDATADELETEEVENT_H

#include <memory>
class ZDevice;

class ZDeviceDataDeletedEvent {
public:

    ZDeviceDataDeletedEvent(const std::string& deviceId, const std::string& apiKey)
    : deviceId_(deviceId),
      apiKey_(apiKey) {
    }

public:

    std::string deviceId() const {
        return deviceId_;
    }

    std::string apiKey() const {
        return apiKey_;
    }
private:
    std::string deviceId_;
    std::string apiKey_;
};

#endif /* ZDEVICEDATADELETEEVENT_H */

