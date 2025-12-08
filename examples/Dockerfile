# Multi-stage Dockerfile for building CARLA client example
# Uses the libcarla .deb package from GitHub releases

ARG UBUNTU_VERSION=22.04
FROM ubuntu:${UBUNTU_VERSION} as builder

# Install build dependencies
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    cmake \
    build-essential \
    wget \
    libboost-filesystem-dev \
    libboost-system-dev \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /build

# Download and install libcarla .deb package from GitHub releases
# Default to latest release, can be overridden with build arg
ARG LIBCARLA_VERSION=latest
ARG TARGETARCH=amd64
ARG UBUNTU_VERSION=22.04

# Download the .deb package from GitHub releases
# Note: Package naming format is libcarla_VERSION-ubuntu-XX.XX_ARCH.deb
RUN if [ "${LIBCARLA_VERSION}" = "latest" ]; then \
        RELEASE_INFO=$(wget -qO- https://api.github.com/repos/the78mole/libcarla/releases/latest); \
        VERSION=$(echo "${RELEASE_INFO}" | grep '"tag_name"' | cut -d '"' -f 4 | sed 's/^v//'); \
        DOWNLOAD_URL=$(echo "${RELEASE_INFO}" | grep "browser_download_url.*ubuntu-${UBUNTU_VERSION}_${TARGETARCH}.deb" | cut -d '"' -f 4); \
    else \
        VERSION="${LIBCARLA_VERSION}"; \
        DOWNLOAD_URL="https://github.com/the78mole/libcarla/releases/download/v${VERSION}/libcarla_${VERSION}-ubuntu-${UBUNTU_VERSION}_${TARGETARCH}.deb"; \
    fi && \
    echo "Downloading libcarla ${VERSION} from: ${DOWNLOAD_URL}" && \
    wget -O libcarla.deb "${DOWNLOAD_URL}" && \
    dpkg -i libcarla.deb || apt-get install -f -y && \
    rm libcarla.deb

# Copy example source code
COPY simple_client.cpp .
COPY CMakeLists.txt .

# Build the example
RUN mkdir build && cd build && \
    cmake .. && \
    make -j$(nproc)

# Runtime stage - minimal image with just the executable
FROM ubuntu:${UBUNTU_VERSION} as runtime

# Install runtime dependencies only
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    libboost-filesystem1.74.0 \
    libboost-system1.74.0 \
    && rm -rf /var/lib/apt/lists/*

# Copy the built executable
COPY --from=builder /build/build/simple_client /usr/local/bin/simple_client

# Set the entrypoint
ENTRYPOINT ["/usr/local/bin/simple_client"]

# Default to localhost:2000, but can be overridden
CMD ["localhost", "2000"]
