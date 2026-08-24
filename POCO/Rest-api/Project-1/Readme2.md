The confusion is mainly because **PATCH and OPTIONS have very different purposes**.

## 1. PATCH — partial update

Think of the difference like this:

### PUT

PUT usually means:

> "Here is the complete new version of this resource."

Existing user:

```json
{
    "id": 1,
    "name": "Shiv",
    "email": "shiv@example.com"
}
```

PUT request:

```http
PUT /api/users/1
```

```json
{
    "name": "Shiv Kumar",
    "email": "shivkumar@example.com"
}
```

You are replacing/updating the complete user representation.

---

### PATCH

PATCH means:

> "Change only this particular field."

For example, only change the email:

```http
PATCH /api/users/1
```

```json
{
    "email": "newemail@example.com"
}
```

The server should produce:

```json
{
    "id": 1,
    "name": "Shiv",
    "email": "newemail@example.com"
}
```

The `name` was **not sent**, so it remains unchanged.

### Simple comparison

```text
PUT
--------------------------------
Replace/update the resource

{
    name: "Shiv Kumar",
    email: "new@example.com"
}


PATCH
--------------------------------
Change selected fields

{
    email: "new@example.com"
}
```

### Better PATCH implementation

The previous example only demonstrated reaching the PATCH endpoint. 
A useful implementation would actually update selected fields.

Now this works:

```bash
curl -X PATCH http://localhost:8080/api/users/1 \
     -H "Content-Type: application/json" \
     -d '{"email":"new@example.com"}'
```

Only `email` changes.

---

# 2. OPTIONS — not an update operation

OPTIONS is completely different.

It basically asks the server:

> **"What HTTP operations/options are available for this endpoint?"**

For example:

```http
OPTIONS /api/users
```

The server can respond:

```http
HTTP/1.1 200 OK

Allow: GET, POST, PUT, PATCH, DELETE, OPTIONS
```

So:

```text
GET       → give me data
POST      → create data
PUT       → replace/update data
PATCH     → partially update data
DELETE    → delete data
OPTIONS   → tell me what operations are supported
```

---

## Why do browsers use OPTIONS?

This is where OPTIONS becomes particularly important for REST APIs.

Suppose your frontend is running here:

```text
http://localhost:3000
```

and your API is here:

```text
http://localhost:8080
```

The browser sees different origins.

Your JavaScript might send:

```javascript
fetch("http://localhost:8080/api/users", {
    method: "DELETE"
});
```

Before sending the actual request, the browser may send a **CORS preflight**:

```http
OPTIONS /api/users
Origin: http://localhost:3000
Access-Control-Request-Method: DELETE
```

The server responds something like:

```http
HTTP/1.1 200 OK
Access-Control-Allow-Origin: http://localhost:3000
Access-Control-Allow-Methods: GET, POST, PUT, PATCH, DELETE, OPTIONS
Access-Control-Allow-Headers: Content-Type, Authorization
```

The browser then knows:

> "Okay, this server allows my DELETE request."

Then it sends:

```http
DELETE /api/users/1
```

So OPTIONS is commonly seen in **CORS**, especially when a browser frontend talks to a REST API.

---

## One important correction to the previous code

For OPTIONS, this:

```cpp
response.set("Access-Control-Allow-Origin", "*");
```

sets an **HTTP response header**.

Whereas:

```cpp
response.setStatus(HTTPResponse::HTTP_OK);
```

sets the HTTP status.

So the conceptual flow is:

```text
Browser
   |
   | OPTIONS /api/users
   |------------------------>
   |
   | 200 OK
   | Allow: GET,POST,...
   | Access-Control-Allow-...
   |<------------------------
   |
   | DELETE /api/users/1
   |------------------------>
   |
```

### In short

**PATCH**

```text
PATCH /users/1
{"email":"new@example.com"}

        ↓

Change ONLY email
```

**OPTIONS**

```text
OPTIONS /users

        ↓

"What methods/headers are allowed?"
```

For your **C++ REST API learning**, I would prioritize them in this order:

```text
GET
 ↓
POST
 ↓
PUT
 ↓
DELETE
 ↓
PATCH
 ↓
OPTIONS / CORS
```

GET/POST/PUT/DELETE/PATCH are your core REST API operations. 
**OPTIONS is more of an HTTP/CORS mechanism than a business API operation.**
