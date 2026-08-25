# Quick Start Guide

## Overview
This is a REST API server built with:
- **C++17** with Poco Framework
- **Redis/Valkey** for caching
- **Nginx** as reverse proxy
- **Docker Compose** for easy deployment

## 1-Minute Quick Start

### Option A: Using Makefile (Recommended)

```bash
cd /home/shiv/Projects/cpp17/nginx/Project-1

# Install dependencies (first time only)
make setup

# Build and run the server
make run

# In another terminal, test the API
make test
```

### Option B: Step-by-Step

```bash
cd /home/shiv/Projects/cpp17/nginx/Project-1

# 1. Setup (first time only)
./setup.sh

# 2. Build
./build.sh

# 3. Run server
cd build
./bin/rest_server

# 4. Test (in another terminal)
cd /home/shiv/Projects/cpp17/nginx/Project-1
./test.sh
```

## What Gets Started

1. **REST API Server** - Runs on `http://localhost:8080`
2. **Redis/Valkey** - Cache backend on `localhost:6379` (already running as systemd service)
3. **Nginx** - Reverse proxy on `http://localhost` (optional, use Docker Compose)

## Quick API Tests

Once the server is running:

```bash
# Health check
curl http://localhost:8080/api/health

# Store data
curl -X POST http://localhost:8080/api/data \
  -H "Content-Type: application/json" \
  -d '{"key": "hello", "value": "world"}'

# Retrieve data
curl http://localhost:8080/api/data/hello
```

## Using Docker (Optional)

```bash
# Start Nginx + Redis containers
docker-compose up -d

# View logs
docker-compose logs -f

# Test through Nginx
curl http://localhost/api/health

# Stop
docker-compose down
```

## Project Files

| File | Purpose |
|------|---------|
| `src/main.cpp` | Entry point |
| `src/RestServer.cpp` | HTTP server implementation |
| `src/RedisCache.cpp` | Redis client wrapper |
| `src/RequestHandler.cpp` | Request routing & handling |
| `include/*.h` | Header files |
| `CMakeLists.txt` | Build configuration |
| `nginx/nginx.conf` | Nginx reverse proxy config |
| `docker-compose.yml` | Docker Compose setup |

## Modify & Rebuild

### Change server port
Edit `src/main.cpp`, line with `RestServer server(8080);`

### Change Redis connection
Edit `src/main.cpp`, modify `RedisCache()` parameters

### Add new API endpoint
Edit `src/RequestHandler.cpp`, add logic in `handleGetRequest()` or `handlePostRequest()`

Then rebuild:
```bash
make build
make run
```

## Troubleshooting

| Problem | Solution |
|---------|----------|
| "Unit redis.service not found" | Run `sudo pacman -S redis` or use Docker |
| Port 8080 already in use | Edit port in `src/main.cpp` or kill process: `sudo lsof -i :8080` |
| CMake not finding Poco | Run `sudo pacman -S poco` |
| Build fails | Run `make clean` and retry, or check `build.sh` output |

## Next Steps

1. **Add Authentication** - Implement API key/JWT validation
2. **Add Database** - Integrate SQL database with Poco::Data
3. **Add Logging** - Use Poco::Logger for better debugging
4. **Add Tests** - Create unit tests with Google Test
5. **Deploy** - Use Docker/Kubernetes for production

## Useful Commands

```bash
make help          # Show all available commands
make build         # Rebuild project
make test          # Run API tests
make clean         # Clean build artifacts
make docker-up     # Start Nginx + Redis
make docker-logs   # View Docker logs

redis-cli          # Interactive Redis client
redis-cli INFO     # Redis server info
redis-cli FLUSHALL # Clear all Redis data

curl -v URL        # Verbose curl (see headers)
```

## Project Links

- Poco Documentation: https://pocoproject.org/
- Redis Commands: https://redis.io/commands/
- Nginx Documentation: https://nginx.org/docs/
- C++17 Reference: https://en.cppreference.com/w/cpp/17
