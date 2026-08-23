Basic C++ rest api project

Run project steps after compile.

Run

./drogon_test

Leave this terminal running.

You should see Drogon listening on port `8080`.

Test REST API

Open **another terminal**:

curl http://127.0.0.1:8080/hello

Expected:

Hello from C++ Drogon!

### What you just built

curl
  │
  │ GET /hello
  ▼
Drogon
  │
  ▼
C++ Handler
  │
  ▼
HTTP 200
  │
  ▼
Hello from C++ Drogon!

This confirms that **Drogon + C++ + CMake + HTTP REST routing** are all working.
