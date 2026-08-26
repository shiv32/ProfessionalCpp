In **Postman**, you don't type `-H`. 
That is curl syntax.

For your request:

```text
GET https://localhost/api/hello
```

do this:

1. Open the request in Postman.
2. Select the **Authorization** tab.
3. Set **Type** to **Bearer Token**.
4. Paste the JWT you received from `/login` into the **Token** field.

It should look like:

```text
Authorization
Type: Bearer Token

Token:
eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9....
```

Postman automatically sends:

```http
Authorization: Bearer eyJhbGciOiJIUzI1NiIs...
```

### Alternative: Headers tab

You can also manually add:

| Key             | Value               |
| --------------- | ------------------- |
| `Authorization` | `Bearer YOUR_TOKEN` |

**Don't put `Bearer` in the Token field when using Postman's "Bearer Token" authorization type**—Postman adds it automatically.

For your current POCO API, the easiest flow is:

POST https://localhost/api/login
        ↓
    Get JWT token
        ↓
GET https://localhost/api/hello
        ↓
Authorization → Bearer Token
        ↓
    Paste JWT
        ↓
       200 OK
```

Since you're using a self-signed certificate, keep **Postman → Settings → General → SSL certificate verification = OFF** for this local test.
