#pragma once

#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <memory>

class RedisCache;

class RequestHandler : public Poco::Net::HTTPRequestHandler {
public:
    RequestHandler(std::shared_ptr<RedisCache> cache);
    
    void handleRequest(
        Poco::Net::HTTPServerRequest& request,
        Poco::Net::HTTPServerResponse& response) override;
        
private:
    std::shared_ptr<RedisCache> cache;
    
    void handleGetRequest(const std::string& path, Poco::Net::HTTPServerResponse& response);
    void handlePostRequest(const std::string& path, Poco::Net::HTTPServerRequest& request, 
                          Poco::Net::HTTPServerResponse& response);
};

class RequestHandlerFactory : public Poco::Net::HTTPRequestHandlerFactory {
public:
    RequestHandlerFactory(std::shared_ptr<RedisCache> cache);
    
    Poco::Net::HTTPRequestHandler* createRequestHandler(
        const Poco::Net::HTTPServerRequest& request) override;
        
private:
    std::shared_ptr<RedisCache> cache;
};
