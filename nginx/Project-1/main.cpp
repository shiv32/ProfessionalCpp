#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/URI.h>
#include <iostream>
#include <sstream>
#include <map>

using namespace Poco::Net;
using namespace Poco::JSON;

// -------- In-Memory DB (simulate Redis) --------
class OrderStore
{
    std::map<int, std::string> db;
    int idCounter = 1;

public:
    static OrderStore &instance()
    {
        static OrderStore inst;
        return inst;
    }

    int create(const std::string &data)
    {
        int id = idCounter++;
        db[id] = data;
        return id;
    }

    std::string get(int id)
    {
        if (db.count(id))
            return db[id];
        return "";
    }

    std::map<int, std::string> &getAll()
    {
        return db;
    }

    bool update(int id, const std::string &data)
    {
        if (!db.count(id))
            return false;
        db[id] = data;
        return true;
    }

    bool remove(int id)
    {
        return db.erase(id) > 0;
    }
};

// -------- Handler --------
class ApiHandler : public HTTPRequestHandler
{
public:
    void handleRequest(HTTPServerRequest &req, HTTPServerResponse &res) override
    {
        res.setContentType("application/json");
        std::ostream &out = res.send();

        Poco::URI uri(req.getURI());
        std::string path = uri.getPath();

        int id = 0;
        for (auto &p : uri.getQueryParameters())
        {
            if (p.first == "id")
                id = std::stoi(p.second);
        }

        // -------- CREATE --------
        if (path == "/api/order" && req.getMethod() == "POST")
        {
            std::stringstream buffer;
            buffer << req.stream().rdbuf();

            int newId = OrderStore::instance().create(buffer.str());

            out << "{\"message\":\"created\",\"id\":" << newId << "}";
            return;
        }

        // -------- READ ONE --------
        if (path == "/api/order" && req.getMethod() == "GET")
        {
            std::string data = OrderStore::instance().get(id);

            if (data.empty())
            {
                res.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                out << "{\"error\":\"not found\"}";
            }
            else
            {
                out << "{\"id\":" << id << ",\"data\":" << data << "}";
            }
            return;
        }

        // -------- READ ALL --------
        if (path == "/api/orders" && req.getMethod() == "GET")
        {
            Array arr;

            for (auto &[key, val] : OrderStore::instance().getAll())
            {
                Object obj;
                obj.set("id", key);

                Parser parser;
                obj.set("data", parser.parse(val));

                arr.add(obj);
            }

            arr.stringify(out);
            return;
        }

        // -------- UPDATE --------
        if (path == "/api/order" && req.getMethod() == "PUT")
        {
            std::stringstream buffer;
            buffer << req.stream().rdbuf();

            bool ok = OrderStore::instance().update(id, buffer.str());

            if (!ok)
            {
                res.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                out << "{\"error\":\"not found\"}";
            }
            else
            {
                out << "{\"message\":\"updated\"}";
            }
            return;
        }

        // -------- DELETE --------
        if (path == "/api/order" && req.getMethod() == "DELETE")
        {
            bool ok = OrderStore::instance().remove(id);

            if (!ok)
            {
                res.setStatus(HTTPResponse::HTTP_NOT_FOUND);
                out << "{\"error\":\"not found\"}";
            }
            else
            {
                out << "{\"message\":\"deleted\"}";
            }
            return;
        }

        // -------- DEFAULT --------
        res.setStatus(HTTPResponse::HTTP_NOT_FOUND);
        out << "{\"error\":\"invalid endpoint\"}";
    }
};

// -------- Factory --------
class HandlerFactory : public HTTPRequestHandlerFactory
{
public:
    HTTPRequestHandler *createRequestHandler(const HTTPServerRequest &) override
    {
        return new ApiHandler();
    }
};

// -------- Main --------
int main()
{
    ServerSocket socket(8080);
    HTTPServer server(new HandlerFactory(), socket, new HTTPServerParams);

    std::cout << "CRUD API running at http://localhost:8080\n";
    server.start();

    std::cin.get();
    server.stop();
    
    return 0;
}