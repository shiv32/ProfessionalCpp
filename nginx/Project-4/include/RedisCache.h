#pragma once

#include <string>
#include <memory>
#include <hiredis/hiredis.h>

class RedisCache {
public:
    RedisCache(const std::string& host = "127.0.0.1", int port = 6379);
    ~RedisCache();
    
    bool connect();
    void disconnect();
    bool isConnected() const;
    
    bool set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    bool exists(const std::string& key);
    bool del(const std::string& key);
    
private:
    std::string host;
    int port;
    redisContext* context;
};
