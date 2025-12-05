#!/bin/bash
# Script to fetch LibCarla sources from CARLA 0.9.16

set -e

CARLA_VERSION="0.9.16"
CARLA_REPO="https://github.com/carla-simulator/carla"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(dirname "$SCRIPT_DIR")"
TEMP_DIR=$(mktemp -d)

echo "Fetching LibCarla sources from CARLA ${CARLA_VERSION}..."

# Clone the CARLA repository (sparse checkout for LibCarla only)
cd "$TEMP_DIR"
git init carla
cd carla
git remote add origin "$CARLA_REPO"
git config core.sparseCheckout true
echo "LibCarla/source/" >> .git/info/sparse-checkout
git fetch --depth=1 origin "$CARLA_VERSION"
git checkout FETCH_HEAD

# Copy LibCarla sources to our source directory
echo "Copying LibCarla sources..."
rm -rf "$ROOT_DIR/source/carla"
rm -rf "$ROOT_DIR/source/third-party"
cp -r LibCarla/source/carla "$ROOT_DIR/source/"
cp -r LibCarla/source/third-party "$ROOT_DIR/source/"
cp -r LibCarla/source/compiler "$ROOT_DIR/source/" 2>/dev/null || true

# Clean up
rm -rf "$TEMP_DIR"

echo "Done! LibCarla sources have been fetched."
