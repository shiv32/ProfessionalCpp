#pragma once

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <memory>

class RestServer {
public:
    RestServer(int port = 8080);
    ~RestServer();
    
    void start();
    void stop();
    bool isRunning() const;
    
private:
    int port;
    std::unique_ptr<Poco::Net::ServerSocket> socket;
    std::unique_ptr<Poco::Net::HTTPServer> server;
    bool running;
};
