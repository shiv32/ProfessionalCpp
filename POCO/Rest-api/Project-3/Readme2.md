Let's build a **complete basic C++17 backend project** using your installed **POCO 1.15.3**, Redis, SQLite and nginx:

Postman
   │
   ▼
 nginx :80
   │
   ▼
 POCO REST API :8080
   │
   ▼
 UserService
   │
   ├──────────► Redis :6379
   │             cache
   │
   └──────────► SQLite
                 persistent DB

We'll implement:

* `GET /api/users`
* `GET /api/users/{id}`
* `POST /api/users`
* `PUT /api/users/{id}`
* `PATCH /api/users/{id}`
* `DELETE /api/users/{id}`
* `OPTIONS /api/users`
* Redis health
* SQLite persistence
* Redis caching
* nginx reverse proxy

---

# 1. Install dependencies

On your Manjaro system:

sudo pacman -S poco redis sqlite nginx

Check POCO:

pacman -Q poco

You have:

poco 1.15.3-2

Check Redis:

redis-server --version

Check nginx:

nginx -v

Check SQLite:

sqlite3 --version


---

# 2. Start Redis

sudo systemctl enable --now redis

Test:

redis-cli ping

Expected:

PONG

---

# 3. Project structure

Create:

mkdir -p ~/Projects/ProfessionalCpp/POCO/Rest-api/Project-3
cd ~/Projects/ProfessionalCpp/POCO/Rest-api/Project-3


Structure:

Project-3/
├── CMakeLists.txt
├── main.cpp
├── User.h
├── UserRepository.h
├── UserRepository.cpp
├── RedisCache.h
├── RedisCache.cpp
├── UserService.h
├── UserService.cpp
└── app.db

---

Run:

./build/PocoRedisSqliteRestApi


You should see:

=====================================
 POCO + REDIS + SQLITE REST API
=====================================

POCO API:
http://localhost:8080

GET     /api/users
GET     /api/users/{id}
POST    /api/users
PUT     /api/users/{id}
PATCH   /api/users/{id}
DELETE  /api/users/{id}
OPTIONS /api/users
GET     /api/redis/health

Press ENTER to stop...
```

---

# 14. Test without nginx first

This is important.

Don't introduce nginx until POCO works.

## POST

Postman:

POST http://localhost:8080/api/users

Body → raw → JSON:

```json
{
    "name": "Shiv",
    "email": "shiv@example.com"
}
```

Expected:

```json
{
    "message": "User created",
    "id": 1,
    "name": "Shiv",
    "email": "shiv@example.com"
}
```

At this point:

```text
POCO
 ↓
UserService
 ↓
SQLite INSERT
 ↓
Redis SET
```

---

# 15. GET all

GET http://localhost:8080/api/users

Response:

```json
{
    "users": [
        {
            "id": 1,
            "name": "Shiv",
            "email": "shiv@example.com"
        }
    ],
    "count": 1
}
```

---

# 16. GET one user

```text
GET http://localhost:8080/api/users/1
```

First request:

POCO
 ↓
UserService
 ↓
Redis GET user:1
 ↓
CACHE MISS
 ↓
SQLite
 ↓
Redis SET user:1
 ↓
Response

Second request:

POCO
 ↓
UserService
 ↓
Redis GET user:1
 ↓
CACHE HIT
 ↓
Response

That's the important Redis caching concept.

---

# 17. Check Redis

Open another terminal:

redis-cli

Run:

KEYS *


You should see something like:

```text
1) "user:1"
```

Then:

```redis
GET user:1
```

You should see:

```text
Shiv|shiv@example.com
```

---

# 18. Check SQLite

You have:

app.db

Run:

sqlite3 app.db

Then:

```sql
.tables
```

Expected:

```text
users
```

Then:

```sql
SELECT * FROM users;
```

Example:

```text
1|Shiv|shiv@example.com
```

Exit:

```sql
.quit
```

---

# 19. PUT

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

Flow:

POCO
 ↓
UserService
 ↓
SQLite UPDATE
 ↓
Redis SET

---

# 20. PATCH

PATCH is useful when you want to modify only one field.

```text
PATCH http://localhost:8080/api/users/1
```

Body:

```json
{
    "name": "Shiv Kumar"
}
```

The email remains unchanged.

Another example:

```json
{
    "email": "new@example.com"
}
```

Flow:

PATCH
  ↓
POCO
  ↓
UserService
  ↓
SQLite
  ↓
Redis update

---

# 21. DELETE

```text
DELETE http://localhost:8080/api/users/1
```

Flow:

```text
POCO
 ↓
UserService
 ↓
SQLite DELETE
 ↓
Redis DEL
```

Response:

```json
{
    "message": "User deleted",
    "id": 1
}
```

---

# 22. OPTIONS

In Postman:

```text
OPTIONS http://localhost:8080/api/users
```

Response headers include:

```text
Allow: GET, POST, PUT, PATCH, DELETE, OPTIONS
```

and body:

```text
OPTIONS OK
Allowed methods: GET, POST, PUT, PATCH, DELETE, OPTIONS
```

---

# 23. Redis health

```text
GET http://localhost:8080/api/redis/health
```

Expected:

```json
{
    "redis": "UP"
}
```

---

# 24. Now configure nginx

Once the direct POCO API works, configure nginx.

Create:
Note : check nginx section in this repo for setup

sudo nano /etc/nginx/nginx.conf

Put:

server {
    listen 80;

    server_name localhost;

    location /api/ {
        proxy_pass http://127.0.0.1:8080;

        proxy_http_version 1.1;

        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
}

Test nginx configuration:

sudo nginx -t

Expected:

syntax is ok
test is successful

Start nginx:

sudo systemctl enable --now nginx

Restart after configuration changes:

sudo systemctl restart nginx

---

# 25. Test through nginx

Now don't use:

localhost:8080

Use:

localhost

For example:

GET http://localhost/api/users

POST:

POST http://localhost/api/users

GET:

GET http://localhost/api/users/1

PATCH:

PATCH http://localhost/api/users/1

DELETE:

DELETE http://localhost/api/users/1

Now the complete request is:

Postman
   │
   │ HTTP :80
   ▼
 nginx
   │
   │ proxy_pass :8080
   ▼
 POCO HTTPServer
   │
   ▼
 Handler
   │
   ▼
 UserService
   │
   ├──────────────┐
   ▼              ▼
 Redis          SQLite
 :6379          app.db

---

# 26. What each layer does

This is the important part for learning backend architecture.

### nginx

Internet/client
      ↓
    nginx

Responsible for:

* Reverse proxy
* TLS/HTTPS
* Connection handling
* Load balancing
* Rate limiting
* Static files

---

### POCO

nginx
  ↓
POCO

Responsible for:

* HTTP server
* HTTP routing
* HTTP request
* HTTP response
* JSON
* REST endpoint handling

Example:

GET /api/users/1

---

### Handler

HTTP request
     ↓
GetUserHandler

Responsible for translating HTTP into an application call:

service.getUser(id, user);

It shouldn't contain database logic.

---

### Service Layer

GetUserHandler
      ↓
UserService

This is your **business logic**.

For example:

if (redis has user)
    return user;

user = sqlite.getUser();

redis.set(user);

return user;

---

### Redis

UserService
     ↓
Redis

Used for:

* Cache
* Session
* Fast lookup
* Temporary data
* Counters
* Rate limiting

Redis is **not your permanent database in this architecture**.

---

### SQLite

UserService
     ↓
SQLite

Used for persistent data:

app.db


If Redis is deleted/restarted:

Redis empty
    ↓
SQLite
    ↓
reload Redis

The user data remains.

---

# 27. Final architecture

You now have a small but realistic backend:

                         ┌─────────────┐
                         │   Postman   │
                         └──────┬──────┘
                                │
                                │ HTTP
                                ▼
                         ┌─────────────┐
                         │    nginx    │
                         │     :80     │
                         └──────┬──────┘
                                │
                         proxy_pass
                                │
                                ▼
                    ┌──────────────────────┐
                    │      POCO REST       │
                    │        :8080         │
                    └──────────┬───────────┘
                               │
                         HTTP Handler
                               │
                               ▼
                    ┌──────────────────────┐
                    │     UserService      │
                    │    Business Logic    │
                    └───────┬───────┬──────┘
                            │       │
                     Cache  │       │ Persistence
                            ▼       ▼
                      ┌────────┐ ┌─────────┐
                      │ Redis  │ │ SQLite  │
                      │ :6379  │ │ app.db  │
                      └────────┘ └─────────┘

This is a good next step from your current POCO + Redis project because it introduces the **Handler → Service → Repository/Cache** separation instead of putting Redis/SQLite code directly inside `handleRequest()`.
