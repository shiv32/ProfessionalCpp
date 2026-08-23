Let's make one **small Drogon CRUD REST API** covering the basic HTTP endpoints:

GET     /hello
GET     /users
GET     /users/{id}
POST    /users
PUT     /users/{id}
DELETE  /users/{id}


It will keep users **in memory**, 
so you can learn REST first without a database.

Run:

./drogon_crud

---

# 4. Test every endpoint

### GET

curl http://127.0.0.1:8080/hello

Response:

{
    "message": "Hello from Drogon"
}

### GET all users

curl http://127.0.0.1:8080/users

Response:

[
    {
        "id": 1,
        "name": "Shiv"
    },
    {
        "id": 2,
        "name": "John"
    }
]

### GET one user

curl http://127.0.0.1:8080/users/1

### POST

curl -X POST \
     -H "Content-Type: application/json" \
     -d '{"name":"Alice"}' \
     http://127.0.0.1:8080/users

You'll get something like:

{
    "id": 3,
    "name": "Alice"
}

### PUT

Change Alice's name:

curl -X PUT \
     -H "Content-Type: application/json" \
     -d '{"name":"Bob"}' \
     http://127.0.0.1:8080/users/3

### DELETE

curl -X DELETE \
     http://127.0.0.1:8080/users/3

Expected HTTP status:

204 No Content

---

## What you're learning

This tiny project gives you the fundamental REST operations:

HTTP       Drogon              Purpose
────────────────────────────────────────
GET        {drogon::Get}       Read
POST       {drogon::Post}      Create
PUT        {drogon::Put}       Update
DELETE     {drogon::Delete}    Delete

And the complete flow is:

QML / Browser / curl
        │
        │ HTTP + JSON
        ▼
      Drogon
        │
        ▼
    REST Handler
        │
        ▼
    C++ Business Logic
        │
        ▼
    vector<User>

