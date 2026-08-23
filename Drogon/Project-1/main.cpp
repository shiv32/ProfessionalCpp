#include <drogon/drogon.h>

int main()
{
    drogon::app().registerHandler(
        "/hello",
        [](const drogon::HttpRequestPtr &, std::function<void(const drogon::HttpResponsePtr &)> &&callback)
        {
            auto response = drogon::HttpResponse::newHttpResponse();

            response->setStatusCode(drogon::k200OK);
            response->setContentTypeCode(drogon::CT_TEXT_PLAIN);
            response->setBody("Hello from C++ Drogon!");

            callback(response);
        },
        {drogon::Get});

    drogon::app()
        .addListener("127.0.0.1", 8080)
        .run();

    return 0;
}