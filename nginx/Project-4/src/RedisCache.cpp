#include "RedisCache.h"
#include <iostream>
#include <cstring>

RedisCache::RedisCache(const std::string& host, int port)
    : host(host), port(port), context(nullptr) {}

RedisCache::~RedisCache() {
    disconnect();
}

bool RedisCache::connect() {
    context = redisConnect(host.c_str(), port);
    
    if (context == nullptr || context->err) {
        if (context) {
            std::cerr << "Redis connection error: " << context->errstr << std::endl;
            redisFree(context);
            context = nullptr;
        } else {
            std::cerr << "Redis connection failed: cannot allocate context" << std::endl;
        }
        return false;
    }
    
    std::cout << "Connected to Redis at " << host << ":" << port << std::endl;
    return true;
}

void RedisCache::disconnect() {
    if (context) {
        redisFree(context);
        context = nullptr;
    }
}

bool RedisCache::isConnected() const {
    return context != nullptr;
}

bool RedisCache::set(const std::string& key, const std::string& value) {
    if (!isConnected()) return false;
    
    redisReply* reply = (redisReply*)redisCommand(context, "SET %s %s", key.c_str(), value.c_str());
    
    if (!reply) return false;
    
    bool success = (reply->type == REDIS_REPLY_STATUS);
    freeReplyObject(reply);
    
    return success;
}

std::string RedisCache::get(const std::string& key) {
    if (!isConnected()) return "";
    
    redisReply* reply = (redisReply*)redisCommand(context, "GET %s", key.c_str());
    
    if (!reply) return "";
    
    std::string result;
    if (reply->type == REDIS_REPLY_STRING) {
        result = std::string(reply->str, reply->len);
    }
    
    freeReplyObject(reply);
    return result;
}

bool RedisCache::exists(const std::string& key) {
    if (!isConnected()) return false;
    
    redisReply* reply = (redisReply*)redisCommand(context, "EXISTS %s", key.c_str());
    
    if (!reply) return false;
    
    bool exists = (reply->integer == 1);
    freeReplyObject(reply);
    
    return exists;
}

bool RedisCache::del(const std::string& key) {
    if (!isConnected()) return false;
    
    redisReply* reply = (redisReply*)redisCommand(context, "DEL %s", key.c_str());
    
    if (!reply) return false;
    
    bool success = (reply->integer > 0);
    freeReplyObject(reply);
    
    return success;
}
