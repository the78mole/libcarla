ARG UBUNTU_VERSION=22.04
FROM ubuntu:${UBUNTU_VERSION}

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install basic development tools
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    sudo \
    vim \
    # Boost libraries
    libboost-all-dev \
    # LibCarla dependencies
    libpng-dev \
    libtiff5-dev \
    libjpeg-dev \
    tzdata \
    sed \
    curl \
    unzip \
    autoconf \
    libtool \
    rsync \
    libxml2-dev \
    # Python for build scripts
    python3 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Create a non-root user
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Set working directory
WORKDIR /workspace

# Copy source code for building
COPY --chown=$USERNAME:$USERNAME . /workspace/

# Copy build script and make it available globally
COPY --chown=root:root .devcontainer/build-libcarla.sh /usr/local/bin/build-libcarla.sh
RUN chmod +x /usr/local/bin/build-libcarla.sh

# Build and install libcarla as root using the script
USER root
RUN /usr/local/bin/build-libcarla.sh /workspace

# Switch back to non-root user
USER $USERNAME

# Default shell
CMD ["/bin/bash"]
