# LibCarla

A standalone repository containing only the LibCarla library from the [CARLA Simulator](https://github.com/carla-simulator/carla) (version 0.9.16).

## Overview

This repository provides a standalone build of LibCarla, the core C++ library used by the CARLA autonomous driving simulator. It allows you to build and use LibCarla independently without the full CARLA simulator.

## Supported Platforms

- **Operating Systems**: Ubuntu 22.04, Ubuntu 24.04
- **Architectures**: x86_64, arm64

## Features

- Pre-built LibCarla binaries for multiple platforms
- Python wheels with precompiled binaries
- Semantic versioning with automatic patch version bumps
- Cross-platform build support

## Installation

### Pre-built Releases

Download the latest release from the [Releases](https://github.com/the78mole/libcarla/releases) page.

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
    libxml2-dev \
    libsqlite3-dev \
    zlib1g-dev \
    python3-dev
```

#### Build

```bash
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```

#### Install

```bash
sudo ninja install
```

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

### Python

```python
import carla
print(f"CARLA Version: {carla.__version__}")
```

## Versioning

This project uses [Semantic Versioning](https://semver.org/). The version is automatically generated using [paulhatch/semantic-version](https://github.com/PaulHatch/semantic-version) GitHub Action.

- **MAJOR** version bumps when you include `(MAJOR)` in commit message
- **MINOR** version bumps when you include `(MINOR)` in commit message
- **PATCH** version bumps with every commit

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

The original CARLA Simulator is also released under the MIT License. See the [CARLA License](https://github.com/carla-simulator/carla/blob/master/LICENSE) for details.

## Acknowledgments

- [CARLA Simulator](https://github.com/carla-simulator/carla) - The autonomous driving simulator
- [Computer Vision Center (CVC)](http://www.cvc.uab.es/) - Universitat Autònoma de Barcelona (UAB)
