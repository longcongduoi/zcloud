/* 
 * File:   ZVarInfo.h
 * Author: tamvh
 *
 * Created on December 14, 2015, 10:15 AM
 */

#ifndef ZVARINFO_H
#define	ZVARINFO_H
#include <map>
#include <list>
#include <string>
#include <vector>
#include <memory>
class ZVarInfo {
public:
    typedef std::shared_ptr<ZVarInfo> Ptr;
    typedef std::vector<Ptr> List;
    typedef std::map<int32_t, Ptr> Map;
    typedef std::vector<int32_t> KeyList;
public:
    ZVarInfo();
    ~ZVarInfo();
public:
    bool operator==(const ZVarInfo& other);
    bool operator!=(const ZVarInfo& other);    
    
    void setDeviceId(const int32_t value);
    int32_t deviceId() const;
    void setVarId(const int32_t value);
    int32_t varId() const;    
    void setVarName(const std::string& value);
    std::string varName() const;
    void setVarValue(const std::string& value);
    std::string varValue() const;
private:
    int32_t varId_;
    int32_t deviceId_;   
    std::string varName_;
    std::string varValue_;
};

#endif	/* ZVARINFO_H */

