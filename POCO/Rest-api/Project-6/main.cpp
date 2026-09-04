#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/SecureServerSocket.h>
#include <Poco/Net/Context.h>
#include <Poco/Net/SSLManager.h>

#include <iostream>
#include <memory>

using namespace Poco::Net;

// -----------------------------
// Request Handler
// -----------------------------
class HelloHandler : public HTTPRequestHandler
{
public:
    void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) override
    {
        response.setContentType("application/json");
        response.setStatus(HTTPResponse::HTTP_OK);

        response.send()
            << R"({"message":"Hello from POCO HTTPS API"})";
    }
};

// -----------------------------
// Handler Factory
// -----------------------------
class HandlerFactory : public HTTPRequestHandlerFactory
{
public:
    HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &request) override
    {
        if (request.getURI() == "/api/hello")
            return new HelloHandler();

        return nullptr;
    }
};

// -----------------------------
// Main
// -----------------------------
int main()
{
    try
    {
        // SSL/TLS context
        Poco::Net::Context::Ptr context =
            new Poco::Net::Context(
                Poco::Net::Context::SERVER_USE,
                "../server.key",
                "../server.crt",
                "",
                Poco::Net::Context::VERIFY_NONE);

        // Initialize SSL
        Poco::Net::SSLManager::instance().initializeServer(nullptr, nullptr, context);

        // HTTPS socket
        SecureServerSocket socket(8443);

        // HTTP server running over HTTPS
        HTTPServer server(new HandlerFactory(), socket, new HTTPServerParams);

        server.start();

        std::cout << "HTTPS server running on port 8443\n";
        std::cout << "Try:\n";
        std::cout << "curl -k https://localhost:8443/api/hello\n";

        std::cin.get();

        server.stop();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}