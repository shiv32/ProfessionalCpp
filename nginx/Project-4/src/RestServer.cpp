#include "RestServer.h"
#include "RequestHandler.h"
#include "RedisCache.h"
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <iostream>

RestServer::RestServer(int port)
    : port(port), server(nullptr), running(false) {}

RestServer::~RestServer() {
    stop();
}

void RestServer::start() {
    try {
        auto cache = std::make_shared<RedisCache>();
        if (!cache->connect()) {
            std::cerr << "Failed to connect to Redis" << std::endl;
            return;
        }
        
        socket = std::make_unique<Poco::Net::ServerSocket>(port);
        auto factory = std::make_unique<RequestHandlerFactory>(cache);
        
        Poco::Net::HTTPServerParams* params = new Poco::Net::HTTPServerParams();
        params->setMaxThreads(10);
        params->setMaxQueued(20);
        
        server = std::make_unique<Poco::Net::HTTPServer>(
            factory.release(), *socket, params);
        
        server->start();
        running = true;
        
        std::cout << "REST API Server started on port " << port << std::endl;
    } catch (std::exception& e) {
        std::cerr << "Error starting server: " << e.what() << std::endl;
    }
}

void RestServer::stop() {
    if (server) {
        server->stop();
        server.reset();
        running = false;
        std::cout << "REST API Server stopped" << std::endl;
    }
}

bool RestServer::isRunning() const {
    return running;
}
