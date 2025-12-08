#!/bin/bash
# Local test build script - uses locally built .deb package

set -e

cd "$(dirname "$0")"

# Check if we have a local .deb file
DEB_FILE=$(ls -1 ../*.deb 2>/dev/null | head -1)

if [ -z "$DEB_FILE" ]; then
    echo "Error: No .deb file found in parent directory"
    echo ""
    echo "Please build libcarla first or copy a .deb package to the parent directory:"
    echo "  cd .."
    echo "  mkdir build && cd build"
    echo "  cmake .."
    echo "  make package"
    echo ""
    exit 1
fi

echo "========================================="
echo "Local Docker Build Test"
echo "========================================="
echo "Using .deb package: $(basename $DEB_FILE)"
echo "========================================="
echo ""

# Copy the .deb file to examples directory
cp "$DEB_FILE" .

# Build the Docker image
docker build \
    -f Dockerfile.local \
    -t carla-simple-client:local-test \
    .

# Clean up copied .deb
rm -f *.deb

echo ""
echo "========================================="
echo "Build completed successfully!"
echo "========================================="
echo "Image: carla-simple-client:local-test"
echo ""
echo "To test the example (requires CARLA server running):"
echo "  docker run --rm --network host carla-simple-client:local-test localhost 2000"
echo ""
echo "To inspect the image:"
echo "  docker run --rm -it --entrypoint /bin/bash carla-simple-client:local-test"
echo "========================================="
