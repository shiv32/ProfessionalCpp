#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>

#include <Poco/JWT/Token.h>
#include <Poco/JWT/Signer.h>
#include <Poco/JWT/JWTException.h>

#include <Poco/Timestamp.h>

#include <iostream>
#include <string>

using namespace Poco::Net;
using namespace Poco::JWT;

// Secret used to sign and verify JWT
const std::string SECRET =
    "0123456789ABCDEF0123456789ABCDEF";

// --------------------------------------------------
// Request Handler
// --------------------------------------------------

class RequestHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        response.setContentType("application/json");

        // ------------------------------------------
        // POST /login
        // ------------------------------------------

        if (request.getMethod() == HTTPRequest::HTTP_POST && request.getURI() == "/api/login")
        {
            createToken(response);
            return;
        }

        // ------------------------------------------
        // GET /api/hello
        // ------------------------------------------

        if (request.getMethod() == HTTPRequest::HTTP_GET && request.getURI() == "/api/hello")
        {
            protectedEndpoint(request, response);
            return;
        }

        // ------------------------------------------
        // 404
        // ------------------------------------------

        response.setStatus(
            HTTPResponse::HTTP_NOT_FOUND);

        response.send()
            << R"({"error":"Not Found"})";
    }

private:
    // ----------------------------------------------
    // Create JWT
    // ----------------------------------------------

    void createToken(HTTPServerResponse &response)
    {
        Token token;

        token.setType("JWT");
        token.setSubject("123");

        token.payload().set(
            "name",
            std::string("Shiv"));

        token.payload().set(
            "role",
            std::string("user"));

        token.setIssuedAt(Poco::Timestamp());

        // JWT expires after 1 hour
        Poco::Timestamp expiration =
            Poco::Timestamp() +
            Poco::Timespan(1, 0, 0, 0, 0);

        token.setExpiration(expiration);

        // POCO JWT signer
        Signer signer(SECRET);

        std::string jwt =
            signer.sign(
                token,
                Signer::ALGO_HS256);

        response.setStatus(
            HTTPResponse::HTTP_OK);

        response.send()
            << "{"
            << "\"token\":\""
            << jwt
            << "\""
            << "}";
    }

    // ----------------------------------------------
    // Protected endpoint
    // ----------------------------------------------

    void protectedEndpoint(
        HTTPServerRequest &request,
        HTTPServerResponse &response)
    {
        std::string authorization =
            request.get(
                "Authorization",
                "");

        const std::string prefix = "Bearer ";

        if (authorization.rfind(prefix, 0) != 0)
        {
            unauthorized(response);
            return;
        }

        std::string jwt =
            authorization.substr(
                prefix.length());

        try
        {
            // POCO verifies the JWT signature
            Signer signer(SECRET);

            Token token =
                signer.verify(jwt);

            std::string name =
                token.payload()
                    .getValue<std::string>("name");

            std::string role =
                token.payload()
                    .getValue<std::string>("role");

            response.setStatus(
                HTTPResponse::HTTP_OK);

            response.send()
                << "{"
                << "\"message\":\"Hello "
                << name
                << "\","
                << "\"role\":\""
                << role
                << "\","
                << "\"jwt\":\"valid\""
                << "}";
        }
        catch (const JWTException &)
        {
            unauthorized(response);
        }
        catch (const std::exception &)
        {
            unauthorized(response);
        }
    }

    // ----------------------------------------------
    // 401
    // ----------------------------------------------

    void unauthorized(
        HTTPServerResponse &response)
    {
        response.setStatus(
            HTTPResponse::HTTP_UNAUTHORIZED);

        response.send()
            << R"({"error":"Invalid JWT"})";
    }
};

// --------------------------------------------------
// Factory
// --------------------------------------------------

class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    HTTPRequestHandler *createRequestHandler(
        const HTTPServerRequest &)
        override
    {
        return new RequestHandler();
    }
};

// --------------------------------------------------
// main
// --------------------------------------------------

int main()
{
    const int port = 8080;

    ServerSocket socket(port);

    HTTPServer server(
        new RequestHandlerFactory(),
        socket,
        new HTTPServerParams);

    server.start();

    std::cout
        << "POCO JWT REST API running on "
        << "http://localhost:"
        << port
        << std::endl;

    std::cout
        << "POST /api/login"
        << std::endl;

    std::cout
        << "GET /api/hello"
        << std::endl;

    std::cin.get();

    server.stop();

    return 0;
}