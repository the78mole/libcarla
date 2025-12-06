# LibCarla

[![Build Status](https://github.com/the78mole/libcarla/actions/workflows/build-and-release.yml/badge.svg)](https://github.com/the78mole/libcarla/actions/workflows/build-and-release.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub Release](https://img.shields.io/github/v/release/the78mole/libcarla)](https://github.com/the78mole/libcarla/releases)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/the78mole/libcarla)

A standalone repository containing only the LibCarla library from the [CARLA Simulator](https://github.com/carla-simulator/carla) (version 0.9.16).

## Overview

This repository provides a standalone build of LibCarla, the core C++ library used by the CARLA autonomous driving simulator. It allows you to build and use LibCarla independently without the full CARLA simulator.

## Supported Platforms

- **Operating Systems**: Ubuntu 22.04, Ubuntu 24.04
- **Architectures**:
  - x86_64 (native build)
  - arm64 (cross-compilation)

## Features

- **Self-contained static libraries** - All dependencies are statically linked for easy deployment
- **Automated CI/CD pipeline** - GitHub Actions for building and releasing
- **Multi-platform support** - Pre-built binaries for Ubuntu 22.04/24.04 on x86_64 and arm64
- **Semantic versioning** - Automatic version management with dev/release tags
- **Pull request previews** - Automatic artifact builds and release notes for PRs
- Cross-platform build support with CMake and Ninja

## Release Contents

Each release includes:

- `libcarla_client.a` - Main LibCarla client static library
- `librpc.a` - RPC library for client-server communication
- `libRecast.a`, `libDetour.a`, `libDetourCrowd.a` - Navigation mesh libraries
- Header files for all libraries

## Installation

### Debian Packages (Recommended)

The easiest way to install LibCarla is using the pre-built Debian packages from the [Releases](https://github.com/the78mole/libcarla/releases) page:

```bash
# Download the appropriate package for your system
# Ubuntu 22.04 amd64
wget https://github.com/the78mole/libcarla/releases/download/v0.9.16/libcarla_0.9.16-ubuntu22.04_amd64.deb

# Ubuntu 24.04 amd64
wget https://github.com/the78mole/libcarla/releases/download/v0.9.16/libcarla_0.9.16-ubuntu24.04_amd64.deb

# Install the package
sudo dpkg -i libcarla_*.deb

# Install any missing dependencies
sudo apt-get install -f
```

**Available packages:**

- `libcarla_<version>-ubuntu22.04_amd64.deb` - Ubuntu 22.04 x86_64
- `libcarla_<version>-ubuntu24.04_amd64.deb` - Ubuntu 24.04 x86_64
- `libcarla_<version>-ubuntu22.04_arm64.deb` - Ubuntu 22.04 ARM64
- `libcarla_<version>-ubuntu24.04_arm64.deb` - Ubuntu 24.04 ARM64

The Debian package installs:

- Libraries to `/usr/lib`
- Headers to `/usr/include/carla`
- CMake config to `/usr/lib/cmake/libcarla`

**Package Details:**

The Debian packages are automatically created during the CI/CD build process from the compiled binaries. Each package includes proper dependency management and post-installation scripts for `ldconfig` integration.

### Pre-built Archives

Alternatively, download archive releases from the [Releases](https://github.com/the78mole/libcarla/releases) page.

Each archive contains:

```text
libcarla-<version>-<platform>/
├── lib/
│   ├── libcarla_client.a
│   ├── librpc.a
│   ├── libRecast.a
│   ├── libDetour.a
│   └── libDetourCrowd.a
└── include/
    ├── carla/           # LibCarla headers
    ├── rpc/             # RPC headers
    └── recast/          # Navigation headers
```

### Building from Source

#### Prerequisites

```bash
# Ubuntu
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    clang \
    libboost-all-dev \
    libpng-dev \
    libjpeg-dev \
    libtiff-dev \
    zlib1g-dev \
    python3-dev
```

#### Build Steps

```bash
# Clone the repository
git clone https://github.com/the78mole/libcarla.git
cd libcarla

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_SHARED_LIBS=OFF \
  -DLIBCARLA_VERSION=0.9.16

# Build
ninja
```

#### Install

```bash
sudo ninja install
```

This will install:

- Libraries to `/usr/local/lib`
- Headers to `/usr/local/include/carla`
- CMake config to `/usr/local/lib/cmake/libcarla`

### Python Package

```bash
cd python
pip install build
python -m build --wheel
pip install dist/*.whl
```

## Usage

### C++

```cpp
#include <carla/Version.h>
#include <iostream>

int main() {
    std::cout << "CARLA Version: " << carla::version() << std::endl;
    return 0;
}
```

### CMake Integration

```cmake
find_package(libcarla REQUIRED)
target_link_libraries(your_target PRIVATE libcarla::carla_client)
```

### Linking Manually

When linking manually, link in this order:

```bash
g++ your_code.cpp -o your_program \
    -L/path/to/libcarla/lib \
    -lcarla_client -lrpc -lRecast -lDetour -lDetourCrowd \
    -lboost_filesystem -lboost_system -lpng -ljpeg -lz -lpthread
```

### Python

```python
import carla
print(f"CARLA Version: {carla.__version__}")
```

## CI/CD Pipeline

This project uses GitHub Actions for automated building and releasing:

- **Automated Builds**: Every push and pull request triggers builds for all supported platforms
- **Artifact Upload**: Build artifacts (tar.gz and .deb) are automatically uploaded for each PR
- **Debian Packages**: Automatically created from build artifacts for easy installation
- **Release Notes**: Automatic generation of release notes with download links
- **Semantic Versioning**: Automatic version bumping based on commit messages

### Build Matrix

The CI/CD pipeline builds for:

- Ubuntu 22.04 x86_64 (native) + Debian package
- Ubuntu 24.04 x86_64 (native) + Debian package
- Ubuntu 22.04 arm64 (cross-compile) + Debian package
- Ubuntu 24.04 arm64 (cross-compile) + Debian package

Each build produces both a `.tar.gz` archive and a `.deb` package from the same compiled binaries.

### Testing Locally with act

You can test the GitHub Actions workflow locally using [act](https://github.com/nektos/act):

```bash
# Install act
gh extension install https://github.com/nektos/gh-act

# Run a specific job
gh act pull_request --job build-libcarla \
  -P ubuntu-22.04=catthehacker/ubuntu:act-22.04 \
  --matrix os:ubuntu-22.04 \
  --matrix arch:x86_64 \
  --artifact-server-path /tmp/artifacts
```

## Versioning

This project uses [Semantic Versioning](https://semver.org/). The version is automatically generated using [paulhatch/semantic-version](https://github.com/PaulHatch/semantic-version) GitHub Action.

- **MAJOR** version bumps when you include `(MAJOR)` in commit message
- **MINOR** version bumps when you include `(MINOR)` in commit message
- **PATCH** version bumps with every commit
- Development versions use `.devN` suffix for PR builds

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

The original CARLA Simulator is also released under the MIT License. See the [CARLA License](https://github.com/carla-simulator/carla/blob/master/LICENSE) for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

### Development Workflow

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Push to your fork and submit a Pull Request
5. The CI/CD pipeline will automatically build and test your changes
6. Review the generated artifacts and release notes in the PR comments

## Known Issues

- Ubuntu 22.04 builds require `#include <mutex>` in some source files due to older compiler behavior
- Cross-compilation for arm64 requires `gcc-aarch64-linux-gnu` toolchain

## Troubleshooting

### Build Errors

If you encounter build errors, ensure you have all prerequisites installed:

```bash
# Update package lists
sudo apt-get update

# Install all dependencies
sudo apt-get install -y build-essential cmake ninja-build clang \
    libboost-all-dev libpng-dev libjpeg-dev libtiff-dev zlib1g-dev python3-dev
```

### Cross-compilation for ARM64

For cross-compilation, install the ARM64 toolchain:

```bash
sudo apt-get install -y gcc-aarch64-linux-gnu g++-aarch64-linux-gnu
```

## Acknowledgments

- [CARLA Simulator](https://github.com/carla-simulator/carla) - The autonomous driving simulator
- [Computer Vision Center (CVC)](http://www.cvc.uab.es/) - Universitat Autònoma de Barcelona (UAB)
- All contributors who have helped improve this project
