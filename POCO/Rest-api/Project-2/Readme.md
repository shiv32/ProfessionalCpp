A good **POCO + Redis + REST API** beginner project is to expose Redis operations through HTTP endpoints.

Architecture:

Postman
   |
   | HTTP REST
   v
POCO HTTP Server
   |
   | GET / POST / DELETE
   v
Redis Client
   |
   v
Redis Server :6379

We'll build these endpoints:

| HTTP     | Endpoint                  | Redis operation |
| -------- | ------------------------- | --------------- |
| `GET`    | `/api/health`             | Check API       |
| `GET`    | `/api/redis/health`       | Check Redis     |
| `POST`   | `/api/redis/{key}`        | SET             |
| `GET`    | `/api/redis/{key}`        | GET             |
| `DELETE` | `/api/redis/{key}`        | DEL             |
| `GET`    | `/api/redis/exists/{key}` | EXISTS          |

## 1. Install Redis

On Manjaro:

```bash
sudo pacman -S redis
```

Start it:

```bash
sudo systemctl enable --now redis
```

Check:

redis-cli ping

Expected:

PONG

---

# 5. Test with Postman

### Check REST API

```text
GET http://localhost:8080/api/health
```

Response:

```json
{
    "status": "OK",
    "message": "POCO REST API is running"
}
```

---

### Check Redis

```text
GET http://localhost:8080/api/redis/health
```

Expected:

```json
{
    "status": "OK",
    "redis": "PONG"
}
```

This confirms:

```text
Postman
   ↓
POCO
   ↓
Redis
```

is working.

---

# 6. SET — POST

Postman:

```text
POST http://localhost:8080/api/redis/name
```

Body → **raw → JSON**

```json
{
    "value": "Shiv"
}
```

This executes internally:

```text
SET name Shiv
```

Response:

```json
{
    "message": "Value stored",
    "key": "name",
    "value": "Shiv",
    "redis": "OK"
}
```

---

# 7. GET

Postman:

```text
GET http://localhost:8080/api/redis/name
```

Internally:

```text
GET name
```

Response:

```json
{
    "key": "name",
    "value": "Shiv"
}
```

You can verify directly:

```bash
redis-cli GET name
```

Result:

```text
"Shiv"
```

---

# 8. EXISTS

Postman:

```text
GET http://localhost:8080/api/redis/exists/name
```

Response:

```json
{
    "key": "name",
    "exists": true
}
```

Internally:

```text
EXISTS name
```

---

# 9. DELETE

Postman:

```text
DELETE http://localhost:8080/api/redis/name
```

Internally:

```text
DEL name
```

Response:

```json
{
    "key": "name",
    "deleted": 1,
    "message": "Key deleted"
}
```

Then:

```text
GET http://localhost:8080/api/redis/name
```

will no longer have the value.

---

## Overall flow

```text
                    POSTMAN
                       |
                       | HTTP
                       v
              +------------------+
              |   POCO HTTP      |
              |     SERVER       |
              +------------------+
                       |
                       v
              Handler Factory
                       |
          +------------+------------+
          |            |            |
         POST         GET        DELETE
          |            |            |
          v            v            v
         SET          GET          DEL
          \            |            /
           \           |           /
            +----------+----------+
                       |
                       v
                +------------+
                |   REDIS    |
                |   :6379    |
                +------------+
```

This is a useful next step from your current POCO REST project because you're now separating the responsibilities:

```text
POCO
  ↓
HTTP / REST layer

Redis
  ↓
Fast key-value storage
```

For a more realistic C++ backend project, the next architecture would be:

```text
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
PostgreSQL
```

where **Redis is used for caching/session/rate-limit/fast temporary data**, 
while PostgreSQL or another database holds the durable application data.
