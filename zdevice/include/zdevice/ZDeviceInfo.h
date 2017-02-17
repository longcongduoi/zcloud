/*
 * File:   ZDeviceInfo.h
 * Author: huuhoa
 *
 * Created on October 27, 2015, 5:21 PM
 */

#ifndef ZDEVICEINFO_H
#define	ZDEVICEINFO_H

#include <map>
#include <list>
#include <string>
#include <vector>
#include <memory>

class ZDeviceInfo {
public:
    typedef std::shared_ptr<ZDeviceInfo> Ptr;
    typedef std::vector<Ptr> List;
    typedef std::map<int32_t, Ptr> Map;
    typedef std::map<std::string, Ptr> MapStr;
    typedef std::vector<int32_t> KeyList;
    typedef std::vector<std::string> KeyListString;
public:
    ZDeviceInfo();
    ~ZDeviceInfo();
public:
    bool operator ==(const ZDeviceInfo& other);
    bool operator !=(const ZDeviceInfo& other);

    void setApiKey(const std::string& value);
    std::string apiKey() const;

    void setUpdatedAt(const uint64_t value);
    uint64_t updatedAt() const;

    void setCreatedAt(const uint64_t value);
    uint64_t createdAt() const;

    void setDeviceGroup(const std::string& value);
    std::string deviceGroup() const;

    void setDeviceType(const int32_t value);
    int32_t deviceType() const;

    void setDeviceName(const std::string& value);
    std::string deviceName() const;
    
    void setDeviceAddress(const std::string& value);
    std::string deviceAddress() const;

    void setDeviceId(const int32_t value);
    int32_t deviceId() const;
    
    void setSecurityNumber(const std::string& value);
    std::string securityNumber();

private:
    int32_t deviceId_;
    std::string deviceName_;
    std::string deviceAddress_;
    int32_t deviceType_;
    std::string deviceGroup_;
    uint64_t createdAt_;
    uint64_t updatedAt_;
    std::string apiKey_;
    std::string securityNumber_;
};

#endif	/* ZDEVICEINFO_H */

