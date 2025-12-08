#!/bin/bash
set -e

echo "==========================================="
echo "Building and Installing LibCarla"
echo "==========================================="

# Get the workspace directory (default to current if not set)
WORKSPACE_DIR="${1:-$(pwd)}"
echo "Workspace: $WORKSPACE_DIR"

# Build libcarla
echo ""
echo "Building libcarla..."
cd "$WORKSPACE_DIR"

# Create build directory if it doesn't exist
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake ..

# Build
echo "Building..."
make -j$(nproc)

# Install (requires sudo if not root)
echo "Installing libcarla..."
if [ "$EUID" -eq 0 ]; then
    make install
    ldconfig
else
    sudo make install
    sudo ldconfig
fi

# Verify installation
echo ""
echo "Verifying installation..."
if [ -f /usr/local/lib/libcarla_client.so ]; then
    echo "✓ libcarla_client.so installed"
else
    echo "✗ libcarla_client.so not found"
fi

if [ -d /usr/local/include/carla ]; then
    echo "✓ Headers installed"
else
    echo "✗ Headers not found"
fi

echo ""
echo "==========================================="
echo "LibCarla build complete!"
echo "==========================================="
echo ""
echo "You can now:"
echo "  - Build examples: cd examples/simple_client && mkdir build && cd build && cmake .. && make"
echo "  - Rebuild libcarla: build-libcarla.sh"
echo ""
