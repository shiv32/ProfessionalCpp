/*
    Improved code for patch and option endpoint in postman client.
*/

#include <iostream>
#include <map>
#include <string>

// POCO Net
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>

// POCO JSON
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>

// POCO Dynamic
#include <Poco/Dynamic/Var.h>

// ============================================================
// Using declarations
// ============================================================

using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPResponse;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::ServerSocket;

using Poco::JSON::Object;
using Poco::JSON::Parser;

using Poco::Dynamic::Var;

// ============================================================
// MODEL
// ============================================================

struct User
{
    int id;
    std::string name;
    std::string email;
};

// ============================================================
// IN-MEMORY DATABASE
// ============================================================

std::map<int, User> users =
    {
        {1, {1, "Shiv", "shiv@example.com"}},
        {2, {2, "Rahul", "rahul@example.com"}}};

// ============================================================
// HELPER FUNCTION
// Send JSON response
// ============================================================

void sendJson(
    HTTPServerResponse &response,
    HTTPResponse::HTTPStatus status,
    const Object &json)
{
    response.setStatus(status);

    response.setContentType(
        "application/json");

    std::ostream &out =
        response.send();

    json.stringify(out);
}

// ============================================================
// GET /api/health
// ============================================================

class HealthHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        Object json;

        json.set(
            "status",
            "OK");

        json.set(
            "message",
            "POCO REST API is running");

        sendJson(
            response,
            HTTPResponse::HTTP_OK,
            json);
    }
};

// ============================================================
// GET /api/users
// ============================================================

class GetUsersHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        Object json;

        // Explicitly use Poco::JSON::Array
        // to avoid Array ambiguity.

        Poco::JSON::Array::Ptr usersArray =
            new Poco::JSON::Array;

        for (const auto &[id, user] : users)
        {
            Object::Ptr userJson =
                new Object;

            userJson->set(
                "id",
                user.id);

            userJson->set(
                "name",
                user.name);

            userJson->set(
                "email",
                user.email);

            usersArray->add(
                userJson);
        }

        json.set(
            "users",
            usersArray);

        sendJson(
            response,
            HTTPResponse::HTTP_OK,
            json);
    }
};

// ============================================================
// GET /api/users/{id}
// ============================================================

class GetUserHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        try
        {
            std::string uri =
                request.getURI();

            const std::string prefix =
                "/api/users/";

            std::string idString =
                uri.substr(prefix.length());

            int id =
                std::stoi(idString);

            auto it =
                users.find(id);

            // User not found
            if (it == users.end())
            {
                Object json;

                json.set(
                    "error",
                    "User not found");

                sendJson(
                    response,
                    HTTPResponse::HTTP_NOT_FOUND,
                    json);

                return;
            }

            Object json;

            json.set(
                "id",
                it->second.id);

            json.set(
                "name",
                it->second.name);

            json.set(
                "email",
                it->second.email);

            sendJson(
                response,
                HTTPResponse::HTTP_OK,
                json);
        }
        catch (...)
        {
            Object json;

            json.set(
                "error",
                "Invalid user ID");

            sendJson(
                response,
                HTTPResponse::HTTP_BAD_REQUEST,
                json);
        }
    }
};

// ============================================================
// POST /api/users
// ============================================================

class CreateUserHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        try
        {
            Parser parser;

            Var result =
                parser.parse(
                    request.stream());

            Object::Ptr json =
                result.extract<Object::Ptr>();

            // Check required fields

            if (!json->has("name") ||
                !json->has("email"))
            {
                Object error;

                error.set(
                    "error",
                    "name and email are required");

                sendJson(
                    response,
                    HTTPResponse::HTTP_BAD_REQUEST,
                    error);

                return;
            }

            // Read JSON

            std::string name =
                json->getValue<std::string>(
                    "name");

            std::string email =
                json->getValue<std::string>(
                    "email");

            // Generate ID

            int id =
                users.empty()
                    ? 1
                    : users.rbegin()->first + 1;

            // Save user

            users[id] =
                {
                    id,
                    name,
                    email};

            // Response

            Object responseJson;

            responseJson.set(
                "message",
                "User created");

            responseJson.set(
                "id",
                id);

            responseJson.set(
                "name",
                name);

            responseJson.set(
                "email",
                email);

            sendJson(
                response,
                HTTPResponse::HTTP_CREATED,
                responseJson);
        }
        catch (...)
        {
            Object json;

            json.set(
                "error",
                "Invalid JSON");

            sendJson(
                response,
                HTTPResponse::HTTP_BAD_REQUEST,
                json);
        }
    }
};

// ============================================================
// PUT /api/users/{id}
// ============================================================

class UpdateUserHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        try
        {
            std::string uri =
                request.getURI();

            const std::string prefix =
                "/api/users/";

            int id =
                std::stoi(
                    uri.substr(prefix.length()));

            // Find user

            auto it =
                users.find(id);

            if (it == users.end())
            {
                Object json;

                json.set(
                    "error",
                    "User not found");

                sendJson(
                    response,
                    HTTPResponse::HTTP_NOT_FOUND,
                    json);

                return;
            }

            // Parse JSON

            Parser parser;

            Var result =
                parser.parse(
                    request.stream());

            Object::Ptr json =
                result.extract<Object::Ptr>();

            // PUT requires both fields

            if (!json->has("name") ||
                !json->has("email"))
            {
                Object error;

                error.set(
                    "error",
                    "PUT requires name and email");

                sendJson(
                    response,
                    HTTPResponse::HTTP_BAD_REQUEST,
                    error);

                return;
            }

            // Update complete resource

            it->second.name =
                json->getValue<std::string>(
                    "name");

            it->second.email =
                json->getValue<std::string>(
                    "email");

            // Response

            Object responseJson;

            responseJson.set(
                "message",
                "User updated using PUT");

            responseJson.set(
                "id",
                it->second.id);

            responseJson.set(
                "name",
                it->second.name);

            responseJson.set(
                "email",
                it->second.email);

            sendJson(
                response,
                HTTPResponse::HTTP_OK,
                responseJson);
        }
        catch (...)
        {
            Object json;

            json.set(
                "error",
                "Invalid request");

            sendJson(
                response,
                HTTPResponse::HTTP_BAD_REQUEST,
                json);
        }
    }
};

// ============================================================
// PATCH /api/users/{id}
// ============================================================

class PatchUserHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        try
        {
            std::string uri =
                request.getURI();

            const std::string prefix =
                "/api/users/";

            int id =
                std::stoi(
                    uri.substr(prefix.length()));

            // Find user

            auto it =
                users.find(id);

            if (it == users.end())
            {
                Object json;

                json.set(
                    "error",
                    "User not found");

                sendJson(
                    response,
                    HTTPResponse::HTTP_NOT_FOUND,
                    json);

                return;
            }

            // Parse JSON

            Parser parser;

            Var result =
                parser.parse(
                    request.stream());

            Object::Ptr json =
                result.extract<Object::Ptr>();

            // --------------------------------------------
            // PATCH name
            // --------------------------------------------

            if (json->has("name"))
            {
                it->second.name =
                    json->getValue<std::string>(
                        "name");
            }

            // --------------------------------------------
            // PATCH email
            // --------------------------------------------

            if (json->has("email"))
            {
                it->second.email =
                    json->getValue<std::string>(
                        "email");
            }

            // --------------------------------------------
            // Response
            // --------------------------------------------

            Object responseJson;

            responseJson.set(
                "message",
                "User partially updated");

            responseJson.set(
                "id",
                it->second.id);

            responseJson.set(
                "name",
                it->second.name);

            responseJson.set(
                "email",
                it->second.email);

            sendJson(
                response,
                HTTPResponse::HTTP_OK,
                responseJson);
        }
        catch (...)
        {
            Object json;

            json.set(
                "error",
                "Invalid JSON");

            sendJson(
                response,
                HTTPResponse::HTTP_BAD_REQUEST,
                json);
        }
    }
};

// ============================================================
// DELETE /api/users/{id}
// ============================================================

class DeleteUserHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        try
        {
            std::string uri =
                request.getURI();

            const std::string prefix =
                "/api/users/";

            int id =
                std::stoi(
                    uri.substr(prefix.length()));

            // Find user

            auto it =
                users.find(id);

            if (it == users.end())
            {
                Object json;

                json.set(
                    "error",
                    "User not found");

                sendJson(
                    response,
                    HTTPResponse::HTTP_NOT_FOUND,
                    json);

                return;
            }

            // Delete

            users.erase(it);

            // Response

            Object json;

            json.set(
                "message",
                "User deleted");

            json.set(
                "id",
                id);

            sendJson(
                response,
                HTTPResponse::HTTP_OK,
                json);
        }
        catch (...)
        {
            Object json;

            json.set(
                "error",
                "Invalid user ID");

            sendJson(
                response,
                HTTPResponse::HTTP_BAD_REQUEST,
                json);
        }
    }
};

// ============================================================
// OPTIONS
// ============================================================

class OptionsHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        // HTTP status

        response.setStatus(
            HTTPResponse::HTTP_OK);

        // Supported HTTP methods

        response.set(
            "Allow",
            "GET, POST, PUT, PATCH, DELETE, OPTIONS");

        // CORS headers

        response.set(
            "Access-Control-Allow-Origin",
            "*");

        response.set(
            "Access-Control-Allow-Methods",
            "GET, POST, PUT, PATCH, DELETE, OPTIONS");

        response.set(
            "Access-Control-Allow-Headers",
            "Content-Type, Authorization");

        // Content type

        response.setContentType(
            "text/plain");

        // Response body

        std::ostream &out =
            response.send();

        out << "OPTIONS OK\n";

        out << "Allowed methods: "
            << "GET, POST, PUT, PATCH, DELETE, OPTIONS\n";
    }
};

// ============================================================
// 404 HANDLER
// ============================================================

class NotFoundHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        Object json;

        json.set(
            "error",
            "Endpoint not found");

        sendJson(
            response,
            HTTPResponse::HTTP_NOT_FOUND,
            json);
    }
};

// ============================================================
// REQUEST HANDLER FACTORY
// ============================================================

class ApiHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    HTTPRequestHandler *createRequestHandler(
        const HTTPServerRequest &request) override
    {
        const std::string method =
            request.getMethod();

        const std::string uri =
            request.getURI();

        // ====================================================
        // GET /api/health
        // ====================================================

        if (method == "GET" &&
            uri == "/api/health")
        {
            return new HealthHandler;
        }

        // ====================================================
        // GET /api/users
        // ====================================================

        if (method == "GET" &&
            uri == "/api/users")
        {
            return new GetUsersHandler;
        }

        // ====================================================
        // POST /api/users
        // ====================================================

        if (method == "POST" &&
            uri == "/api/users")
        {
            return new CreateUserHandler;
        }

        // ====================================================
        // OPTIONS
        // ====================================================

        if (method == "OPTIONS" &&
            (uri == "/api/users" ||
             uri.rfind("/api/users/", 0) == 0))
        {
            return new OptionsHandler;
        }

        // ====================================================
        // /api/users/{id}
        // ====================================================

        if (uri.rfind("/api/users/", 0) == 0)
        {
            // GET
            if (method == "GET")
            {
                return new GetUserHandler;
            }

            // PUT
            if (method == "PUT")
            {
                return new UpdateUserHandler;
            }

            // PATCH
            if (method == "PATCH")
            {
                return new PatchUserHandler;
            }

            // DELETE
            if (method == "DELETE")
            {
                return new DeleteUserHandler;
            }
        }

        // ====================================================
        // Unknown endpoint
        // ====================================================

        return new NotFoundHandler;
    }
};

// ============================================================
// MAIN
// ============================================================

int main()
{
    try
    {
        const int port = 8080;

        // Create socket

        ServerSocket socket(port);

        // Server parameters

        HTTPServerParams *params =
            new HTTPServerParams;

        params->setMaxQueued(100);

        params->setMaxThreads(16);

        // Create server

        HTTPServer server(
            new ApiHandlerFactory,
            socket,
            params);

        // Start

        server.start();

        // Console output

        std::cout
            << "========================================\n";

        std::cout
            << "       POCO C++17 REST API SERVER\n";

        std::cout
            << "========================================\n";

        std::cout
            << "Server running at:\n";

        std::cout
            << "http://localhost:"
            << port
            << "\n\n";

        std::cout
            << "Available endpoints:\n\n";

        std::cout
            << "GET     /api/health\n";

        std::cout
            << "GET     /api/users\n";

        std::cout
            << "GET     /api/users/{id}\n";

        std::cout
            << "POST    /api/users\n";

        std::cout
            << "PUT     /api/users/{id}\n";

        std::cout
            << "PATCH   /api/users/{id}\n";

        std::cout
            << "DELETE  /api/users/{id}\n";

        std::cout
            << "OPTIONS /api/users\n\n";

        std::cout
            << "Press ENTER to stop...\n";

        std::cin.get();

        // Stop

        server.stop();
    }
    catch (const std::exception &e)
    {
        std::cerr
            << "Server error: "
            << e.what()
            << '\n';

        return 1;
    }

    return 0;
}