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

### Connecting from WSL to Windows CARLA Server

If you're running the CARLA server on Windows and trying to connect from WSL (e.g., using the Docker client), you need to configure networking properly:

#### 1. Find the Windows Host IP Address

From WSL, you cannot use `localhost` or `127.0.0.1` to reach Windows. Instead, use the Windows network adapter IP:

```bash
# On Windows, open PowerShell or Command Prompt:
ipconfig /all

# Look for the "Ethernet adapter vEthernet (WSL)" section
# or your main network adapter and note the IPv4 address (e.g., 10.0.4.100)
```

Then use this IP when running the client:
```bash
docker run --rm carla-simple-client:latest 10.0.4.100 2000
```

#### 2. Configure Windows Firewall (Critical!)

**This is the most common reason for connection failures.** Windows Firewall blocks incoming connections from the WSL network on port 2000 by default.

To allow connections:

1. Press the **Windows key** and type **"Windows Defender Firewall with Advanced Security"**
2. Click on **Inbound Rules** in the left panel
3. Click **New Rule...** in the right panel
4. Select **Port** → Click **Next**
5. Select **TCP** and enter **Specific local ports**: `2000-2002`
   - CARLA uses port 2000 for RPC communication
   - Ports 2001-2002 are used for streaming sensor data
6. Click **Next** → Select **Allow the connection** → Click **Next**
7. Keep all checkboxes selected (Domain, Private, Public) → Click **Next**
8. Give the rule a name, e.g., **"CARLA Simulator WSL Access"**
9. Click **Finish**

#### 3. Verify the Connection

Test if the port is accessible from WSL:

```bash
# Install netcat if needed
sudo apt-get install netcat

# Test connection (should not immediately fail if port is open)
nc -zv 10.0.4.100 2000
```

### Connection refused
- Make sure the CARLA server is running on the specified host and port
- Check firewall settings if connecting to a remote server
- For WSL-to-Windows connections, see the section above

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
