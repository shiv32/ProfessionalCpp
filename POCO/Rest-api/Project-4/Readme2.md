**TLS certificate and private key**.

For local learning/testing, you can create a **self-signed certificate**.

### 1. Create the directory

```bash
sudo mkdir -p /etc/nginx/ssl
```

### 2. Generate a local certificate

```bash
sudo openssl req -x509 -nodes -days 365 \
    -newkey rsa:2048 \
    -keyout /etc/nginx/ssl/server.key \
    -out /etc/nginx/ssl/server.crt
```

It will ask you some questions. For **Common Name**, enter:

```text
localhost
```

You should now have:

```text
/etc/nginx/ssl/
├── server.crt
└── server.key
```

Check:

```bash
ls -l /etc/nginx/ssl/
```

### 3. Use them in Nginx

Your Nginx config can now contain:

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

### 4. Test Nginx configuration

```bash
sudo nginx -t
```

If successful:

```text
syntax is ok
test is successful
```

Then:

```bash
sudo systemctl restart nginx
```

### 5. Start your POCO server

Your C++ program should still listen on:

```text
127.0.0.1:8080
```

Then test the complete setup:

```bash
curl -k https://localhost/api/hello
```

You should get:

```json
{"message":"Hello from C++ POCO API"}
```

`-k` is needed because the certificate is **self-signed** and therefore isn't trusted by your system.

**Important:** For a real production server, don't use this self-signed certificate. 
You would normally use a certificate issued by a trusted CA, such as Let's Encrypt.

----------------------------------
Disable SSL verification in Postman

In Postman:

File -> Settings → General → SSL certificate verification → OFF

Then try:

GET https://localhost/api/hello
---------------------------------