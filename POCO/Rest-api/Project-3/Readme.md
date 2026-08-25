If you want to learn a realistic C++ backend architecture, a good basic flow is:

Postman
   ↓
nginx
   ↓
POCO REST API
   ↓
Service Layer
   ↓
Redis
   ↓
SQLite

But one correction: **Redis and SQLite are normally not strictly sequential**. 
The service layer usually decides when to use each:

                  ┌──→ Redis
                  │
Postman → nginx → POCO → Service Layer
                  │
                  └──→ SQLite

For a basic project, we can make Redis a cache and SQLite the persistent database.

## Project structure

Project-3/
├── CMakeLists.txt
├── main.cpp
├── UserService.h
├── UserService.cpp
├── RedisCache.h
├── RedisCache.cpp
├── UserRepository.h
├── UserRepository.cpp
└── app.db

Flow:

GET /api/users/1
       ↓
POCO Handler
       ↓
UserService
       ↓
Redis GET user:1
       ↓
   found?
    /   \
  yes    no
   ↓      ↓
 return  SQLite
          ↓
       save to Redis
          ↓
        return

For SQLite, POCO has `Poco::Data::SQLite`, so you can keep the entire implementation in C++.

### Basic REST endpoints

| HTTP    | Endpoint       | Purpose              |
| ------- | -------------- | -------------------- |
| GET     | `/api/users`   | Get all users        |
| GET     | `/api/users/1` | Get user             |
| POST    | `/api/users`   | Create user          |
| PUT     | `/api/users/1` | Replace user         |
| PATCH   | `/api/users/1` | Update user          |
| DELETE  | `/api/users/1` | Delete user          |
| OPTIONS | `/api/users`   | Show allowed methods |

### Example POST

POST http://localhost:8080/api/users
Content-Type: application/json

{
    "name": "Shiv",
    "email": "shiv@example.com"
}

The service does:

POCO
 ↓
UserService::createUser()
 ↓
SQLite INSERT
 ↓
Redis SET
 ↓
JSON response

### Example GET

GET http://localhost:8080/api/users/1

First:

Redis:
GET user:1

If found:

Redis → UserService → POCO → Postman

If not found:

SQLite → UserService
          ↓
       Redis SET
          ↓
        POCO
          ↓
       Postman

### nginx

nginx would listen on, for example:

localhost:80

and forward to POCO:

localhost:8080

Conceptually:

Postman
   |
   | HTTP :80
   ↓
 nginx
   |
   | proxy_pass
   ↓
 POCO :8080
   |
   ↓
 UserService
   |
   ├── Redis :6379
   |
   └── SQLite app.db

For your learning path, I'd recommend building it in **three stages**:

**Stage 1**

Postman
 ↓
POCO
 ↓
SQLite

**Stage 2**

Postman
 ↓
POCO
 ↓
Service Layer
 ↓
Redis + SQLite

**Stage 3**

Postman
 ↓
nginx
 ↓
POCO
 ↓
Service Layer
 ↓
Redis + SQLite

That gives you a very good **C++ backend/infrastructure project** without introducing unnecessary complexity initially.
