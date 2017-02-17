/* 
 * File:   ZDBProxy.cpp
 * Author: huuhoa
 * 
 * Created on October 28, 2015, 5:04 PM
 */

#include "zdb/ZDBProxy.h"
#include <string>
#include <hiredis/hiredis.h>
#include <Poco/Util/Application.h>
#include <zcommon/StringUtil.h>
#include <sstream>
#include <iostream>
#include <redis3m/redis3m.hpp>

using Poco::Util::Application;

namespace {

ZDBProxy::StringList toStringList(const redis3m::reply& r) {
    ZDBProxy::StringList retVal;
    const std::vector<redis3m::reply>& ra = r.elements();
    for (auto iter = ra.begin(); iter != ra.end(); iter++) {
        retVal.push_back(iter->str());
    }
    return retVal;
}

ZDBProxy::StringMap toStringMap(const redis3m::reply& r) {
    ZDBProxy::StringMap retVal;
    std::string key;
    bool isKey = true;
    const std::vector<redis3m::reply>& ra = r.elements();
    for (auto iter = ra.begin(); iter != ra.end(); iter++) {
        if (isKey) {
            key = iter->str();
            isKey = false;
        } else {
            retVal[key] = iter->str();
            isKey = true;
        }
    }
    return retVal;
}

}
//
//ZDBProxy::reply::reply(void *c):
//    _type(type_t::ERROR),
//    _integer(0) {
//    redisReply* c_reply = static_cast<redisReply*>(c);
//    _type = static_cast<type_t>(c_reply->type);
//    switch (_type) {
//	case type_t::ERROR:
//	case type_t::STRING:
//	case type_t::STATUS:
//            _str = std::string(c_reply->str, c_reply->len);
//            break;
//	case type_t::INTEGER:
//            _integer = c_reply->integer;
//            break;
//	case type_t::ARRAY:
//            for (size_t i=0; i < c_reply->elements; ++i) {
//                _elements.push_back(reply(c_reply->element[i]));
//            }
//            break;
//        default:
//            break;
//    }
//}
//
//ZDBProxy::reply::reply():
//    _type(type_t::ERROR),
//    _integer(0) {
//}
//
//std::string ZDBProxy::reply::toString() const {
//    std::stringstream ss;
//    switch (_type) {
//    case type_t::ARRAY:
//        ss << "Type [ARRAY]";
//        ss << ", [";
//        {
//            for (auto iter = elements().begin(); iter != elements().end(); iter++) {
//                ss << iter->toString();
//                ss << ", ";
//            }
//        }
//        ss << "]";
//        break;
//    case type_t::ERROR:
//        ss << "Type [ERROR]";
//        ss << ", ";
//        ss << str();
//        break;
//    case type_t::INTEGER:
//        ss << "Type [INTEGER]";
//        ss << ", ";
//        ss << integer();
//        break;
//    case type_t::NIL:
//        ss << "Type [NIL]";
//        break;
//    case type_t::STATUS:
//        ss << "Type [STATUS]";
//        ss << ", ";
//        ss << str();
//        break;
//    case type_t::STRING:
//        ss << "Type [STRING]";
//        ss << ", ";
//        ss << str();
//        break;
//    default:
//        ss << "UNKNOWN";
//        break;
//    }
//    return ss.str();
//}

ZDBProxy::ZDBProxy()
{
}

ZDBProxy::~ZDBProxy() {
}

/// connect to server
bool ZDBProxy::connect(const std::string& host, int port) {
    _connectionPool = redis3m::simple_pool::create(host, port);
    try {
        redis3m::connection::ptr_t connection = _connectionPool->get();
        _connectionPool->put(connection);
    } catch (redis3m::connection_error& e) {
        _connectionPool.reset();
        return false;
    }
    return true;
}

/// connect to server with timeout
bool ZDBProxy::connect(const std::string& host, int port, const struct timeval timeout) {
    _connectionPool = redis3m::simple_pool::create(host, port);
    return true;
}

bool ZDBProxy::isConnected() const {
    return (bool)_connectionPool;
}

/// Append a value to a key
/// http://redis.io/commands/append
int64_t ZDBProxy::APPEND(const std::string& key, const std::string& value) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"APPEND", key, value});
    _connectionPool->put(connection);
    return r.integer();
}

/// Count the number of set bits (population counting) in a string
/// http://redis.io/commands/bitcount
int64_t ZDBProxy::BITCOUNT(const std::string& key, int start, int end) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "BITCOUNT";
    c << key;
    c << start;
    c << end;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return r.integer();
}

int64_t ZDBProxy::BITCOUNT(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"BITCOUNT", key});
    _connectionPool->put(connection);
    return r.integer();
}

int64_t ZDBProxy::BITOP(const std::string& op, const std::string& destkey, const StringList& srckeys) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "BITOP";
    c << op;
    c << destkey;
    c << srckeys;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return r.integer();
}

/// Perform a bitwise operation between multiple keys (containing string values)
/// and store the result in the destination key
/// http://redis.io/commands/bitop
int64_t ZDBProxy::BITOP_AND(const std::string& destkey, const StringList& srckeys) {
    return BITOP("AND", destkey, srckeys);
}

int64_t ZDBProxy::BITOP_OR(const std::string& destkey, const StringList& srckeys) {
    return BITOP("OR", destkey, srckeys);
}

int64_t ZDBProxy::BITOP_XOR(const std::string& destkey, const StringList& srckeys) {
    return BITOP("XOR", destkey, srckeys);
}

int64_t ZDBProxy::BITOP_NOT(const std::string& destkey, const std::string& srckey) {
    return BITOP("NOT", destkey, { srckey });
}

/// Decrements the integer value of a key by one
/// http://redis.io/commands/decr
int64_t ZDBProxy::DECR(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"DECR", key});
    _connectionPool->put(connection);
    return r.integer();
}

/// Decrements the integer value of a key by the given number
/// http://redis.io/commands/decrby
int64_t ZDBProxy::DECRBY(const std::string& key, int64_t decrement) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "DECRBY";
    c << key;
    c << decrement;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return r.integer();
}

/// Get the value of a key
/// http://redis.io/commands/get
std::string ZDBProxy::GET(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"GET", key});
    _connectionPool->put(connection);
    return r.str();
}

/// Increments the number stored at key by one
/// http://redis.io/commands/incr
int64_t ZDBProxy::INCR(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "INCR";
    c << key;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return r.integer();
}

/// Increments the integer value of a key by the given number
/// http://redis.io/commands/incrby
int64_t ZDBProxy::INCRBY(const std::string& key, int64_t increment) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "INCRBY";
    c << key;
    c << increment;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return r.integer();
}

/// Get the values of all the given keys
/// http://redis.io/commands/mget
ZDBProxy::StringList ZDBProxy::MGET(const ZDBProxy::StringList& keys) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "MGET";
    c << keys;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return toStringList(r);
}

/// Set multiple keys to multiple values
/// http://redis.io/commands/mset
void ZDBProxy::MSET(const ZDBProxy::StringMap& keyValues) {
    if (keyValues.empty()) {
        return;
    }
    
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "MSET";
    c << keyValues;
    connection->run(c);
    _connectionPool->put(connection);
}

/// Set the string value of a key
/// http://redis.io/commands/set
bool ZDBProxy::SET(const std::string& key, const std::string& value) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"SET", key, value});
    _connectionPool->put(connection);
    return r.type() == redis3m::reply::type_t::STATUS;
}

/// Set the string value of a key
/// http://redis.io/commands/lpush
int64_t ZDBProxy::LPUSH(const std::string& key, const std::string& value) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"LPUSH", key, value});
    _connectionPool->put(connection);
    return r.integer();
    
}

/// get list value from-to
/// http://redis.io/commands/lrange
ZDBProxy::StringList ZDBProxy::LRANGE(const std::string& key, int32_t start, int32_t stop) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "LRANGE";
    c << key;
    c << start;
    c << stop;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return toStringList(r);
}

bool ZDBProxy::LTRIM(const std::string& key, int32_t start, int32_t stop) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "LTRIM";
    c << key;
    c << start;
    c << stop;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return r.type() == redis3m::reply::type_t::STATUS;
}


/// Sets or clears the bit at offset in the string value stored at key
/// http://redis.io/commands/setbit
int64_t ZDBProxy::SETBIT(const std::string& key, int offset, int value) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "SETBIT";
    c << key;
    c << offset;
    c << value;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return r.integer();
}

/// Get the length of the value stored in a key
/// http://redis.io/commands/strlen
int64_t ZDBProxy::STRLEN(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"STRLEN", key});
    _connectionPool->put(connection);
    return r.integer();
}

/// KEYS COMMANDS

/// Removes the specified keys
/// http://redis.io/commands/del
int64_t ZDBProxy::DEL(const std::string& key) {
    StringList list;
    list.push_back(key);
    return DEL(list);
}

int64_t ZDBProxy::DEL(const ZDBProxy::StringList& keys) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "DEL";
    c << keys;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return r.integer();
}

/// Determine if a key exists
/// http://redis.io/commands/exists
bool ZDBProxy::EXISTS(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"EXISTS", key});
    _connectionPool->put(connection);
    return r.integer() == 1;
}

/// HASH COMMANDS

/// Removes the specified fields from the hash stored at key
/// http://redis.io/commands/hdel
int64_t ZDBProxy::HDEL(const std::string& key, const std::string& field) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "HDEL";
    c << key;
    c << field;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return r.integer();
}

int64_t ZDBProxy::HDEL(const std::string& key, const ZDBProxy::StringList& fields) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "HDEL";
    c << key;
    c << fields;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return r.integer();
}

/// Determine if a hash field exists
/// http://redis.io/commands/hexists
bool ZDBProxy::HEXISTS(const std::string& key, const std::string& field) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"HEXISTS", key, field});
    _connectionPool->put(connection);
    return r.integer() == 1;
}

/// Get the value of a hash field
/// http://redis.io/commands/hget
std::string ZDBProxy::HGET(const std::string& key, const std::string& field) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"HGET", key, field});
    _connectionPool->put(connection);
    return r.str();
}

/// Get all the fields and values in a hash
/// http://redis.io/commands/hgetall
ZDBProxy::StringMap ZDBProxy::HGETALL(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"HGETALL", key});
    _connectionPool->put(connection);
    return toStringMap(r);
}

/// Get all the fields in a hash
/// http://redis.io/commands/hkeys
ZDBProxy::StringList ZDBProxy::HKEYS(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"HKEYS", key});
    _connectionPool->put(connection);
    return toStringList(r);
}

/// Get the number of fields in a hash
/// http://redis.io/commands/hlen
int64_t ZDBProxy::HLEN(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"HLEN", key});
    _connectionPool->put(connection);
    return r.integer();
}

/// Get the values of all the given hash fields
/// http://redis.io/commands/hmget
ZDBProxy::StringList ZDBProxy::HMGET(const std::string& key, const ZDBProxy::StringList& fields) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c("HMGET");
    c << key;
    c << fields;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return toStringList(r);
}

/// Set multiple hash fields to multiple values
/// http://redis.io/commands/hmset
void ZDBProxy::HMSET(const std::string& key, const ZDBProxy::StringMap& fieldValues) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c("HMSET");
    c << key;
    c << fieldValues;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
}

/// Set the string value of a hash field
/// http://redis.io/commands/hset
bool ZDBProxy::HSET(const std::string& key, const std::string& field, const std::string& value) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"HSET", key, field, value});
    _connectionPool->put(connection);

    return r.type() == redis3m::reply::type_t::INTEGER;
}

/// Get all the values in a hash
/// http://redis.io/commands/hvals
ZDBProxy::StringList ZDBProxy::HVALS(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"HVALS", key});
    _connectionPool->put(connection);
    return toStringList(r);
}


/// CONNECTION COMMANDS

/// Authenticate to the server
/// http://redis.io/commands/auth
bool ZDBProxy::AUTH(const std::string& password) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"AUTH", password});
    _connectionPool->put(connection);
    return r == "OK";
}

/// Echo the given string
/// http://redis.io/commands/echo
std::string ZDBProxy::ECHO(const std::string& message) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"ECHO", message});
    _connectionPool->put(connection);
    return r.str();
}

/// Ping the server
/// http://redis.io/commands/ping
std::string ZDBProxy::PING(const std::string& message) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"PING", message});
    _connectionPool->put(connection);
    return r.str();
}

/// Close the connection
/// http://redis.io/commands/quit
void ZDBProxy::QUIT() {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    connection->run({"QUIT"});
    _connectionPool->put(connection);
}


/// SETS COMMANDS

/// Add one or more members to a set
/// http://redis.io/commands/sadd
int64_t ZDBProxy::SADD(const std::string& key, const StringList& members) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c("SADD");
    c << key;
    c << members;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    
    return r.integer();
}

/// Get the number of members in a set
/// http://redis.io/commands/scard
int64_t ZDBProxy::SCARD(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"SCARD", key});
    _connectionPool->put(connection);
    return r.integer();
}

/// Determine if a given value is a member of a set
/// http://redis.io/commands/sismember
bool ZDBProxy::SISMEMBER(const std::string& key, const std::string& member) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"SISMEMBER", key, member});
    _connectionPool->put(connection);
    return r.integer() == 1;
}

/// Remove one or more members from a set
/// http://redis.io/commands/srem
int64_t ZDBProxy::SREM(const std::string& key, const StringList& members) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "SREM";
    c << key;
    c << members;
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
    return r.integer();
}

/// Incrementally iterate Set elements
/// http://redis.io/commands/sscan
void ZDBProxy::SSCAN(const std::string& key, int64_t cursor, int count) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::command c;
    c << "SSCAN";
    c << key;
    if (count > 0) {
        c << "COUNT";
        c << count;
    }
    redis3m::reply r = connection->run(c);
    _connectionPool->put(connection);
}

ZDBProxy::StringList ZDBProxy::SMEMBERS(const std::string& key) {
    redis3m::connection::ptr_t connection = _connectionPool->get();
    redis3m::reply r = connection->run({"SMEMBERS", key});
    _connectionPool->put(connection);
    return toStringList(r);
}
