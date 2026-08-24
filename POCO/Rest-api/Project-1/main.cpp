#include <iostream>
#include <map>
#include <string>

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServerParams.h>

#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/Dynamic/Var.h>

using namespace Poco::Net;
using namespace Poco::JSON;
using namespace Poco::Dynamic;

// --------------------------------------------------
// In-memory database
// --------------------------------------------------

struct User
{
    int id;
    std::string name;
    std::string email;
};

std::map<int, User> users =
    {
        {1, {1, "Shiv", "shiv@example.com"}},
        {2, {2, "Rahul", "rahul@example.com"}}};

// --------------------------------------------------
// Helper: send JSON response
// --------------------------------------------------

void sendJson(
    HTTPServerResponse &response,
    int status,
    const Object &object)
{
    response.setStatus(
        static_cast<HTTPResponse::HTTPStatus>(status));

    response.setContentType("application/json");

    object.stringify(response.send());
}

// --------------------------------------------------
// GET /api/health
// --------------------------------------------------

class HealthHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        Object json;

        json.set("status", "OK");
        json.set("message", "Server is running");

        sendJson(response, HTTPResponse::HTTP_OK, json);
    }
};

// --------------------------------------------------
// GET /api/users
// --------------------------------------------------

class GetUsersHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        Object json;

        Poco::JSON::Array::Ptr array =
            new Poco::JSON::Array;

        for (const auto &[id, user] : users)
        {
            Object::Ptr obj = new Object;

            obj->set("id", user.id);
            obj->set("name", user.name);
            obj->set("email", user.email);

            array->add(obj);
        }

        json.set("users", array);

        sendJson(response, HTTPResponse::HTTP_OK, json);
    }
};

// --------------------------------------------------
// GET /api/users/{id}
// --------------------------------------------------

class GetUserHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        std::string path = request.getURI();

        // Example:
        // /api/users/1

        std::string idString =
            path.substr(std::string("/api/users/").length());

        int id = std::stoi(idString);

        auto it = users.find(id);

        if (it == users.end())
        {
            Object json;

            json.set("error", "User not found");

            sendJson(
                response,
                HTTPResponse::HTTP_NOT_FOUND,
                json);

            return;
        }

        Object json;

        json.set("id", it->second.id);
        json.set("name", it->second.name);
        json.set("email", it->second.email);

        sendJson(response, HTTPResponse::HTTP_OK, json);
    }
};

// --------------------------------------------------
// POST /api/users
// --------------------------------------------------

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
                parser.parse(request.stream());

            Object::Ptr json =
                result.extract<Object::Ptr>();

            std::string name =
                json->getValue<std::string>("name");

            std::string email =
                json->getValue<std::string>("email");

            int id = users.empty()
                         ? 1
                         : users.rbegin()->first + 1;

            users[id] =
                {
                    id,
                    name,
                    email};

            Object responseJson;

            responseJson.set("message", "User created");
            responseJson.set("id", id);
            responseJson.set("name", name);
            responseJson.set("email", email);

            sendJson(
                response,
                HTTPResponse::HTTP_CREATED,
                responseJson);
        }
        catch (...)
        {
            Object json;

            json.set("error", "Invalid JSON");

            sendJson(
                response,
                HTTPResponse::HTTP_BAD_REQUEST,
                json);
        }
    }
};

// --------------------------------------------------
// PUT /api/users/{id}
// --------------------------------------------------

class UpdateUserHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        try
        {
            std::string path = request.getURI();

            int id = std::stoi(
                path.substr(
                    std::string("/api/users/").length()));

            auto it = users.find(id);

            if (it == users.end())
            {
                Object json;
                json.set("error", "User not found");

                sendJson(
                    response,
                    HTTPResponse::HTTP_NOT_FOUND,
                    json);

                return;
            }

            Parser parser;

            Var result =
                parser.parse(request.stream());

            Object::Ptr json =
                result.extract<Object::Ptr>();

            it->second.name =
                json->getValue<std::string>("name");

            it->second.email =
                json->getValue<std::string>("email");

            Object responseJson;

            responseJson.set("message", "User updated");
            responseJson.set("id", id);
            responseJson.set("name", it->second.name);
            responseJson.set("email", it->second.email);

            sendJson(
                response,
                HTTPResponse::HTTP_OK,
                responseJson);
        }
        catch (...)
        {
            Object json;
            json.set("error", "Invalid request");

            sendJson(
                response,
                HTTPResponse::HTTP_BAD_REQUEST,
                json);
        }
    }
};

// --------------------------------------------------
// PATCH /api/users/{id}
// --------------------------------------------------

class PatchUserHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        Object json;

        json.set("message",
                 "PATCH endpoint reached");

        json.set("note",
                 "Update only selected fields");

        sendJson(
            response,
            HTTPResponse::HTTP_OK,
            json);
    }
};

// --------------------------------------------------
// DELETE /api/users/{id}
// --------------------------------------------------

class DeleteUserHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        std::string path = request.getURI();

        int id = std::stoi(
            path.substr(
                std::string("/api/users/").length()));

        auto it = users.find(id);

        if (it == users.end())
        {
            Object json;

            json.set("error", "User not found");

            sendJson(
                response,
                HTTPResponse::HTTP_NOT_FOUND,
                json);

            return;
        }

        users.erase(it);

        Object json;

        json.set("message", "User deleted");
        json.set("id", id);

        sendJson(
            response,
            HTTPResponse::HTTP_OK,
            json);
    }
};

// --------------------------------------------------
// OPTIONS /api/users
// --------------------------------------------------

class OptionsHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        response.setStatus(
            HTTPResponse::HTTP_OK);

        response.set("Access-Control-Allow-Origin", "*");

        response.set(
            "Access-Control-Allow-Methods",
            "GET, POST, PUT, PATCH, DELETE, OPTIONS");

        response.set(
            "Access-Control-Allow-Headers",
            "Content-Type, Authorization");

        response.send();
    }
};

// --------------------------------------------------
// 404 Handler
// --------------------------------------------------

class NotFoundHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        Object json;

        json.set("error", "Endpoint not found");

        sendJson(
            response,
            HTTPResponse::HTTP_NOT_FOUND,
            json);
    }
};

// --------------------------------------------------
// Handler Factory
// --------------------------------------------------

class ApiHandlerFactory
    : public HTTPRequestHandlerFactory
{
public:
    HTTPRequestHandler *createRequestHandler(
        const HTTPServerRequest &request) override
    {
        const std::string method =
            request.getMethod();

        const std::string uri =
            request.getURI();

        // Health
        if (method == "GET" &&
            uri == "/api/health")
        {
            return new HealthHandler;
        }

        // GET all users
        if (method == "GET" &&
            uri == "/api/users")
        {
            return new GetUsersHandler;
        }

        // POST user
        if (method == "POST" &&
            uri == "/api/users")
        {
            return new CreateUserHandler;
        }

        // OPTIONS
        if (method == "OPTIONS" &&
            uri == "/api/users")
        {
            return new OptionsHandler;
        }

        // User ID endpoints
        if (uri.rfind("/api/users/", 0) == 0)
        {
            if (method == "GET")
                return new GetUserHandler;

            if (method == "PUT")
                return new UpdateUserHandler;

            if (method == "PATCH")
                return new PatchUserHandler;

            if (method == "DELETE")
                return new DeleteUserHandler;
        }

        return new NotFoundHandler;
    }
};

// --------------------------------------------------
// main
// --------------------------------------------------

int main()
{
    try
    {
        ServerSocket socket(8080);

        HTTPServerParams *params = new HTTPServerParams;

        params->setMaxQueued(100);
        params->setMaxThreads(16);

        HTTPServer server(
            new ApiHandlerFactory,
            socket,
            params);

        server.start();

        std::cout
            << "POCO REST API running on "
            << "http://localhost:8080\n";

        std::cout << "Press Enter to stop...\n";

        std::cin.get();

        server.stop();
    }
    catch (const std::exception &e)
    {
        std::cerr
            << "Error: "
            << e.what()
            << '\n';

        return 1;
    }

    return 0;
}