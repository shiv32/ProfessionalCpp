#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <mutex>
#include <cstdlib>

// =========================
// POCO HTTP
// =========================
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerParams.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/HTTPResponse.h>

// =========================
// POCO JSON
// =========================
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

// =========================
// POCO Redis
// =========================
#include <Poco/Redis/Client.h>
#include <Poco/Redis/Command.h>
#include <Poco/Redis/Array.h>
#include <Poco/Redis/Type.h>

// =========================
// SQLite
// =========================
#include <sqlite3.h>

// =========================
// Helpers
// =========================

std::string toJsonString(Poco::JSON::Object::Ptr object)
{
    std::ostringstream oss;
    object->stringify(oss);
    return oss.str();
}

std::string toJsonString(Poco::JSON::Array::Ptr array)
{
    std::ostringstream oss;
    array->stringify(oss);
    return oss.str();
}

void sendJson(
    Poco::Net::HTTPServerResponse &response,
    Poco::Net::HTTPResponse::HTTPStatus status,
    const std::string &json)
{
    response.setStatus(status);
    response.setContentType("application/json");
    response.setContentLength(json.size());

    std::ostream &out = response.send();
    out << json;
}

Poco::JSON::Object::Ptr errorJson(const std::string &message)
{
    Poco::JSON::Object::Ptr obj = new Poco::JSON::Object;
    obj->set("error", message);
    return obj;
}

// Extract /api/users/123
bool extractId(const std::string &path, long long &id)
{
    const std::string prefix = "/api/users/";

    if (path.find(prefix) != 0)
        return false;

    std::string idString = path.substr(prefix.size());

    if (idString.empty())
        return false;

    try
    {
        size_t pos = 0;
        id = std::stoll(idString, &pos);

        return pos == idString.size();
    }
    catch (...)
    {
        return false;
    }
}

// =========================
// SQLite Repository
// =========================

struct User
{
    long long id;
    std::string name;
    std::string email;
};

class UserRepository
{
private:
    sqlite3 *db;
    std::mutex mutex;

public:
    UserRepository()
        : db(nullptr)
    {
        int rc = sqlite3_open("users.db", &db);

        if (rc != SQLITE_OK)
        {
            std::string error =
                db ? sqlite3_errmsg(db) : "Unable to open database";

            if (db)
                sqlite3_close(db);

            throw std::runtime_error(error);
        }

        initialize();
    }

    ~UserRepository()
    {
        if (db)
            sqlite3_close(db);
    }

    void initialize()
    {
        const char *sql =
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "name TEXT NOT NULL,"
            "email TEXT NOT NULL"
            ");";

        char *errorMessage = nullptr;

        int rc = sqlite3_exec(
            db,
            sql,
            nullptr,
            nullptr,
            &errorMessage);

        if (rc != SQLITE_OK)
        {
            std::string error =
                errorMessage ? errorMessage : "SQLite error";

            sqlite3_free(errorMessage);

            throw std::runtime_error(error);
        }
    }

    std::vector<User> getAll()
    {
        std::lock_guard<std::mutex> lock(mutex);

        std::vector<User> users;

        const char *sql =
            "SELECT id, name, email FROM users ORDER BY id;";

        sqlite3_stmt *stmt = nullptr;

        if (sqlite3_prepare_v2(
                db,
                sql,
                -1,
                &stmt,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(sqlite3_errmsg(db));
        }

        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            User user;

            user.id =
                sqlite3_column_int64(stmt, 0);

            const unsigned char *name =
                sqlite3_column_text(stmt, 1);

            const unsigned char *email =
                sqlite3_column_text(stmt, 2);

            user.name =
                name ? reinterpret_cast<const char *>(name) : "";

            user.email =
                email ? reinterpret_cast<const char *>(email) : "";

            users.push_back(user);
        }

        sqlite3_finalize(stmt);

        return users;
    }

    bool getById(long long id, User &user)
    {
        std::lock_guard<std::mutex> lock(mutex);

        const char *sql =
            "SELECT id, name, email "
            "FROM users "
            "WHERE id = ?;";

        sqlite3_stmt *stmt = nullptr;

        if (sqlite3_prepare_v2(
                db,
                sql,
                -1,
                &stmt,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(sqlite3_errmsg(db));
        }

        sqlite3_bind_int64(stmt, 1, id);

        bool found = false;

        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            found = true;

            user.id =
                sqlite3_column_int64(stmt, 0);

            const unsigned char *name =
                sqlite3_column_text(stmt, 1);

            const unsigned char *email =
                sqlite3_column_text(stmt, 2);

            user.name =
                name ? reinterpret_cast<const char *>(name) : "";

            user.email =
                email ? reinterpret_cast<const char *>(email) : "";
        }

        sqlite3_finalize(stmt);

        return found;
    }

    long long create(
        const std::string &name,
        const std::string &email)
    {
        std::lock_guard<std::mutex> lock(mutex);

        const char *sql =
            "INSERT INTO users(name, email) "
            "VALUES(?, ?);";

        sqlite3_stmt *stmt = nullptr;

        if (sqlite3_prepare_v2(
                db,
                sql,
                -1,
                &stmt,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(sqlite3_errmsg(db));
        }

        sqlite3_bind_text(
            stmt,
            1,
            name.c_str(),
            -1,
            SQLITE_TRANSIENT);

        sqlite3_bind_text(
            stmt,
            2,
            email.c_str(),
            -1,
            SQLITE_TRANSIENT);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::string error = sqlite3_errmsg(db);

            sqlite3_finalize(stmt);

            throw std::runtime_error(error);
        }

        sqlite3_finalize(stmt);

        return sqlite3_last_insert_rowid(db);
    }

    bool update(
        long long id,
        const std::string &name,
        const std::string &email)
    {
        std::lock_guard<std::mutex> lock(mutex);

        const char *sql =
            "UPDATE users "
            "SET name = ?, email = ? "
            "WHERE id = ?;";

        sqlite3_stmt *stmt = nullptr;

        if (sqlite3_prepare_v2(
                db,
                sql,
                -1,
                &stmt,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(sqlite3_errmsg(db));
        }

        sqlite3_bind_text(
            stmt,
            1,
            name.c_str(),
            -1,
            SQLITE_TRANSIENT);

        sqlite3_bind_text(
            stmt,
            2,
            email.c_str(),
            -1,
            SQLITE_TRANSIENT);

        sqlite3_bind_int64(
            stmt,
            3,
            id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::string error = sqlite3_errmsg(db);

            sqlite3_finalize(stmt);

            throw std::runtime_error(error);
        }

        int changes = sqlite3_changes(db);

        sqlite3_finalize(stmt);

        return changes > 0;
    }

    bool patch(
        long long id,
        const std::string *name,
        const std::string *email)
    {
        User existing;

        if (!getById(id, existing))
            return false;

        std::string finalName =
            name ? *name : existing.name;

        std::string finalEmail =
            email ? *email : existing.email;

        return update(
            id,
            finalName,
            finalEmail);
    }

    bool remove(long long id)
    {
        std::lock_guard<std::mutex> lock(mutex);

        const char *sql =
            "DELETE FROM users WHERE id = ?;";

        sqlite3_stmt *stmt = nullptr;

        if (sqlite3_prepare_v2(
                db,
                sql,
                -1,
                &stmt,
                nullptr) != SQLITE_OK)
        {
            throw std::runtime_error(sqlite3_errmsg(db));
        }

        sqlite3_bind_int64(stmt, 1, id);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            std::string error = sqlite3_errmsg(db);

            sqlite3_finalize(stmt);

            throw std::runtime_error(error);
        }

        int changes = sqlite3_changes(db);

        sqlite3_finalize(stmt);

        return changes > 0;
    }
};

// =========================
// Redis Cache
// =========================

class RedisCache
{
private:
    Poco::Redis::Client client;
    std::mutex mutex;

public:
    RedisCache()
        : client("127.0.0.1", 6379)
    {
    }

    bool ping()
    {
        std::lock_guard<std::mutex> lock(mutex);

        Poco::Redis::Array command;
        command << "PING";

        std::string result =
            client.execute<std::string>(command);

        return result == "PONG";
    }

    void set(
        const std::string &key,
        const std::string &value)
    {
        std::lock_guard<std::mutex> lock(mutex);

        Poco::Redis::Array command;

        command << "SET"
                << key
                << value;

        client.execute<std::string>(command);
    }

    bool get(
        const std::string &key,
        std::string &value)
    {
        std::lock_guard<std::mutex> lock(mutex);

        Poco::Redis::Array command;

        command << "GET"
                << key;

        // IMPORTANT:
        // BulkString can represent Redis NULL.
        Poco::Redis::BulkString result =
            client.execute<Poco::Redis::BulkString>(command);

        if (result.isNull())
        {
            return false;
        }

        value = result.value();

        return true;
    }

    void remove(const std::string &key)
    {
        std::lock_guard<std::mutex> lock(mutex);

        Poco::Redis::Array command;

        command << "DEL"
                << key;

        client.execute<Poco::Int64>(command);
    }

    void clearUser(long long id)
    {
        remove("user:" + std::to_string(id));
    }

    void cacheUser(const User &user)
    {
        Poco::JSON::Object::Ptr json =
            new Poco::JSON::Object;

        json->set("id", user.id);
        json->set("name", user.name);
        json->set("email", user.email);

        set(
            "user:" + std::to_string(user.id),
            toJsonString(json));
    }
};

// =========================
// REST Server Handler
// =========================

class UserHandler
    : public Poco::Net::HTTPRequestHandler
{
private:
    UserRepository &repository;
    RedisCache &redis;

public:
    UserHandler(
        UserRepository &repository,
        RedisCache &redis)
        : repository(repository),
          redis(redis)
    {
    }

    void handleRequest(
        Poco::Net::HTTPServerRequest &request,
        Poco::Net::HTTPServerResponse &response) override
    {
        try
        {
            const std::string method =
                request.getMethod();

            const std::string path =
                request.getURI();

            // =================================
            // OPTIONS
            // =================================

            if (method == "OPTIONS")
            {
                response.setStatus(
                    Poco::Net::HTTPResponse::HTTP_NO_CONTENT);

                response.set(
                    "Allow",
                    "GET, POST, PUT, PATCH, DELETE, OPTIONS");

                response.set(
                    "Access-Control-Allow-Origin",
                    "*");

                response.set(
                    "Access-Control-Allow-Methods",
                    "GET, POST, PUT, PATCH, DELETE, OPTIONS");

                response.set(
                    "Access-Control-Allow-Headers",
                    "Content-Type");

                response.send();

                return;
            }

            // =================================
            // GET /api/redis/health
            // =================================

            if (method == "GET" &&
                path == "/api/redis/health")
            {
                bool ok = redis.ping();

                Poco::JSON::Object::Ptr json =
                    new Poco::JSON::Object;

                json->set("redis", ok ? "UP" : "DOWN");

                sendJson(
                    response,
                    ok
                        ? Poco::Net::HTTPResponse::HTTP_OK
                        : Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
                    toJsonString(json));

                return;
            }

            // =================================
            // GET /api/users
            // =================================

            if (method == "GET" &&
                path == "/api/users")
            {
                std::vector<User> users =
                    repository.getAll();

                Poco::JSON::Array::Ptr array =
                    new Poco::JSON::Array;

                for (const User &user : users)
                {
                    Poco::JSON::Object::Ptr json =
                        new Poco::JSON::Object;

                    json->set("id", user.id);
                    json->set("name", user.name);
                    json->set("email", user.email);

                    array->add(json);
                }

                Poco::JSON::Object::Ptr result =
                    new Poco::JSON::Object;

                result->set("users", array);

                sendJson(
                    response,
                    Poco::Net::HTTPResponse::HTTP_OK,
                    toJsonString(result));

                return;
            }

            // =================================
            // GET /api/users/{id}
            // =================================

            if (method == "GET")
            {
                long long id;

                if (extractId(path, id))
                {
                    std::string cached;

                    // Try Redis first
                    if (redis.get(
                            "user:" + std::to_string(id),
                            cached))
                    {
                        std::cout
                            << "[CACHE HIT] user:"
                            << id
                            << std::endl;

                        sendJson(
                            response,
                            Poco::Net::HTTPResponse::HTTP_OK,
                            cached);

                        return;
                    }

                    std::cout
                        << "[CACHE MISS] user:"
                        << id
                        << std::endl;

                    User user;

                    if (!repository.getById(id, user))
                    {
                        sendJson(
                            response,
                            Poco::Net::HTTPResponse::HTTP_NOT_FOUND,
                            toJsonString(
                                errorJson("User not found")));

                        return;
                    }

                    Poco::JSON::Object::Ptr json =
                        new Poco::JSON::Object;

                    json->set("id", user.id);
                    json->set("name", user.name);
                    json->set("email", user.email);

                    std::string jsonString =
                        toJsonString(json);

                    // Store in Redis
                    redis.set(
                        "user:" + std::to_string(id),
                        jsonString);

                    sendJson(
                        response,
                        Poco::Net::HTTPResponse::HTTP_OK,
                        jsonString);

                    return;
                }
            }

            // =================================
            // POST /api/users
            // =================================

            if (method == "POST" &&
                path == "/api/users")
            {
                std::string body;

                std::istream &input =
                    request.stream();

                std::ostringstream stream;

                stream << input.rdbuf();

                body = stream.str();

                Poco::JSON::Parser parser;

                Poco::Dynamic::Var parsed =
                    parser.parse(body);

                Poco::JSON::Object::Ptr json =
                    parsed.extract<Poco::JSON::Object::Ptr>();

                if (!json->has("name") ||
                    !json->has("email"))
                {
                    sendJson(
                        response,
                        Poco::Net::HTTPResponse::HTTP_BAD_REQUEST,
                        toJsonString(
                            errorJson(
                                "name and email are required")));

                    return;
                }

                std::string name =
                    json->getValue<std::string>("name");

                std::string email =
                    json->getValue<std::string>("email");

                long long id =
                    repository.create(name, email);

                Poco::JSON::Object::Ptr result =
                    new Poco::JSON::Object;

                result->set("message", "User created");
                result->set("id", id);
                result->set("name", name);
                result->set("email", email);

                sendJson(
                    response,
                    Poco::Net::HTTPResponse::HTTP_CREATED,
                    toJsonString(result));

                return;
            }

            // =================================
            // PUT /api/users/{id}
            // =================================

            if (method == "PUT")
            {
                long long id;

                if (!extractId(path, id))
                {
                    sendJson(
                        response,
                        Poco::Net::HTTPResponse::HTTP_BAD_REQUEST,
                        toJsonString(
                            errorJson("Invalid user ID")));

                    return;
                }

                std::string body;

                std::istream &input =
                    request.stream();

                std::ostringstream stream;

                stream << input.rdbuf();

                body = stream.str();

                Poco::JSON::Parser parser;

                Poco::Dynamic::Var parsed =
                    parser.parse(body);

                Poco::JSON::Object::Ptr json =
                    parsed.extract<Poco::JSON::Object::Ptr>();

                if (!json->has("name") ||
                    !json->has("email"))
                {
                    sendJson(
                        response,
                        Poco::Net::HTTPResponse::HTTP_BAD_REQUEST,
                        toJsonString(
                            errorJson(
                                "PUT requires name and email")));

                    return;
                }

                std::string name =
                    json->getValue<std::string>("name");

                std::string email =
                    json->getValue<std::string>("email");

                if (!repository.update(
                        id,
                        name,
                        email))
                {
                    sendJson(
                        response,
                        Poco::Net::HTTPResponse::HTTP_NOT_FOUND,
                        toJsonString(
                            errorJson("User not found")));

                    return;
                }

                // Invalidate Redis cache
                redis.clearUser(id);

                Poco::JSON::Object::Ptr result =
                    new Poco::JSON::Object;

                result->set("message", "User updated");
                result->set("id", id);
                result->set("name", name);
                result->set("email", email);

                sendJson(
                    response,
                    Poco::Net::HTTPResponse::HTTP_OK,
                    toJsonString(result));

                return;
            }

            // =================================
            // PATCH /api/users/{id}
            // =================================

            if (method == "PATCH")
            {
                long long id;

                if (!extractId(path, id))
                {
                    sendJson(
                        response,
                        Poco::Net::HTTPResponse::HTTP_BAD_REQUEST,
                        toJsonString(
                            errorJson("Invalid user ID")));

                    return;
                }

                std::string body;

                std::istream &input =
                    request.stream();

                std::ostringstream stream;

                stream << input.rdbuf();

                body = stream.str();

                Poco::JSON::Parser parser;

                Poco::Dynamic::Var parsed =
                    parser.parse(body);

                Poco::JSON::Object::Ptr json =
                    parsed.extract<Poco::JSON::Object::Ptr>();

                std::string name;
                std::string email;

                const std::string *namePtr = nullptr;
                const std::string *emailPtr = nullptr;

                if (json->has("name"))
                {
                    name =
                        json->getValue<std::string>("name");

                    namePtr = &name;
                }

                if (json->has("email"))
                {
                    email =
                        json->getValue<std::string>("email");

                    emailPtr = &email;
                }

                if (!namePtr && !emailPtr)
                {
                    sendJson(
                        response,
                        Poco::Net::HTTPResponse::HTTP_BAD_REQUEST,
                        toJsonString(
                            errorJson(
                                "PATCH requires name or email")));

                    return;
                }

                if (!repository.patch(
                        id,
                        namePtr,
                        emailPtr))
                {
                    sendJson(
                        response,
                        Poco::Net::HTTPResponse::HTTP_NOT_FOUND,
                        toJsonString(
                            errorJson("User not found")));

                    return;
                }

                // Invalidate Redis
                redis.clearUser(id);

                User updated;

                repository.getById(
                    id,
                    updated);

                Poco::JSON::Object::Ptr result =
                    new Poco::JSON::Object;

                result->set(
                    "message",
                    "User patched");

                result->set(
                    "id",
                    updated.id);

                result->set(
                    "name",
                    updated.name);

                result->set(
                    "email",
                    updated.email);

                sendJson(
                    response,
                    Poco::Net::HTTPResponse::HTTP_OK,
                    toJsonString(result));

                return;
            }

            // =================================
            // DELETE /api/users/{id}
            // =================================

            if (method == "DELETE")
            {
                long long id;

                if (!extractId(path, id))
                {
                    sendJson(
                        response,
                        Poco::Net::HTTPResponse::HTTP_BAD_REQUEST,
                        toJsonString(
                            errorJson("Invalid user ID")));

                    return;
                }

                if (!repository.remove(id))
                {
                    sendJson(
                        response,
                        Poco::Net::HTTPResponse::HTTP_NOT_FOUND,
                        toJsonString(
                            errorJson("User not found")));

                    return;
                }

                // Remove Redis cache
                redis.clearUser(id);

                Poco::JSON::Object::Ptr result =
                    new Poco::JSON::Object;

                result->set(
                    "message",
                    "User deleted");

                result->set(
                    "id",
                    id);

                sendJson(
                    response,
                    Poco::Net::HTTPResponse::HTTP_OK,
                    toJsonString(result));

                return;
            }

            // =================================
            // Unknown endpoint
            // =================================

            sendJson(
                response,
                Poco::Net::HTTPResponse::HTTP_NOT_FOUND,
                toJsonString(
                    errorJson("Endpoint not found")));
        }
        catch (const Poco::Redis::RedisException &e)
        {
            sendJson(
                response,
                Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
                toJsonString(
                    errorJson(
                        std::string("Redis error: ") +
                        e.displayText())));
        }
        catch (const Poco::Exception &e)
        {
            sendJson(
                response,
                Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
                toJsonString(
                    errorJson(
                        e.displayText())));
        }
        catch (const std::exception &e)
        {
            sendJson(
                response,
                Poco::Net::HTTPResponse::HTTP_INTERNAL_SERVER_ERROR,
                toJsonString(
                    errorJson(
                        e.what())));
        }
    }
};

// =========================
// Handler Factory
// =========================

class UserHandlerFactory
    : public Poco::Net::HTTPRequestHandlerFactory
{
private:
    UserRepository &repository;
    RedisCache &redis;

public:
    UserHandlerFactory(
        UserRepository &repository,
        RedisCache &redis)
        : repository(repository),
          redis(redis)
    {
    }

    Poco::Net::HTTPRequestHandler *
    createRequestHandler(
        const Poco::Net::HTTPServerRequest &)
        override
    {
        return new UserHandler(
            repository,
            redis);
    }
};

// =========================
// MAIN
// =========================

int main()
{
    try
    {
        std::cout
            << "====================================\n"
            << " POCO + Redis + SQLite REST API\n"
            << "====================================\n";

        // SQLite
        UserRepository repository;

        std::cout
            << "[OK] SQLite database ready\n";

        // Redis
        RedisCache redis;

        try
        {
            if (redis.ping())
            {
                std::cout
                    << "[OK] Redis connected\n";
            }
        }
        catch (const std::exception &e)
        {
            std::cout
                << "[WARNING] Redis unavailable: "
                << e.what()
                << "\n";

            std::cout
                << "Start Redis using:\n"
                << "  sudo systemctl start redis\n";
        }

        // HTTP server
        Poco::Net::ServerSocket socket(8080);

        Poco::Net::HTTPServerParams *params =
            new Poco::Net::HTTPServerParams;

        params->setMaxQueued(100);
        params->setMaxThreads(16);

        Poco::Net::HTTPServer server(
            new UserHandlerFactory(
                repository,
                redis),
            socket,
            params);

        server.start();

        std::cout
            << "\nREST API started\n"
            << "http://localhost:8080\n\n"

            << "Endpoints:\n"
            << "GET     /api/users\n"
            << "GET     /api/users/{id}\n"
            << "POST    /api/users\n"
            << "PUT     /api/users/{id}\n"
            << "PATCH   /api/users/{id}\n"
            << "DELETE  /api/users/{id}\n"
            << "OPTIONS /api/users\n"
            << "GET     /api/redis/health\n\n"

            << "Press ENTER to stop...\n";

        std::cin.get();

        server.stop();

        std::cout
            << "Server stopped.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr
            << "FATAL ERROR: "
            << e.what()
            << "\n";

        return 1;
    }

    return 0;
}