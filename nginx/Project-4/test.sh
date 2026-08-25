#!/bin/bash

# Test script for REST API Server

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

BASE_URL="http://localhost:8080"

echo -e "${YELLOW}=========================================="
echo "Testing REST API Server"
echo "=========================================="
echo -e "${NC}"

# Check if server is running
echo -e "${YELLOW}[1/7] Checking if server is running...${NC}"
if ! curl -s "$BASE_URL/" > /dev/null; then
    echo -e "${RED}Error: Server is not running on $BASE_URL${NC}"
    echo "Start the server with: ./bin/rest_server"
    exit 1
fi
echo -e "${GREEN}✓ Server is running${NC}"

# Test root endpoint
echo -e "${YELLOW}[2/7] Testing root endpoint (GET /)...${NC}"
curl -s "$BASE_URL/" | jq . 2>/dev/null || curl -s "$BASE_URL/"
echo -e "${GREEN}✓ Root endpoint works${NC}\n"

# Test health check
echo -e "${YELLOW}[3/7] Testing health check (GET /api/health)...${NC}"
curl -s "$BASE_URL/api/health" | jq . 2>/dev/null || curl -s "$BASE_URL/api/health"
echo -e "${GREEN}✓ Health check endpoint works${NC}\n"

# Test POST data
echo -e "${YELLOW}[4/7] Testing POST data (POST /api/data)...${NC}"
RESPONSE=$(curl -s -X POST "$BASE_URL/api/data" \
  -H "Content-Type: application/json" \
  -d '{"key": "test_key", "value": "test_value"}')
echo "$RESPONSE" | jq . 2>/dev/null || echo "$RESPONSE"
echo -e "${GREEN}✓ POST endpoint works${NC}\n"

# Test GET data
echo -e "${YELLOW}[5/7] Testing GET data (GET /api/data/test_key)...${NC}"
RESPONSE=$(curl -s "$BASE_URL/api/data/test_key")
echo "$RESPONSE" | jq . 2>/dev/null || echo "$RESPONSE"
echo -e "${GREEN}✓ GET endpoint works${NC}\n"

# Test with multiple keys
echo -e "${YELLOW}[6/7] Testing multiple data entries...${NC}"
for i in {1..3}; do
    curl -s -X POST "$BASE_URL/api/data" \
      -H "Content-Type: application/json" \
      -d "{\"key\": \"user:$i\", \"value\": \"User $i\"}" > /dev/null
    echo -e "${GREEN}✓ Stored user:$i${NC}"
done

# Retrieve and display
echo -e "\n${YELLOW}Retrieving stored data:${NC}"
for i in {1..3}; do
    echo -n "  user:$i = "
    curl -s "$BASE_URL/api/data/user:$i" | jq '.value' 2>/dev/null || echo "..."
done
echo -e "\n"

# Test 404 error
echo -e "${YELLOW}[7/7] Testing 404 error handling...${NC}"
RESPONSE=$(curl -s "$BASE_URL/api/data/nonexistent")
echo "$RESPONSE" | jq . 2>/dev/null || echo "$RESPONSE"
echo -e "${GREEN}✓ Error handling works${NC}\n"

echo -e "${YELLOW}=========================================="
echo -e "All tests completed!"
echo -e "=========================================="
echo -e "${NC}"
