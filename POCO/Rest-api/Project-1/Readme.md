For learning **C++17 + POCO REST API**, a good basic example is to cover the common HTTP endpoint types:

* `GET /api/users`
* `GET /api/users/{id}`
* `POST /api/users`
* `PUT /api/users/{id}`
* `PATCH /api/users/{id}`
* `DELETE /api/users/{id}`
* `GET /api/health`
* `POST /api/login` — example of a POST with JSON
* `OPTIONS /api/users` — CORS/preflight example

Below is a small **POCO Net + JSON + C++17** server structure.

### 1. Project structure

PocoRestApi/
├── CMakeLists.txt
└── main.cpp


### 5. Test all endpoints

**GET**

curl http://localhost:8080/api/health

curl http://localhost:8080/api/users


curl http://localhost:8080/api/users/1

**POST**

curl -X POST http://localhost:8080/api/users \
     -H "Content-Type: application/json" \
     -d '{"name":"Amit","email":"amit@example.com"}'

**PUT**

curl -X PUT http://localhost:8080/api/users/1 \
     -H "Content-Type: application/json" \
     -d '{"name":"Shiv Kumar","email":"shivk@example.com"}'

**PATCH**

curl -X PATCH http://localhost:8080/api/users/1 \
     -H "Content-Type: application/json" \
     -d '{"name":"New Name"}'

**DELETE**

curl -X DELETE http://localhost:8080/api/users/1

**OPTIONS**

curl -X OPTIONS -i http://localhost:8080/api/users

### Endpoint map

| HTTP    | Endpoint          | Purpose             |
| ------- | ----------------- | ------------------- |
| GET     | `/api/health`     | Health check        |
| GET     | `/api/users`      | Get all users       |
| GET     | `/api/users/{id}` | Get one user        |
| POST    | `/api/users`      | Create user         |
| PUT     | `/api/users/{id}` | Replace/update user |
| PATCH   | `/api/users/{id}` | Partial update      |
| DELETE  | `/api/users/{id}` | Delete user         |
| OPTIONS | `/api/users`      | CORS/preflight      |

This is a good **POCO REST API fundamentals project**. 
The next step toward an industry-style C++ REST stack would be:

Client
   ↓
nginx
   ↓
POCO REST API
   ↓
Service Layer
   ↓
Repository / DAO
   ↓
SQLite / PostgreSQL / Redis

And importantly, POCO's `HTTPServer` is handling the HTTP layer while your C++17 code handles the endpoint/business logic.
