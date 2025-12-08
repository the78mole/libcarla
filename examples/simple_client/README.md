# Simple CARLA Client Example

A basic example demonstrating how to connect to a CARLA server, spawn a vehicle, and control it using the autopilot feature.

## What This Example Does

1. Connects to a CARLA server
2. Retrieves world and blueprint information
3. Finds the spawn point closest to a target location
4. Spawns a Tesla Model 3
5. Enables autopilot for 60 seconds
6. Drives manually for 5 seconds
7. Stops and destroys the vehicle

## Building

### Using Docker (Recommended)

```bash
# Build for Ubuntu 22.04 (default)
./build-docker.sh

# Build for Ubuntu 24.04
./build-docker.sh --ubuntu 24.04

# Build with specific libcarla version
./build-docker.sh --version 0.0.16
```

The build script will:
- Download the libcarla `.deb` package from GitHub releases
- Install all dependencies
- Compile the example
- Create a minimal Docker image with just the executable

### Local Build

If you have libcarla installed locally:

```bash
mkdir build && cd build
cmake ..
make
```

## Running

### With Docker

```bash
# Connect to CARLA server on Windows from WSL
docker run --rm carla-simple-client:latest 10.0.4.100 2000

# Connect to local CARLA server on Linux
docker run --rm --network host carla-simple-client:latest localhost 2000

# Connect to remote CARLA server
docker run --rm carla-simple-client:latest 192.168.1.100 2000
```

### Local Build

```bash
./build/simple_client <host> <port>

# Examples:
./build/simple_client localhost 2000
./build/simple_client 10.0.4.100 2000
```

## Expected Output

```
Connecting to CARLA server at 10.0.4.100:2000...
Connected to CARLA server version: 0.9.16
World loaded: Carla/Maps/Town10HD_Opt
Blueprint library contains 214 blueprints
Found vehicle blueprint: vehicle.tesla.model3
Total spawn points available: 155

Target location: (-83.5462, 131.02, 0)
Closest spawn point [78] at distance 12.3446m: (-71.2697, 132.315, 0.6)
Spawning vehicle...
Vehicle spawned with ID: 37

Enabling autopilot for 60 seconds...
  [1s] Position: (-72.2023, 132.032, 0.00139183) Speed: 3.90585 m/s
  [2s] Position: (-78.2366, 129.805, 0.00166454) Speed: 8.87542 m/s
  ...
  [60s] Position: (90.787, -50.702, 0.00181858) Speed: 8.57336 m/s

Disabling autopilot...
Manual driving for 5 seconds...
  [1s] Position: (98.8374, -43.3295, 0.00174168) Speed: 12.923 m/s
  ...

Stopping vehicle...
Destroying vehicle...
Example completed successfully!
```

## Customizing

### Change Target Spawn Location

Edit `main.cpp` and modify the target location:

```cpp
// Target location near the curve
cg::Location target(-83.5462f, 131.02f, 0.0f);
```

The example will automatically find the closest spawn point to this location.

### Change Autopilot Duration

Edit `main.cpp` and modify the loop count:

```cpp
// Monitor vehicle position for 60 seconds
for (int i = 0; i < 60; ++i) {
```

### Change Vehicle Blueprint

Edit `main.cpp` and modify the blueprint ID:

```cpp
auto vehicle_bp = blueprint_library->Find("vehicle.tesla.model3");
```

Other options include:
- `vehicle.audi.a2`
- `vehicle.bmw.grandtourer`
- `vehicle.chevrolet.impala`
- `vehicle.dodge.charger_2020`
- `vehicle.ford.mustang`
- `vehicle.mercedes.coupe_2020`

## Troubleshooting

### Cannot connect to server

See the [main examples README](../README.md) for detailed troubleshooting, especially the section on connecting from WSL to Windows.

### Vehicle spawning fails

- The target location might not have nearby spawn points
- Try a different map with more spawn points
- Change the target location in the code

### Autopilot not working

- Some maps have better navigation than others
- The vehicle might get stuck at traffic lights (this is normal)
- Try a different spawn location

## Code Structure

- `main.cpp` - Main example code
- `CMakeLists.txt` - CMake build configuration
- `Dockerfile` - Multi-stage Docker build for Ubuntu 22.04
- `Dockerfile.ubuntu24` - Multi-stage Docker build for Ubuntu 24.04
- `Dockerfile.local` - Docker build using local `.deb` package
- `build-docker.sh` - Build script for Docker images

## See Also

- [CARLA Documentation](https://carla.readthedocs.io/)
- [LibCarla API Reference](https://github.com/the78mole/libcarla)
- [Main Examples README](../README.md)
