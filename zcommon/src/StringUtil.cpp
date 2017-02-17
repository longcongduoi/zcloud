/*
 * File:   StringUtil.cpp
 * Author: huuhoa
 *
 * Created on October 30, 2015, 3:19 PM
 */

#include <map>
#include <sstream>
#include <iterator>
#include <algorithm>

#include <Poco/NumberParser.h>

#include "zcommon/StringUtil.h"

using namespace Poco::JSON;
using namespace Poco::Dynamic;
std::string StringUtil::join(const std::vector<std::string> &list, const std::string& separator) {
    if (list.empty()) {
        return "";
    }

    std::stringstream ss;

    auto iter = list.begin();

    ss << *iter;
    iter ++;
    if (iter == list.end()) {
        return ss.str();
    }

    for (; iter != list.end(); iter ++) {
        ss << separator;
        ss << *iter;
    }

    return ss.str();
}

std::string StringUtil::join(const std::map<std::string,std::string>& map, const std::string& separator) {
    if (map.empty()) {
        return "";
    }

    std::stringstream ss;

    auto iter = map.begin();

    ss << iter->first << " " << iter->second;
    iter ++;
    if (iter == map.end()) {
        return ss.str();
    }

    for (; iter != map.end(); iter ++) {
        ss << separator;
        ss << iter->first << " " << iter->second;
    }

    return ss.str();
}

int32_t stringToInteger(const std::string& s)
{
    return Poco::NumberParser::parse(s);
}

std::vector<int32_t> StringUtil::convert(const std::vector<std::string> &list) {
    std::vector<int32_t> result;
    std::transform(list.begin(), list.end(), std::back_inserter(result), stringToInteger);
    return result;
}

Poco::JSON::Object::Ptr StringUtil::convertString2Json(const std::string& msg) {
    Poco::JSON::Object::Ptr emptyResult;
    if (msg.empty()) {
        return emptyResult;
    }

    Parser parser;
    try {
        Var result = parser.parse(msg);
        if (result.type() != typeid (Object::Ptr)) {
            return emptyResult;
        }

        Object::Ptr object = result.extract<Object::Ptr>();
        if (object->size() == 0) {
            return emptyResult;
        }
        return object;
    } catch (Poco::Exception &ex) {
        return emptyResult;
    }
    return emptyResult;
}
