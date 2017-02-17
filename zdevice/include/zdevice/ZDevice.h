/* 
 * File:   ZDevice.h
 * Author: huuhoa
 *
 * Created on November 30, 2015, 2:22 PM
 */

#ifndef ZDEVICE_H
#define	ZDEVICE_H

class ZDeviceInfo;

#include <memory>

class ZDevice {
public:
    ZDevice();
    ZDevice(const ZDevice& orig);
    ~ZDevice();

public:

    std::shared_ptr<ZDeviceInfo> info() const {
        return info_;
    }

    void setInfo(std::shared_ptr<ZDeviceInfo> info) {
        info_ = info;
    }
private:
    std::shared_ptr<ZDeviceInfo> info_;
};

#endif	/* ZDEVICE_H */

