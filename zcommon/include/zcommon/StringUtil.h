/*
 * File:   StringUtil.h
 * Author: huuhoa
 *
 * Created on October 30, 2015, 3:19 PM
 */

#ifndef STRINGUTIL_H
#define	STRINGUTIL_H

#include <vector>
#include <map>
#include <string>
#include <Poco/Util/Application.h>
#include <Poco/Dynamic/Struct.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/Dynamic/Var.h>

class StringUtil {
public:
    static std::string join(const std::vector<std::string> &list, const std::string& separator);
    static std::string join(const std::map<std::string, std::string>& map, const std::string& separator);
    static std::vector<int32_t> convert(const std::vector<std::string> &list);
    static Poco::JSON::Object::Ptr convertString2Json(const std::string& msg);
private:

};

#endif	/* STRINGUTIL_H */

