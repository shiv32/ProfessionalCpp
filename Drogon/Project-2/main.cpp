#include <drogon/drogon.h>
#include <algorithm>
#include <string>
#include <vector>

struct User
{
    int id;
    std::string name;
};

std::vector<User> users = {
    {1, "Shiv"},
    {2, "John"}};

int nextId = 3;

int main()
{
    // ============================================================
    // GET /hello
    // ============================================================

    drogon::app().registerHandler(
        "/hello",
        [](const drogon::HttpRequestPtr &,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback)
        {
            Json::Value json;
            json["message"] = "Hello from Drogon";

            auto response =
                drogon::HttpResponse::newHttpJsonResponse(json);

            callback(response);
        },
        {drogon::Get});

    // ============================================================
    // GET /users
    // ============================================================

    drogon::app().registerHandler(
        "/users",
        [](const drogon::HttpRequestPtr &,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback)
        {
            Json::Value result(Json::arrayValue);

            for (const auto &user : users)
            {
                Json::Value json;

                json["id"] = user.id;
                json["name"] = user.name;

                result.append(json);
            }

            auto response =
                drogon::HttpResponse::newHttpJsonResponse(result);

            callback(response);
        },
        {drogon::Get});

    // ============================================================
    // GET /users/{id}
    // ============================================================

    drogon::app().registerHandler(
        "/users/{id}",
        [](const drogon::HttpRequestPtr &,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback,
           int id)
        {
            auto it = std::find_if(
                users.begin(),
                users.end(),
                [id](const User &user)
                {
                    return user.id == id;
                });

            if (it == users.end())
            {
                auto response =
                    drogon::HttpResponse::newHttpResponse(
                        drogon::k404NotFound,
                        drogon::CT_TEXT_PLAIN);

                response->setBody("User not found");

                callback(response);
                return;
            }

            Json::Value json;

            json["id"] = it->id;
            json["name"] = it->name;

            auto response =
                drogon::HttpResponse::newHttpJsonResponse(json);

            callback(response);
        },
        {drogon::Get});

    // ============================================================
    // POST /users
    // ============================================================

    drogon::app().registerHandler(
        "/users",
        [](const drogon::HttpRequestPtr &request,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback)
        {
            auto json = request->getJsonObject();

            if (!json || !json->isMember("name"))
            {
                auto response =
                    drogon::HttpResponse::newHttpResponse(
                        drogon::k400BadRequest,
                        drogon::CT_TEXT_PLAIN);

                response->setBody(
                    "JSON must contain 'name'");

                callback(response);
                return;
            }

            User user;

            user.id = nextId++;
            user.name = (*json)["name"].asString();

            users.push_back(user);

            Json::Value result;

            result["id"] = user.id;
            result["name"] = user.name;

            auto response =
                drogon::HttpResponse::newHttpJsonResponse(result);

            response->setStatusCode(drogon::k201Created);

            callback(response);
        },
        {drogon::Post});

    // ============================================================
    // PUT /users/{id}
    // ============================================================

    drogon::app().registerHandler(
        "/users/{id}",
        [](const drogon::HttpRequestPtr &request,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback,
           int id)
        {
            auto json = request->getJsonObject();

            if (!json || !json->isMember("name"))
            {
                auto response =
                    drogon::HttpResponse::newHttpResponse(
                        drogon::k400BadRequest,
                        drogon::CT_TEXT_PLAIN);

                response->setBody(
                    "JSON must contain 'name'");

                callback(response);
                return;
            }

            auto it = std::find_if(
                users.begin(),
                users.end(),
                [id](const User &user)
                {
                    return user.id == id;
                });

            if (it == users.end())
            {
                auto response =
                    drogon::HttpResponse::newHttpResponse(
                        drogon::k404NotFound,
                        drogon::CT_TEXT_PLAIN);

                response->setBody("User not found");

                callback(response);
                return;
            }

            it->name = (*json)["name"].asString();

            Json::Value result;

            result["id"] = it->id;
            result["name"] = it->name;

            auto response =
                drogon::HttpResponse::newHttpJsonResponse(result);

            callback(response);
        },
        {drogon::Put});

    // ============================================================
    // DELETE /users/{id}
    // ============================================================

    drogon::app().registerHandler(
        "/users/{id}",
        [](const drogon::HttpRequestPtr &,
           std::function<void(const drogon::HttpResponsePtr &)> &&callback,
           int id)
        {
            auto it = std::find_if(
                users.begin(),
                users.end(),
                [id](const User &user)
                {
                    return user.id == id;
                });

            if (it == users.end())
            {
                auto response =
                    drogon::HttpResponse::newHttpResponse(
                        drogon::k404NotFound,
                        drogon::CT_TEXT_PLAIN);

                response->setBody("User not found");

                callback(response);
                return;
            }

            users.erase(it);

            auto response =
                drogon::HttpResponse::newHttpResponse(
                    drogon::k204NoContent,
                    drogon::CT_TEXT_PLAIN);

            callback(response);
        },
        {drogon::Delete});

    // ============================================================
    // Start server
    // ============================================================

    drogon::app()
        .addListener("127.0.0.1", 8080)
        .run();

    return 0;
}