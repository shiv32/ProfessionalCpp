#!/bin/bash

# Setup script for REST API Server dependencies

echo "=========================================="
echo "Setting up REST API Server dependencies"
echo "=========================================="

# Check if running on Manjaro/Arch
if ! command -v pacman &> /dev/null; then
    echo "Error: This script is designed for Arch/Manjaro Linux"
    echo "Please install dependencies manually"
    exit 1
fi

echo "Installing dependencies..."

# Update package database
echo "Updating package database..."
sudo pacman -Sy

# Install required packages
echo "Installing required packages..."
sudo pacman -S --noconfirm \
    base-devel \
    cmake \
    poco \
    hiredis \
    nginx

# Verify installations
echo ""
echo "Verifying installations..."

if command -v cmake &> /dev/null; then
    echo "✓ CMake: $(cmake --version | head -1)"
fi

if command -v make &> /dev/null; then
    echo "✓ Make installed"
fi

if pacman -Q poco &> /dev/null; then
    echo "✓ Poco Framework installed"
fi

if pacman -Q hiredis &> /dev/null; then
    echo "✓ Hiredis installed"
fi

if command -v redis-cli &> /dev/null; then
    echo "✓ Redis CLI available"
fi

if command -v nginx &> /dev/null; then
    echo "✓ Nginx installed"
fi

# Check if Redis/Valkey is running
echo ""
echo "Checking Redis/Valkey status..."
if redis-cli ping &> /dev/null; then
    echo "✓ Redis/Valkey is running"
else
    echo "⚠ Redis/Valkey is not running. Start it with:"
    echo "  sudo systemctl start valkey"
fi

echo ""
echo "=========================================="
echo "Setup completed!"
echo "=========================================="
echo ""
echo "Next steps:"
echo "1. cd $(pwd)"
echo "2. ./build.sh"
echo "3. ./bin/rest_server"
echo "4. In another terminal: ./test.sh"
echo ""
