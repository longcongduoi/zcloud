/* 
 * File:   ZVarLastestValue.h
 * Author: tamvh
 *
 * Created on December 14, 2015, 11:27 AM
 */

#ifndef ZVARLASTESTVALUE_H
#define	ZVARLASTESTVALUE_H
#include <map>
#include <list>
#include <string>
#include <vector>
#include <memory>

class ZVarLastestValue {
public:
    typedef std::shared_ptr<ZVarLastestValue> Ptr;
    typedef std::vector<Ptr> List;
    typedef std::map<int32_t, Ptr> Map;
    typedef std::vector<int32_t> KeyList;
public:
    ZVarLastestValue();
    ~ZVarLastestValue();
public:
    bool operator==(const ZVarLastestValue& other);
    bool operator!=(const ZVarLastestValue& other);

    void setVarId(const int32_t value);
    int32_t varId() const;
    void setValue(const std::string& value);
    std::string value() const;
private:
    int32_t varId_;
    std::string value_;

};

#endif	/* ZVARLASTESTVALUE_H */

