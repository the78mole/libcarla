#!/bin/bash

# Script to test CARLA client with CARLA server in Docker containers

set -e

GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}=========================================${NC}"
echo -e "${GREEN}CARLA Client-Server Test${NC}"
echo -e "${GREEN}=========================================${NC}"
echo

# Check if client image exists
if ! docker image inspect carla-simple-client:latest >/dev/null 2>&1; then
    echo -e "${RED}Error: carla-simple-client:latest image not found${NC}"
    echo "Please run ./build-docker.sh first"
    exit 1
fi

echo -e "${YELLOW}Step 1: Cleaning up existing containers...${NC}"
docker stop carla-server 2>/dev/null || true
docker rm carla-server 2>/dev/null || true
docker stop carla-client 2>/dev/null || true
docker rm carla-client 2>/dev/null || true

echo -e "${YELLOW}Step 2: Creating Docker network...${NC}"
docker network create carla-network 2>/dev/null || echo "Network already exists"

echo -e "${YELLOW}Step 3: Starting CARLA server...${NC}"
echo "This may take 30-60 seconds to initialize..."
docker run -d \
    --name carla-server \
    --network carla-network \
    -p 2000:2000 \
    carlasim/carla:0.9.16 \
    /bin/bash -c "./CarlaUE4.sh -RenderOffScreen -quality-level=Low -nosound -carla-rpc-port=2000"

# Wait for server to be ready
echo -e "${YELLOW}Step 4: Waiting for CARLA server to be ready...${NC}"
MAX_WAIT=120
WAITED=0
until docker exec carla-server bash -c "timeout 1 bash -c '</dev/tcp/localhost/2000'" 2>/dev/null; do
    if [ $WAITED -ge $MAX_WAIT ]; then
        echo -e "${RED}Error: CARLA server did not start within ${MAX_WAIT} seconds${NC}"
        echo "Server logs:"
        docker logs carla-server | tail -30
        docker stop carla-server
        docker rm carla-server
        exit 1
    fi
    if [ $((WAITED % 10)) -eq 0 ]; then
        echo -n " ${WAITED}s"
    else
        echo -n "."
    fi
    sleep 3
    WAITED=$((WAITED + 3))
done
echo
echo -e "${GREEN}✓ CARLA server is ready!${NC}"

echo -e "${YELLOW}Step 5: Running CARLA client...${NC}"
docker run --rm \
    --name carla-client \
    --network carla-network \
    carla-simple-client:latest \
    carla-server 2000

CLIENT_EXIT=$?

echo
echo -e "${YELLOW}Step 6: Cleaning up...${NC}"
docker stop carla-server
docker rm carla-server
docker network rm carla-network 2>/dev/null || true

echo
if [ $CLIENT_EXIT -eq 0 ]; then
    echo -e "${GREEN}=========================================${NC}"
    echo -e "${GREEN}✓ Test completed successfully!${NC}"
    echo -e "${GREEN}=========================================${NC}"
else
    echo -e "${RED}=========================================${NC}"
    echo -e "${RED}✗ Test failed with exit code: $CLIENT_EXIT${NC}"
    echo -e "${RED}=========================================${NC}"
    exit $CLIENT_EXIT
fi
