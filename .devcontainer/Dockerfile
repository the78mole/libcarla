ARG UBUNTU_VERSION=22.04
FROM public.ecr.aws/ubuntu/ubuntu:${UBUNTU_VERSION}

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
    gpg \
    lsb-release \
    zsh \
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
    # ARM64 cross-compilation toolchain
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    binutils-aarch64-linux-gnu \
    && rm -rf /var/lib/apt/lists/*

# Add ARM64 architecture for cross-compilation
RUN dpkg --add-architecture arm64 \
    && sed -i 's/deb http/deb [arch=amd64] http/g' /etc/apt/sources.list \
    && echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports $(lsb_release -cs) main universe" >> /etc/apt/sources.list.d/arm64.list \
    && echo "deb [arch=arm64] http://ports.ubuntu.com/ubuntu-ports $(lsb_release -cs)-updates main universe" >> /etc/apt/sources.list.d/arm64.list \
    && apt-get update \
    && apt-get install -y \
        libboost-filesystem-dev:arm64 \
        libboost-system-dev:arm64 \
    && rm -rf /var/lib/apt/lists/*

# Configure LibCarla APT repository
ARG REPO_OWNER=the78mole
ARG REPO_NAME=libcarla
RUN curl -fsSL https://${REPO_OWNER}.github.io/${REPO_NAME}/public.key | gpg --dearmor -o /usr/share/keyrings/libcarla-archive-keyring.gpg \
    && echo "deb [arch=amd64,arm64 signed-by=/usr/share/keyrings/libcarla-archive-keyring.gpg] https://${REPO_OWNER}.github.io/${REPO_NAME} stable main" > /etc/apt/sources.list.d/libcarla.list \
    && apt-get update

# Create a non-root user
ARG USERNAME=vscode
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME -s /bin/zsh \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Install oh-my-zsh for vscode user
USER $USERNAME
RUN sh -c "$(curl -fsSL https://raw.githubusercontent.com/ohmyzsh/ohmyzsh/master/tools/install.sh)" "" --unattended
USER root

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
