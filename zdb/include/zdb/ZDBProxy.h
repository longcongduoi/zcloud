/* 
 * File:   ZDBProxy.h
 * Author: huuhoa
 *
 * Created on October 28, 2015, 5:04 PM
 */

#ifndef ZDBPROXY_H
#define	ZDBPROXY_H

#include <vector>
#include <string>
#include <map>
#include <exception>
#include <memory>


#ifndef _NOEXCEPT
#   if __GNUC__ >= 4
#       define _NOEXCEPT _GLIBCXX_USE_NOEXCEPT
#   endif // if __GNUC__ >= 4
#endif // #ifndef _NOEXCEPT 

namespace redis3m {
class simple_pool;
}
/// DB Proxy đóng vai trò interface giữa các service trong ZCloud với GDB.
/// GDB là dịch vụ (cloud service) cung cấp DB (tương thích với Redis) cho 
/// các cloud service/app.
class ZDBProxy {
public:
    ZDBProxy();
    virtual ~ZDBProxy();

    /// connect to server
    bool connect(const std::string& host, int port);
    
    /// connect to server with timeout
    bool connect(const std::string& host, int port, const struct timeval timeout);
    
    bool isConnected() const;
public:
    typedef std::vector<std::string> StringList;
    typedef std::map<std::string, std::string> StringMap;
public:
    /// @section GDB interface 

    /// STRING COMMANDS
    
    /// Append a value to a key
    /// http://redis.io/commands/append
    int64_t APPEND(const std::string& key, const std::string& value);
    
    /// Count the number of set bits (population counting) in a string
    /// http://redis.io/commands/bitcount
    int64_t BITCOUNT(const std::string& key, int start, int end);
    int64_t BITCOUNT(const std::string& key);

    /// Perform a bitwise operation between multiple keys (containing string values)
    /// and store the result in the destination key
    /// http://redis.io/commands/bitop
    int64_t BITOP_AND(const std::string& destkey, const StringList& srckeys);
    int64_t BITOP_OR(const std::string& destkey, const StringList& srckeys);
    int64_t BITOP_XOR(const std::string& destkey, const StringList& srckeys);
    int64_t BITOP_NOT(const std::string& destkey, const std::string& srckey);
    
    /// Decrements the integer value of a key by one
    /// http://redis.io/commands/decr
    int64_t DECR(const std::string& key);
    
    /// Decrements the integer value of a key by the given number
    /// http://redis.io/commands/decrby
    int64_t DECRBY(const std::string& key, int64_t decrement);

    /// Get the value of a key
    /// http://redis.io/commands/get
    std::string GET(const std::string& key);
        
    /// Increments the number stored at key by one
    /// http://redis.io/commands/incr
    int64_t INCR(const std::string& key);
    
    /// Increments the integer value of a key by the given number
    /// http://redis.io/commands/incrby
    int64_t INCRBY(const std::string& key, int64_t increment);
    
    /// Get the values of all the given keys
    /// http://redis.io/commands/mget
    StringList MGET(const StringList& keys);
    
    /// Set multiple keys to multiple values
    /// http://redis.io/commands/mset
    void MSET(const StringMap& keyValues);
    
    /// Set the string value of a key
    /// http://redis.io/commands/set
    bool SET(const std::string& key, const std::string& value);
    
    /// Set the string value of a key
    /// http://redis.io/commands/lpush
    int64_t LPUSH(const std::string& key, const std::string& value);
    
    /// get list value from-to
    /// http://redis.io/commands/lrange
    ZDBProxy::StringList LRANGE(const std::string& key, int32_t start, int32_t stop);
    bool LTRIM(const std::string& key, int32_t start, int32_t stop);
    
    /// Sets or clears the bit at offset in the string value stored at key
    /// http://redis.io/commands/setbit
    int64_t SETBIT(const std::string& key, int offset, int value);
    
    /// Get the length of the value stored in a key
    /// http://redis.io/commands/strlen
    int64_t STRLEN(const std::string& key);
    
    /// KEYS COMMANDS
    
    /// Removes the specified keys
    /// http://redis.io/commands/del
    int64_t DEL(const std::string& key);
    int64_t DEL(const StringList& keys);
    
    /// Determine if a key exists
    /// http://redis.io/commands/exists
    bool EXISTS(const std::string& key);
    
    /// HASH COMMANDS
    
    /// Removes the specified fields from the hash stored at key
    /// http://redis.io/commands/hdel
    int64_t HDEL(const std::string& key, const std::string& field);
    int64_t HDEL(const std::string& key, const StringList& fields);
    
    /// Determine if a hash field exists
    /// http://redis.io/commands/hexists
    bool HEXISTS(const std::string& key, const std::string& field);
    
    /// Get the value of a hash field
    /// http://redis.io/commands/hget
    std::string HGET(const std::string& key, const std::string& field);
    
    /// Get all the fields and values in a hash
    /// http://redis.io/commands/hgetall
    StringMap HGETALL(const std::string& key);
    
    /// Get all the fields in a hash
    /// http://redis.io/commands/hkeys
    StringList HKEYS(const std::string& key);

    /// Get the number of fields in a hash
    /// http://redis.io/commands/hlen
    int64_t HLEN(const std::string& key);

    /// Get the values of all the given hash fields
    /// http://redis.io/commands/hmget
    StringList HMGET(const std::string& key, const StringList& fields);

    /// Set multiple hash fields to multiple values
    /// http://redis.io/commands/hmset
    void HMSET(const std::string& key, const StringMap& fieldValues);
    
    /// Set the string value of a hash field
    /// http://redis.io/commands/hset
    bool HSET(const std::string& key, const std::string& field, const std::string& value);
    
    /// Get all the values in a hash
    /// http://redis.io/commands/hvals
    StringList HVALS(const std::string& key);
    
    
    /// SETS COMMANDS
    
    /// Add one or more members to a set
    /// http://redis.io/commands/sadd
    int64_t SADD(const std::string& key, const StringList& members);
    
    /// Get the number of members in a set
    /// http://redis.io/commands/scard
    int64_t SCARD(const std::string& key);

    /// Determine if a given value is a member of a set
    /// http://redis.io/commands/sismember
    bool SISMEMBER(const std::string& key, const std::string& member);
    
    /// Remove one or more members from a set
    /// http://redis.io/commands/srem
    int64_t SREM(const std::string& key, const StringList& members);
    
    /// Incrementally iterate Set elements
    /// http://redis.io/commands/sscan
    void SSCAN(const std::string& key, int64_t cursor, int count=10);
    
    /// Returns all the members of the set value stored at key.
    /// Time complexity: O(N) where N is the set cardinality.
    /// http://redis.io/commands/smembers
    StringList SMEMBERS(const std::string& key);
    

    /// CONNECTION COMMANDS
    
    /// Authenticate to the server
    /// http://redis.io/commands/auth
    bool AUTH(const std::string& password);

    /// Echo the given string
    /// http://redis.io/commands/echo
    std::string ECHO(const std::string& message);

    /// Ping the server
    /// http://redis.io/commands/ping
    std::string PING(const std::string& message="");
    
    /// Close the connection
    /// http://redis.io/commands/quit
    void QUIT();
    
    /// @endsection

private:
    int64_t BITOP(const std::string& op, const std::string& destkey, const StringList& srckeys);
    
public:
    class connection_error : public std::exception {
    public:
        connection_error(const std::string& msg) _NOEXCEPT :
            _what(msg)
        {}
        virtual ~connection_error() _NOEXCEPT {}
        virtual const char* what() const _NOEXCEPT {
            return _what.c_str();
        }
    private:
        std::string _what;
    };
    class run_command_error : public std::exception {
    public:
        run_command_error(const std::string& msg) _NOEXCEPT :
            _what(msg)
        {}
        virtual ~run_command_error() _NOEXCEPT {}
        virtual const char* what() const _NOEXCEPT {
            return _what.c_str();
        }
    private:
        std::string _what;
    };
private:
    std::shared_ptr<redis3m::simple_pool> _connectionPool;
};

#endif	/* ZDBPROXY_H */

