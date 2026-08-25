# REST API Server with C++17, Nginx, and Redis

A mini REST API server built with C++17, Poco framework, Nginx reverse proxy, and Redis caching.

## Project Structure

```
Project-1/
├── CMakeLists.txt              # CMake build configuration
├── docker-compose.yml          # Docker Compose configuration for Nginx + Redis
├── include/
│   ├── RestServer.h           # REST server class
│   ├── RedisCache.h           # Redis cache wrapper
│   └── RequestHandler.h       # HTTP request handler
├── src/
│   ├── main.cpp               # Entry point
│   ├── RestServer.cpp         # REST server implementation
│   ├── RedisCache.cpp         # Redis cache implementation
│   └── RequestHandler.cpp     # Request handler implementation
├── nginx/
│   └── nginx.conf             # Nginx configuration
└── build/                     # Build output directory
```

## Prerequisites

### System Dependencies (Manjaro Linux)

```bash
# Install development tools
sudo pacman -S base-devel cmake

# Install Poco C++ Libraries
sudo pacman -S poco

# Install Redis/Valkey (already installed)
# Verify:
redis-cli ping  # Should return PONG

# Install hiredis (Redis C client)
sudo pacman -S hiredis

# Install Nginx (already installed)
sudo systemctl status nginx
```

## Building the Project

### 1. Navigate to project directory
```bash
cd /home/shiv/Projects/cpp17/nginx/Project-1
```

### 2. Create and configure build
```bash
mkdir -p build
cd build
cmake ..
```

### 3. Build the project
```bash
make -j$(nproc)
```

### 4. Run the server
```bash
./bin/rest_server
```

The server will start on port 8080 and connect to Redis on localhost:6379.

## API Endpoints

### 1. Health Check
```bash
curl http://localhost:8080/
```

Response:
```json
{
    "message": "Welcome to REST API Server",
    "endpoints": {
        "GET /": "This message",
        "GET /api/health": "Health check",
        "GET /api/data/:key": "Get data from cache",
        "POST /api/data": "Store data in cache"
    }
}
```

### 2. Server Health
```bash
curl http://localhost:8080/api/health
```

Response:
```json
{"status": "healthy", "cache": "connected"}
```

### 3. Store Data in Cache
```bash
curl -X POST http://localhost:8080/api/data \
  -H "Content-Type: application/json" \
  -d '{"key": "user:1", "value": "John Doe"}'
```

Response:
```json
{"message": "Data stored", "key": "user:1"}
```

### 4. Retrieve Data from Cache
```bash
curl http://localhost:8080/api/data/user:1
```

Response:
```json
{"key": "user:1", "value": "John Doe"}
```

## Using Docker Compose

### Start Services (Nginx + Redis)
```bash
docker-compose up -d
```

This starts:
- **Valkey (Redis)** on port 6379
- **Nginx** on port 80 and 443

### Verify Services
```bash
docker ps
docker-compose logs -f
```

### Access through Nginx
```bash
curl http://localhost/api/health
```

### Stop Services
```bash
docker-compose down
```

## Testing the Complete Setup

### Terminal 1: Start Redis/Valkey
```bash
# Valkey should already be running as systemd service
redis-cli ping
```

### Terminal 2: Build and Run REST API Server
```bash
cd /home/shiv/Projects/cpp17/nginx/Project-1/build
cmake ..
make
./bin/rest_server
```

### Terminal 3: Test Nginx Reverse Proxy
```bash
# Start Nginx (if not already running)
sudo systemctl start nginx

# Or use Docker
docker-compose up -d

# Test the API
curl http://localhost:8080/
curl http://localhost/api/health
```

### Terminal 4: Test API Calls
```bash
# Store data
curl -X POST http://localhost:8080/api/data \
  -H "Content-Type: application/json" \
  -d '{"key": "test", "value": "hello world"}'

# Retrieve data
curl http://localhost:8080/api/data/test

# Through Nginx proxy
curl http://localhost/api/data/test
```

## Configuration

### Nginx Configuration
Edit `nginx/nginx.conf` to customize:
- Worker processes
- Rate limiting (currently 10 req/s for GET, 5 req/s for POST)
- Upstream server address
- SSL/TLS settings
- Logging

### Redis Configuration
Edit `/etc/redis/redis.conf` to customize:
- Port (default 6379)
- Persistence settings
- Memory limits

### Application Configuration
Modify `src/main.cpp` to change:
- REST server port (default 8080)
- Redis host/port (default 127.0.0.1:6379)

## Stopping Services

```bash
# Stop REST API Server
# Press Ctrl+C in the terminal running the server

# Stop Nginx
sudo systemctl stop nginx

# Stop Redis/Valkey
sudo systemctl stop valkey

# Or stop Docker services
docker-compose down
```

## Troubleshooting

### Redis Connection Failed
```bash
# Check if Redis is running
redis-cli ping

# Start Redis
sudo systemctl start valkey
```

### Port Already in Use
```bash
# Find process using port
sudo lsof -i :8080  # For REST API
sudo lsof -i :80    # For Nginx

# Kill process if needed
sudo kill -9 <PID>
```

### CMake Not Finding Poco
```bash
# Install Poco development libraries
sudo pacman -S poco

# Or manually specify path in CMake
cmake -DCMAKE_PREFIX_PATH=/usr/local/poco ..
```

### Build Errors
```bash
# Clean build
rm -rf build
mkdir build
cd build
cmake ..
make clean
make -j$(nproc)
```

## Performance Tuning

### Nginx Rate Limiting
Modify `nginx.conf` zone parameters:
```nginx
limit_req_zone $binary_remote_addr zone=api_limit:10m rate=20r/s;
```

### Redis Memory Limits
Edit `/etc/redis/redis.conf`:
```
maxmemory 256mb
maxmemory-policy allkeys-lru
```

### Thread Pool
Adjust HTTP server threads in `src/RestServer.cpp`:
```cpp
params->setMaxThreads(20);
params->setMaxQueued(50);
```

## License

MIT License

## Support

For issues with:
- **Poco Framework**: [poco.apache.org](https://pocoproject.org/)
- **Redis/Valkey**: [valkey.io](https://valkey.io/)
- **Nginx**: [nginx.org](https://nginx.org/)
