/* 
 * File:   ZUserInfo.h
 * Author: huuhoa
 *
 * Created on October 23, 2015, 2:06 PM
 */

#ifndef ZUSERINFO_H
#define ZUSERINFO_H

#include <map>
#include <list>
#include <string>
#include <vector>
#include <memory>

/// Hold basic user information

class ZUserInfo {
public:
    typedef std::shared_ptr<ZUserInfo> Ptr;
    typedef std::vector<Ptr> List;
    typedef std::map<int32_t, Ptr> Map;
    typedef std::vector<int32_t> KeyList;
public:
    ZUserInfo();
public:
    bool operator==(const ZUserInfo& other);
    bool operator!=(const ZUserInfo& other);
    
    void setApiKey(const std::string& value);
    std::string apiKey() const;
            
    void setUserId(const int32_t value);
    int32_t userId() const;
    
    void setUserName(const std::string& value);
    std::string userName() const;
    
    void setPassword(const std::string& value);
    std::string password() const;
    
    void setDisplayName(const std::string& value);
    std::string displayName() const;
    
    void setAvatar(const std::string& value);
    std::string avatar() const;
            
    void setCreatedAt(const int64_t value);
    int64_t createdAt() const;
    
    void setUpdatedAt(const int64_t value);
    int64_t updatedAt() const;
private:
    int32_t userId_;
    std::string userName_;
    std::string password_;
    std::string displayName_;
    std::string avatar_;
    int64_t createdAt_;
    int64_t updatedAt_;

    // not saved in the basic info, saved in key users:uid:apikey
    std::string apiKey_; 
};

#endif /* ZUSERINFO_H */

