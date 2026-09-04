Basic poco https rest api

Yes. Here is a **minimal POCO HTTPS REST API** in a single `main.cpp`.

It starts an HTTPS server on port `8443` and responds to:

`GET https://localhost:8443/api/hello`

### Certificate files

You need:

PocoHttpsApi/
├── CMakeLists.txt
├── main.cpp
├── server.crt
└── server.key

For local testing, you can create a self-signed certificate:

openssl req -x509 -newkey rsa:2048 \
    -keyout server.key \
    -out server.crt \
    -days 365 \
    -nodes \
    -subj "/CN=localhost"

Then compile/link against POCO Net and SSL libraries, for example:

g++ -std=c++17 main.cpp \
    -lPocoNet -lPocoNetSSL -lPocoFoundation \
    -lssl -lcrypto \
    -o server

Run:

```bash
./server
```

Test:

```bash
curl -k https://localhost:8443/api/hello
```

Response:

```json
{"message":"Hello from POCO HTTPS API"}
```

### Architecture

This is the **direct HTTPS POCO API**:

Client
   │
   │ HTTPS :8443
   ▼
POCO HTTPServer
   │
   └── /api/hello

Whereas your earlier Nginx architecture would be:

Client
   │
   │ HTTPS :443
   ▼
Nginx
   │
   │ HTTP :8080
   ▼
POCO HTTPServer

One important point: **POCO's HTTPS server is still an HTTP REST server**. 
HTTPS is simply HTTP transported over TLS.
