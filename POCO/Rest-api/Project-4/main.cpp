#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>

#include <iostream>

using namespace Poco::Net;

// Handles HTTP requests
class RequestHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        std::cout << request.getMethod()
                  << " " << request.getURI()
                  << std::endl;

        response.setStatus(HTTPResponse::HTTP_OK);
        response.setContentType("application/json");

        std::ostream &out = response.send();

        if (request.getURI() == "/api/hello")
        {
            out << R"({"message":"Hello from C++ POCO API"})";
        }
        else
        {
            response.setStatus(HTTPResponse::HTTP_NOT_FOUND);
            out << R"({"error":"Not Found"})";
        }
    }
};

// Creates request handlers
class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    HTTPRequestHandler *createRequestHandler(
        const HTTPServerRequest &) override
    {
        return new RequestHandler();
    }
};

int main()
{
    const int port = 8080;

    ServerSocket socket(port);

    HTTPServer server(
        new RequestHandlerFactory(),
        socket,
        new HTTPServerParams);

    server.start();

    std::cout << "POCO REST API running on port "
              << port << std::endl;

    std::cout << "Try: http://localhost:8080/api/hello"
              << std::endl;

    std::cin.get();

    server.stop();

    return 0;
}