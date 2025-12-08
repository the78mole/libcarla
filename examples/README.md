# LibCarla Examples

This directory contains examples demonstrating how to use libcarla to interact with a CARLA simulator server.

## Prerequisites

Before running these examples, make sure you have:

1. **LibCarla installed**: Either install the `.deb` package or build from source
2. **CARLA Server running**: Download and run the CARLA simulator server from [carla.org](https://carla.org)
   - The CARLA server requires a GPU with OpenGL/Vulkan support
   - Running CARLA in Docker requires GPU passthrough (NVIDIA Docker runtime)
   - For testing without GPU, you can run CARLA natively on your host machine
3. **CMake 3.22+** and a C++ compiler

## Building the Examples

### Option 1: Using Docker (Recommended)

The easiest way to build and run the examples is using Docker with the pre-built `.deb` packages from GitHub releases:

```bash
cd examples

# Build for Ubuntu 22.04 (default)
./build-docker.sh

# Build for Ubuntu 24.04
./build-docker.sh --ubuntu 24.04

# Build with specific libcarla version
./build-docker.sh --version 0.0.25.dev

# Custom image name and tag
./build-docker.sh --ubuntu 24.04 --image-tag ubuntu24.04
```

The Docker build process:
1. Downloads the `.deb` package from GitHub releases
2. Installs libcarla and dependencies
3. Builds the example
4. Creates a minimal runtime image

### Option 2: Using installed libcarla package

```bash
cd examples
mkdir build
cd build
cmake ..
make
```

### Option 3: Using libcarla from source

```bash
cd examples
mkdir build
cd build
cmake -Dlibcarla_DIR=/path/to/libcarla/build ..
make
```

## Running the Examples

### Simple Client

The simple client example demonstrates basic operations:
- Connecting to the CARLA server
- Getting world information
- Spawning a vehicle
- Controlling the vehicle
- Cleaning up

**Usage:**
```bash
# Start CARLA server first (default: localhost:2000)
./CarlaUE4.sh

# In another terminal, run the example:
./build/simple_client

# Or specify custom host and port:
./build/simple_client localhost 2000
```

**Expected Output:**
```
Connecting to CARLA server at localhost:2000...
Connected to CARLA server version: 0.9.16
World loaded: Town01
Blueprint library contains 500+ blueprints
Found vehicle blueprint: vehicle.tesla.model3
Spawn point: (x, y, z)
Spawning vehicle...
Vehicle spawned with ID: 123
Driving vehicle forward...
Stopping vehicle...
Destroying vehicle...
Example completed successfully!
```

## Troubleshooting

### Connection refused
- Make sure the CARLA server is running on the specified host and port
- Check firewall settings if connecting to a remote server

### Blueprint not found
- The example uses `vehicle.tesla.model3`. If this blueprint is not available in your CARLA version, modify the code to use a different vehicle blueprint
- List available blueprints using: `client.GetWorld().GetBlueprintLibrary()`

### Spawn point issues
- If no spawn points are available, the map may not have recommended spawn points defined
- Try a different map (e.g., Town01, Town02, etc.)

## Additional Examples

More examples coming soon:
- Sensor data collection
- Traffic scenario simulation
- Autonomous driving agent
- Multi-vehicle coordination

## Documentation

For more information about libcarla API:
- [CARLA Documentation](https://carla.readthedocs.io/)
- [LibCarla Source Code](https://github.com/the78mole/libcarla)
