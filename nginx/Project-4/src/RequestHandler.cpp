#include "RequestHandler.h"
#include "RedisCache.h"
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <sstream>
#include <iostream>

using namespace rapidjson;

RequestHandler::RequestHandler(std::shared_ptr<RedisCache> cache)
    : cache(cache) {}

void RequestHandler::handleRequest(
    Poco::Net::HTTPServerRequest& request,
    Poco::Net::HTTPServerResponse& response) {
    
    response.setContentType("application/json");
    response.add("Access-Control-Allow-Origin", "*");
    
    const std::string& method = request.getMethod();
    const std::string& path = request.getURI();
    
    try {
        if (method == "GET") {
            handleGetRequest(path, response);
        } else if (method == "POST") {
            handlePostRequest(path, request, response);
        } else {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
            std::ostream& ostr = response.send();
            ostr << R"({"error": "Method not allowed"})";
        }
    } catch (std::exception& e) {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
        std::ostream& ostr = response.send();
        ostr << R"({"error": ")" << e.what() << R"("})";
    }
}

void RequestHandler::handleGetRequest(
    const std::string& path,
    Poco::Net::HTTPServerResponse& response) {
    
    if (path == "/") {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        std::ostream& ostr = response.send();
        
        Document doc;
        doc.SetObject();
        Document::AllocatorType& allocator = doc.GetAllocator();
        
        doc.AddMember("message", Value("Welcome to REST API Server", allocator), allocator);
        
        Value endpoints(kObjectType);
        endpoints.AddMember("GET /", Value("This message", allocator), allocator);
        endpoints.AddMember("GET /api/health", Value("Health check", allocator), allocator);
        endpoints.AddMember("GET /api/data/:key", Value("Get data from cache", allocator), allocator);
        endpoints.AddMember("POST /api/data", Value("Store data in cache", allocator), allocator);
        doc.AddMember("endpoints", endpoints, allocator);
        
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);
        ostr << buffer.GetString();
    }
    else if (path == "/api/health") {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
        std::ostream& ostr = response.send();
        
        Document doc;
        doc.SetObject();
        Document::AllocatorType& allocator = doc.GetAllocator();
        
        doc.AddMember("status", Value("healthy", allocator), allocator);
        std::string cacheStatus = cache->isConnected() ? "connected" : "disconnected";
        doc.AddMember("cache", Value(cacheStatus.c_str(), allocator), allocator);
        
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);
        ostr << buffer.GetString();
    }
    else if (path.find("/api/data/") == 0) {
        std::string key = path.substr(10); // Remove "/api/data/"
        
        if (cache->exists(key)) {
            std::string value = cache->get(key);
            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
            std::ostream& ostr = response.send();
            
            Document doc;
            doc.SetObject();
            Document::AllocatorType& allocator = doc.GetAllocator();
            doc.AddMember("key", Value(key.c_str(), allocator), allocator);
            doc.AddMember("value", Value(value.c_str(), allocator), allocator);
            
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            doc.Accept(writer);
            ostr << buffer.GetString();
        } else {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
            std::ostream& ostr = response.send();
            
            Document doc;
            doc.SetObject();
            Document::AllocatorType& allocator = doc.GetAllocator();
            doc.AddMember("error", Value("Key not found", allocator), allocator);
            
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            doc.Accept(writer);
            ostr << buffer.GetString();
        }
    }
    else {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        std::ostream& ostr = response.send();
        
        Document doc;
        doc.SetObject();
        Document::AllocatorType& allocator = doc.GetAllocator();
        doc.AddMember("error", Value("Endpoint not found", allocator), allocator);
        
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);
        ostr << buffer.GetString();
    }
}

void RequestHandler::handlePostRequest(
    const std::string& path,
    Poco::Net::HTTPServerRequest& request,
    Poco::Net::HTTPServerResponse& response) {
    
    if (path == "/api/data") {
        std::istream& istr = request.stream();
        std::string body;
        std::string line;
        
        while (std::getline(istr, line)) {
            body += line;
        }
        
        try {
            Document doc;
            doc.Parse(body.c_str());
            
            if (doc.HasParseError()) {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                std::ostream& ostr = response.send();
                
                Document errDoc;
                errDoc.SetObject();
                Document::AllocatorType& allocator = errDoc.GetAllocator();
                errDoc.AddMember("error", Value("Invalid JSON format", allocator), allocator);
                
                StringBuffer buffer;
                Writer<StringBuffer> writer(buffer);
                errDoc.Accept(writer);
                ostr << buffer.GetString();
                return;
            }
            
            if (!doc.HasMember("key") || !doc.HasMember("value") ||
                !doc["key"].IsString() || !doc["value"].IsString()) {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_BAD_REQUEST);
                std::ostream& ostr = response.send();
                
                Document errDoc;
                errDoc.SetObject();
                Document::AllocatorType& allocator = errDoc.GetAllocator();
                errDoc.AddMember("error", Value("Missing key or value", allocator), allocator);
                
                StringBuffer buffer;
                Writer<StringBuffer> writer(buffer);
                errDoc.Accept(writer);
                ostr << buffer.GetString();
                return;
            }
            
            std::string key = doc["key"].GetString();
            std::string value = doc["value"].GetString();
            
            if (cache->set(key, value)) {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                std::ostream& ostr = response.send();
                
                Document succDoc;
                succDoc.SetObject();
                Document::AllocatorType& allocator = succDoc.GetAllocator();
                succDoc.AddMember("message", Value("Data stored", allocator), allocator);
                succDoc.AddMember("key", Value(key.c_str(), allocator), allocator);
                
                StringBuffer buffer;
                Writer<StringBuffer> writer(buffer);
                succDoc.Accept(writer);
                ostr << buffer.GetString();
            } else {
                response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
                std::ostream& ostr = response.send();
                
                Document errDoc;
                errDoc.SetObject();
                Document::AllocatorType& allocator = errDoc.GetAllocator();
                errDoc.AddMember("error", Value("Failed to store data", allocator), allocator);
                
                StringBuffer buffer;
                Writer<StringBuffer> writer(buffer);
                errDoc.Accept(writer);
                ostr << buffer.GetString();
            }
        } catch (std::exception& e) {
            response.setStatus(Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            std::ostream& ostr = response.send();
            
            Document errDoc;
            errDoc.SetObject();
            Document::AllocatorType& allocator = errDoc.GetAllocator();
            std::string errMsg = std::string("Error: ") + e.what();
            errDoc.AddMember("error", Value(errMsg.c_str(), allocator), allocator);
            
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            errDoc.Accept(writer);
            ostr << buffer.GetString();
        }
    } else {
        response.setStatus(Poco::Net::HTTPResponse::HTTP_NOT_FOUND);
        std::ostream& ostr = response.send();
        
        Document doc;
        doc.SetObject();
        Document::AllocatorType& allocator = doc.GetAllocator();
        doc.AddMember("error", Value("Endpoint not found", allocator), allocator);
        
        StringBuffer buffer;
        Writer<StringBuffer> writer(buffer);
        doc.Accept(writer);
        ostr << buffer.GetString();
    }
}

RequestHandlerFactory::RequestHandlerFactory(std::shared_ptr<RedisCache> cache)
    : cache(cache) {}

Poco::Net::HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(
    const Poco::Net::HTTPServerRequest& request) {
    return new RequestHandler(cache);
}
