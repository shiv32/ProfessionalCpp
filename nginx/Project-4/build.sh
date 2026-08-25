#!/bin/bash

# Build script for REST API Server

set -e

echo "=========================================="
echo "Building REST API Server"
echo "=========================================="

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo -e "${YELLOW}Project directory: $PROJECT_DIR${NC}"

# Check prerequisites
echo -e "${YELLOW}Checking prerequisites...${NC}"

command -v cmake >/dev/null 2>&1 || { echo "CMake is required but not installed. Install with: sudo pacman -S cmake"; exit 1; }
command -v make >/dev/null 2>&1 || { echo "Make is required but not installed. Install with: sudo pacman -S make"; exit 1; }

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# Navigate to build directory
cd "$BUILD_DIR"

# Run CMake
echo -e "${YELLOW}Running CMake...${NC}"
cmake ..

# Build
echo -e "${YELLOW}Building project...${NC}"
make -j$(nproc)

echo -e "${GREEN}=========================================="
echo "Build completed successfully!"
echo "=========================================="
echo "Output: $BUILD_DIR/bin/rest_server"
echo ""
echo "To run the server:"
echo "  cd $BUILD_DIR"
echo "  ./bin/rest_server"
echo "=========================================="
echo -e "${NC}"
