Poco JWT

Let's use **POCO's own `Poco::JWT`**

POCO's JWT module provides `Token` and `Signer`; `Signer` can both **sign and verify** JWTs, including HS256. ([docs.pocoproject.org][1])

We'll keep it deliberately small:

```text
Client
   │
   │ HTTPS
   ▼
Nginx :443
   │
   │ HTTP
   ▼
POCO C++ :8080
   │
   └── Poco::JWT
         ├── Signer → create JWT
         └── Signer → verify JWT
```

POCO's documentation shows the same basic `Token → Signer → sign()` and `Signer → verify()` model.

---

POCO's JWT package contains `Token`, `Signer`, `Serializer`, and JWT exception classes.
---

# 4. Test `/login`

Because your Nginx HTTPS setup is already working:

```bash
curl -k -X POST https://localhost/api/login
```

You should receive:

```json
{
    "token": "eyJhbGciOiJIUzI1NiIs..."
}
```

Copy the token.

---

# 5. Call protected API WITHOUT JWT

```bash
curl -k https://localhost/api/hello
```

Response:

```json
{
    "error": "Invalid JWT"
}
```

---

# 6. Call protected API WITH JWT

Replace `YOUR_TOKEN`:

```bash
curl -k \
  -H "Authorization: Bearer YOUR_TOKEN" \
  https://localhost/api/hello
```

Response:

```json
{
    "message": "Hello Shiv",
    "role": "user",
    "jwt": "valid"
}
```

---

# 7. Nginx

You **don't need to change your existing Nginx configuration**.

```nginx
server {
    listen 443 ssl;
    server_name localhost;

    ssl_certificate /etc/nginx/ssl/server.crt;
    ssl_certificate_key /etc/nginx/ssl/server.key;

    location / {
        proxy_pass http://127.0.0.1:8080;

        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto https;
    }
}
```

So your final system is:

                 HTTPS
                  :443
                   │
                   ▼
             ┌───────────┐
             │   Nginx   │
             │ TLS       │
             │ terminate │
             └─────┬─────┘
                   │
                   │ HTTP
                   │ :8080
                   ▼
          ┌─────────────────┐
          │   POCO C++      │
          │   REST API      │
          └────────┬────────┘
                   │
                   ▼
             Poco::JWT
            ┌────────────┐
            │   Signer   │
            ├────────────┤
            │  HS256     │
            ├────────────┤
            │ sign()     │
            │ verify()   │
            └────────────┘
```

### One important correction to keep in mind

`Nginx` is **not generating or validating the JWT in this example**.

Nginx → HTTPS/TLS
POCO  → REST API
POCO::JWT → JWT authentication

That's a good architecture for learning your **C++ + POCO + Nginx + HTTPS + JWT** stack.

Also, for a production implementation, use a sufficiently strong secret and validate the claims you actually require (such as expiration, issuer, and audience), rather than treating signature verification alone as complete authentication. 
POCO's `Token` supports claims including `exp`, `iss`, `aud`, `sub`, and `iat`. 
