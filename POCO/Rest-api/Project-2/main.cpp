#include <iostream>
#include <string>

// ============================================================
// POCO NET
// ============================================================

#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPResponse.h>

// ============================================================
// POCO JSON
// ============================================================

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>

// ============================================================
// POCO REDIS
// ============================================================

#include <Poco/Redis/Client.h>
#include <Poco/Redis/Command.h>
#include <Poco/Redis/Type.h>

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

using Poco::Redis::Client;
using Poco::Redis::Command;

// ============================================================
// Redis configuration
// ============================================================

const std::string REDIS_HOST = "127.0.0.1";
const int REDIS_PORT = 6379;

// ============================================================
// JSON response helper
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
// Extract Redis key
//
// /api/redis/name
//
// -> name
// ============================================================

std::string getRedisKey(
    const std::string &uri)
{
    const std::string prefix =
        "/api/redis/";

    if (uri.rfind(prefix, 0) != 0)
    {
        return "";
    }

    return uri.substr(
        prefix.length());
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
// GET /api/redis/health
// ============================================================

class RedisHealthHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        try
        {
            Client client(
                REDIS_HOST,
                REDIS_PORT);

            Command command =
                Command::ping();

            std::string result =
                client.execute<std::string>(
                    command);

            client.disconnect();

            Object json;

            json.set(
                "status",
                "OK");

            json.set(
                "redis",
                result);

            sendJson(
                response,
                HTTPResponse::HTTP_OK,
                json);
        }
        catch (const std::exception &e)
        {
            Object json;

            json.set(
                "status",
                "ERROR");

            json.set(
                "message",
                e.what());

            sendJson(
                response,
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
                json);
        }
    }
};

// ============================================================
// POST /api/redis/{key}
//
// Body:
//
// {
//     "value": "Shiv"
// }
//
// Redis:
//
// SET key value
// ============================================================

class RedisSetHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        try
        {
            std::string key =
                getRedisKey(
                    request.getURI());

            if (key.empty())
            {
                Object json;

                json.set(
                    "error",
                    "Invalid Redis key");

                sendJson(
                    response,
                    HTTPResponse::HTTP_BAD_REQUEST,
                    json);

                return;
            }

            // =================================================
            // Read request body
            // =================================================

            std::string body;

            std::istream &input =
                request.stream();

            std::getline(
                input,
                body,
                '\0');

            // =================================================
            // Parse JSON
            // =================================================

            Parser parser;

            Poco::Dynamic::Var parsed =
                parser.parse(body);

            Object::Ptr jsonObject =
                parsed.extract<Object::Ptr>();

            if (!jsonObject->has("value"))
            {
                Object json;

                json.set(
                    "error",
                    "value is required");

                sendJson(
                    response,
                    HTTPResponse::HTTP_BAD_REQUEST,
                    json);

                return;
            }

            std::string value =
                jsonObject->getValue<std::string>(
                    "value");

            // =================================================
            // Redis SET
            // =================================================

            Client client(
                REDIS_HOST,
                REDIS_PORT);

            Command command =
                Command::set(
                    key,
                    value);

            std::string result =
                client.execute<std::string>(
                    command);

            client.disconnect();

            // =================================================
            // Response
            // =================================================

            Object json;

            json.set(
                "message",
                "Value stored");

            json.set(
                "key",
                key);

            json.set(
                "value",
                value);

            json.set(
                "redis",
                result);

            sendJson(
                response,
                HTTPResponse::HTTP_OK,
                json);
        }
        catch (const std::exception &e)
        {
            Object json;

            json.set(
                "error",
                e.what());

            sendJson(
                response,
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
                json);
        }
    }
};

// ============================================================
// GET /api/redis/{key}
//
// IMPORTANT:
//
// Redis GET returns BulkString.
//
// POCO 1.15.3:
// BulkString is defined in:
//
// Poco/Redis/Type.h
// ============================================================

class RedisGetHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        try
        {
            std::string key =
                getRedisKey(
                    request.getURI());

            if (key.empty())
            {
                Object json;

                json.set(
                    "error",
                    "Invalid Redis key");

                sendJson(
                    response,
                    HTTPResponse::HTTP_BAD_REQUEST,
                    json);

                return;
            }

            Client client(
                REDIS_HOST,
                REDIS_PORT);

            // =================================================
            // Redis GET
            // =================================================

            Command command =
                Command::get(key);

            // IMPORTANT:
            //
            // GET -> BulkString
            //

            Poco::Redis::BulkString result =
                client.execute<Poco::Redis::BulkString>(
                    command);

            client.disconnect();

            // =================================================
            // JSON response
            // =================================================

            Object json;

            json.set(
                "key",
                key);

            // Redis key exists

            if (!result.isNull())
            {
                json.set(
                    "exists",
                    true);

                json.set(
                    "value",
                    result.value());
            }
            else
            {
                json.set(
                    "exists",
                    false);

                json.set(
                    "message",
                    "Key not found");
            }

            sendJson(
                response,
                HTTPResponse::HTTP_OK,
                json);
        }
        catch (const std::exception &e)
        {
            Object json;

            json.set(
                "error",
                e.what());

            sendJson(
                response,
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
                json);
        }
    }
};

// ============================================================
// DELETE /api/redis/{key}
//
// Redis:
//
// DEL key
// ============================================================

class RedisDeleteHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        try
        {
            std::string key =
                getRedisKey(
                    request.getURI());

            if (key.empty())
            {
                Object json;

                json.set(
                    "error",
                    "Invalid Redis key");

                sendJson(
                    response,
                    HTTPResponse::HTTP_BAD_REQUEST,
                    json);

                return;
            }

            Client client(
                REDIS_HOST,
                REDIS_PORT);

            Command command =
                Command::del(key);

            Poco::Int64 deleted =
                client.execute<Poco::Int64>(
                    command);

            client.disconnect();

            Object json;

            json.set(
                "key",
                key);

            json.set(
                "deleted",
                static_cast<long long>(
                    deleted));

            if (deleted == 1)
            {
                json.set(
                    "message",
                    "Key deleted");
            }
            else
            {
                json.set(
                    "message",
                    "Key not found");
            }

            sendJson(
                response,
                HTTPResponse::HTTP_OK,
                json);
        }
        catch (const std::exception &e)
        {
            Object json;

            json.set(
                "error",
                e.what());

            sendJson(
                response,
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
                json);
        }
    }
};

// ============================================================
// GET /api/redis/exists/{key}
//
// Redis:
//
// EXISTS key
// ============================================================

class RedisExistsHandler : public HTTPRequestHandler
{
public:
    void handleRequest(
        HTTPServerRequest &request,
        HTTPServerResponse &response) override
    {
        try
        {
            const std::string prefix =
                "/api/redis/exists/";

            const std::string uri =
                request.getURI();

            std::string key =
                uri.substr(
                    prefix.length());

            if (key.empty())
            {
                Object json;

                json.set(
                    "error",
                    "Invalid Redis key");

                sendJson(
                    response,
                    HTTPResponse::HTTP_BAD_REQUEST,
                    json);

                return;
            }

            Client client(
                REDIS_HOST,
                REDIS_PORT);

            Command command =
                Command::exists(key);

            Poco::Int64 exists =
                client.execute<Poco::Int64>(
                    command);

            client.disconnect();

            Object json;

            json.set(
                "key",
                key);

            json.set(
                "exists",
                exists == 1);

            sendJson(
                response,
                HTTPResponse::HTTP_OK,
                json);
        }
        catch (const std::exception &e)
        {
            Object json;

            json.set(
                "error",
                e.what());

            sendJson(
                response,
                HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
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
        response.setStatus(
            HTTPResponse::HTTP_OK);

        response.set(
            "Allow",
            "GET, POST, DELETE, OPTIONS");

        response.set(
            "Access-Control-Allow-Origin",
            "*");

        response.set(
            "Access-Control-Allow-Methods",
            "GET, POST, DELETE, OPTIONS");

        response.set(
            "Access-Control-Allow-Headers",
            "Content-Type, Authorization");

        response.setContentType(
            "text/plain");

        std::ostream &out =
            response.send();

        out << "OPTIONS OK\n";

        out << "Allowed methods: "
            << "GET, POST, DELETE, OPTIONS\n";
    }
};

// ============================================================
// 404
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
        // GET /api/redis/health
        // ====================================================

        if (method == "GET" &&
            uri == "/api/redis/health")
        {
            return new RedisHealthHandler;
        }

        // ====================================================
        // OPTIONS
        // ====================================================

        if (method == "OPTIONS" &&
            uri.rfind(
                "/api/redis",
                0) == 0)
        {
            return new OptionsHandler;
        }

        // ====================================================
        // EXISTS
        // ====================================================

        if (method == "GET" &&
            uri.rfind(
                "/api/redis/exists/",
                0) == 0)
        {
            return new RedisExistsHandler;
        }

        // ====================================================
        // POST
        // ====================================================

        if (method == "POST" &&
            uri.rfind(
                "/api/redis/",
                0) == 0)
        {
            return new RedisSetHandler;
        }

        // ====================================================
        // GET
        // ====================================================

        if (method == "GET" &&
            uri.rfind(
                "/api/redis/",
                0) == 0)
        {
            return new RedisGetHandler;
        }

        // ====================================================
        // DELETE
        // ====================================================

        if (method == "DELETE" &&
            uri.rfind(
                "/api/redis/",
                0) == 0)
        {
            return new RedisDeleteHandler;
        }

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

        ServerSocket socket(port);

        HTTPServerParams *params =
            new HTTPServerParams;

        params->setMaxQueued(100);

        params->setMaxThreads(16);

        HTTPServer server(
            new ApiHandlerFactory,
            socket,
            params);

        server.start();

        std::cout
            << "========================================\n";

        std::cout
            << "       POCO + REDIS REST API\n";

        std::cout
            << "========================================\n\n";

        std::cout
            << "Server: "
            << "http://localhost:"
            << port
            << "\n\n";

        std::cout
            << "Endpoints:\n\n";

        std::cout
            << "GET     /api/health\n";

        std::cout
            << "GET     /api/redis/health\n";

        std::cout
            << "POST    /api/redis/{key}\n";

        std::cout
            << "GET     /api/redis/{key}\n";

        std::cout
            << "DELETE  /api/redis/{key}\n";

        std::cout
            << "GET     /api/redis/exists/{key}\n";

        std::cout
            << "OPTIONS /api/redis/{key}\n\n";

        std::cout
            << "Press ENTER to stop...\n";

        std::cin.get();

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