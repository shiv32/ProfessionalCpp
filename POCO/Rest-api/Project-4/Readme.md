https & nginx

Here is the **smallest practical architecture**:

**Client → HTTPS/Nginx → HTTP/POCO C++ REST API**

For learning, keep the C++ server to **one `.cpp` file**. 
Nginx handles HTTPS/TLS and forwards requests to POCO over HTTP.

Run:

```bash
./server
```

Test directly:

```bash
curl http://localhost:8080/api/hello
```

Response:

```json
{"message":"Hello from C++ POCO API"}
```

---

## 3. Add Nginx HTTPS

The important point is:

**POCO does NOT need to know about HTTPS in this basic architecture.**

```text
                 HTTPS
Client ──────────────────────> Nginx :443
                                  |
                                  | HTTP
                                  v
                            POCO C++ :8080
```

Example Nginx configuration:
Note: Check nginx module for conf in this repo

server {
    listen 443 ssl;
    server_name localhost;

    ssl_certificate     /etc/nginx/ssl/server.crt;
    ssl_certificate_key /etc/nginx/ssl/server.key;

    location / {
        proxy_pass http://127.0.0.1:8080;

        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto https;
    }
}

Now the request flow becomes:

curl
  |
  | HTTPS :443
  v
+----------------+
|     Nginx      |
| TLS termination|
+----------------+
        |
        | HTTP :8080
        v
+----------------+
| C++ POCO       |
| REST API       |
+----------------+

Then:

```bash
curl -k https://localhost/api/hello
```

returns:

```json
{"message":"Hello from C++ POCO API"}
```

### What each component does

| Component       | Responsibility                       |
| --------------- | ------------------------------------ |
| **C++17**       | Application/business logic           |
| **POCO Net**    | HTTP server / REST endpoints         |
| **Nginx**       | Reverse proxy                        |
| **Nginx SSL**   | HTTPS/TLS termination                |
| **Certificate** | Proves server identity + enables TLS |
| **curl/client** | Calls the REST API                   |

This is a good **first POCO + Nginx + HTTPS project** before adding JWT, Redis, JSON parsing, logging, etc.
