/*
 * File:   ZVarValue.h
 * Author: tamvh
 *
 * Created on December 14, 2015, 11:06 AM
 */

#ifndef ZVARVALUE_H
#define	ZVARVALUE_H
#include <map>
#include <list>
#include <string>
#include <vector>
#include <memory>
class ZVarValue {
public:
    typedef std::shared_ptr<ZVarValue> Ptr;
    typedef std::vector<Ptr> List;
    typedef std::map<int32_t, Ptr> Map;
    typedef std::map<std::string, Ptr> MapString;
    typedef std::map<std::string, std::string> MapStr;
    typedef std::vector<int32_t> KeyList;
    typedef std::vector<std::string> KeyListString;
    typedef std::vector<std::string> ListString;
public:
    ZVarValue();
    ~ZVarValue();
public:
    bool operator==(const ZVarValue& other);
    bool operator!=(const ZVarValue& other);

    void setVarId(const int32_t value);
    int32_t varId() const;
    void setVarKey(const std::string& value);
    std::string varKey() const;
    void setTimeStamp(const uint64_t value);
    uint64_t timeStamp() const;
    void setValue(const std::string& value);
    std::string value() const;
    std::string toString();
private:
    int32_t varId_;
    uint64_t timeStamp_;
    std::string varkey_;
    std::string value_;

};

#endif	/* ZVARVALUE_H */

