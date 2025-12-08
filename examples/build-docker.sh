#!/bin/bash
# Build script for CARLA client example Docker image

set -e

# Default values
UBUNTU_VERSION="24.04"
LIBCARLA_VERSION="latest"
IMAGE_NAME="carla-simple-client"
IMAGE_TAG="latest"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --ubuntu)
            UBUNTU_VERSION="$2"
            shift 2
            ;;
        --version)
            LIBCARLA_VERSION="$2"
            shift 2
            ;;
        --image-name)
            IMAGE_NAME="$2"
            shift 2
            ;;
        --image-tag)
            IMAGE_TAG="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --ubuntu VERSION      Ubuntu version (22.04 or 24.04, default: 22.04)"
            echo "  --version VERSION     LibCarla version (default: latest)"
            echo "  --image-name NAME     Docker image name (default: carla-simple-client)"
            echo "  --image-tag TAG       Docker image tag (default: latest)"
            echo "  --help                Show this help message"
            echo ""
            echo "Examples:"
            echo "  $0                                    # Build with defaults"
            echo "  $0 --ubuntu 24.04                     # Build for Ubuntu 24.04"
            echo "  $0 --version 0.0.25.dev               # Build with specific version"
            echo "  $0 --ubuntu 24.04 --version 0.0.25.dev --image-tag ubuntu24.04"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Determine Dockerfile
if [ "$UBUNTU_VERSION" = "24.04" ]; then
    DOCKERFILE="Dockerfile.ubuntu24"
else
    DOCKERFILE="Dockerfile"
fi

# Build the Docker image
echo "========================================="
echo "Building CARLA Client Example"
echo "========================================="
echo "Ubuntu Version:   $UBUNTU_VERSION"
echo "LibCarla Version: $LIBCARLA_VERSION"
echo "Dockerfile:       $DOCKERFILE"
echo "Image Name:       $IMAGE_NAME:$IMAGE_TAG"
echo "========================================="
echo ""

docker build \
    -f "$DOCKERFILE" \
    --build-arg UBUNTU_VERSION="$UBUNTU_VERSION" \
    --build-arg LIBCARLA_VERSION="$LIBCARLA_VERSION" \
    -t "$IMAGE_NAME:$IMAGE_TAG" \
    .

echo ""
echo "========================================="
echo "Build completed successfully!"
echo "========================================="
echo "Image: $IMAGE_NAME:$IMAGE_TAG"
echo ""
echo "To run the example:"
echo "  docker run --rm $IMAGE_NAME:$IMAGE_TAG <host> <port>"
echo ""
echo "Example (CARLA server on host machine):"
echo "  docker run --rm --network host $IMAGE_NAME:$IMAGE_TAG localhost 2000"
echo ""
echo "Example (CARLA server on remote host):"
echo "  docker run --rm $IMAGE_NAME:$IMAGE_TAG 192.168.1.100 2000"
echo "========================================="
