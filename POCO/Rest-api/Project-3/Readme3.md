Run:

./build/PocoRedisRestApi

You should see something similar to:

====================================
 POCO + Redis + SQLite REST API
====================================
[OK] SQLite database ready
[OK] Redis connected

REST API started
http://localhost:8080

Endpoints:
GET     /api/users
GET     /api/users/{id}
POST    /api/users
PUT     /api/users/{id}
PATCH   /api/users/{id}
DELETE  /api/users/{id}
OPTIONS /api/users
GET     /api/redis/health

Press ENTER to stop...

---

# 6. Test with Postman

### POST

```text
POST http://localhost:8080/api/users
```

Body → raw → JSON:

```json
{
    "name": "Shiv",
    "email": "shiv@example.com"
}
```

Response:

```json
{
    "message": "User created",
    "id": 1,
    "name": "Shiv",
    "email": "shiv@example.com"
}
```

---

### GET all

```text
GET http://localhost:8080/api/users
```

Response:

```json
{
    "users": [
        {
            "id": 1,
            "name": "Shiv",
            "email": "shiv@example.com"
        }
    ]
}
```

---

### GET one

```text
GET http://localhost:8080/api/users/1
```

First request:

```text
[CACHE MISS] user:1
```

The data comes from:

```text
SQLite
```

and is then placed into:

```text
Redis
```

Second request:

```text
[CACHE HIT] user:1
```

Now it comes from:

```text
Redis
```

This is the important part of the architecture.

---

### PUT

```text
PUT http://localhost:8080/api/users/1
```

Body:

```json
{
    "name": "Shiv Kumar",
    "email": "shivkumar@example.com"
}
```

The code updates SQLite and invalidates:

```text
Redis user:1
```

---

### PATCH

For example, change only the name:

```text
PATCH http://localhost:8080/api/users/1
```

Body:

```json
{
    "name": "Shiv Kumar"
}
```

Or only email:

```json
{
    "email": "new@example.com"
}
```

This is the difference from PUT:

```text
PUT
 └── normally replace/update complete resource

PATCH
 └── update only supplied fields
```

---

### DELETE

```text
DELETE http://localhost:8080/api/users/1
```

It deletes from:

```text
SQLite
```

and:

```text
Redis
```

---

### OPTIONS

In Postman:

```text
OPTIONS http://localhost:8080/api/users
```

You won't get a JSON body because this implementation intentionally returns:

```text
204 No Content
```

But the response headers contain:

```text
Allow: GET, POST, PUT, PATCH, DELETE, OPTIONS
```

This is the correct way to demonstrate an OPTIONS endpoint.

---

### Redis health

```text
GET http://localhost:8080/api/redis/health
```

Response:

```json
{
    "redis": "UP"
}
```

---

# 7. Your architecture

Without nginx:

Postman
   |
   v
POCO HTTP Server :8080
   |
   v
REST Handler
   |
   v
Repository
   |
   +------------+
   |            |
   v            v
SQLite        Redis
Database      Cache

With nginx:

                 ┌─────────────┐
Postman ────────>│    nginx    │
                 │    :80      │
                 └──────┬──────┘
                        |
                        v
                 ┌─────────────┐
                 │    POCO     │
                 │ REST API    │
                 │    :8080    │
                 └──────┬──────┘
                        |
                        v
                 ┌─────────────┐
                 │   Service   │
                 │    Logic    │
                 └──────┬──────┘
                        |
                ┌───────┴────────┐
                v                v
          ┌──────────┐      ┌──────────┐
          │  Redis   │      │  SQLite  │
          │  Cache   │      │   DB     │
          └──────────┘      └──────────┘


For your **C++ backend/infrastructure learning**, this is a good small project because you can later split this single file into:

main.cpp
    |
    +-- HTTP Layer
    |
    +-- UserController
    |
    +-- UserService
    |
    +-- UserRepository
    |
    +-- RedisCache
    |
    +-- SQLite

That would be the natural next step toward a more industry-style **POCO + nginx + Redis + SQLite C++ REST backend**.
