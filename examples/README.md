# LibCarla Examples

This directory contains examples demonstrating how to use libcarla to interact with a CARLA simulator server.

## Available Examples

- **[simple_client](simple_client/)** - Basic client that connects to CARLA, spawns a vehicle, and controls it using autopilot

## General Setup

### Running CARLA Server on Windows with WSL Clients

This is a common development setup where you run the CARLA Unreal Engine server on Windows (which has GPU access) and connect to it from clients running in WSL/Docker.

#### 1. Start CARLA Server on Windows

Download and extract the CARLA server from [carla.org](https://carla.org) or the GitHub releases, then run:

```powershell
# Navigate to CARLA directory
cd C:\CARLA_0.9.16

# Start the server
.\CarlaUE4.exe
```

The server will start on `localhost:2000` by default.

#### 2. Find Your Windows Host IP Address

From Windows PowerShell or Command Prompt:

```powershell
ipconfig /all
```

Look for your main network adapter's IPv4 address (e.g., `10.0.4.100`). This is the IP you'll use to connect from WSL.

**Note:** Do NOT use the WSL adapter IP or the gateway IP that you can get from `ip route` in WSL - use the actual Windows network adapter IP.

#### 3. Configure Windows Firewall

**This is critical!** Windows Firewall blocks incoming connections from WSL by default.

1. Press the **Windows key** and type **"Windows Defender Firewall with Advanced Security"**
2. Click on **Inbound Rules** in the left panel
3. Click **New Rule...** in the right panel
4. Select **Port** → Click **Next**
5. Select **TCP** and enter **Specific local ports**: `2000-2002`
   - Port 2000: RPC communication
   - Ports 2001-2002: Streaming sensor data
6. Click **Next** → Select **Allow the connection** → Click **Next**
7. Keep all checkboxes selected (Domain, Private, Public) → Click **Next**
8. Give the rule a name, e.g., **"CARLA Simulator WSL Access"**
9. Click **Finish**

#### 4. Connect from WSL/Docker

Now you can run your clients from WSL using the Windows IP:

```bash
# From the example directory
cd simple_client
./build-docker.sh

# Run the client
docker run --rm carla-simple-client:latest 10.0.4.100 2000
```

Replace `10.0.4.100` with your actual Windows IP address.

#### 5. Verify the Connection

Test if the port is accessible from WSL:

```bash
# Install netcat if needed
sudo apt-get install netcat

# Test connection (should not immediately fail if port is open)
nc -zv 10.0.4.100 2000
```

### Alternative: Running Everything on Linux

If you prefer to run both server and client on Linux:

```bash
# Download CARLA server for Linux
wget https://carla-releases.s3.eu-west-3.amazonaws.com/Linux/CARLA_0.9.16.tar.gz
tar -xzf CARLA_0.9.16.tar.gz
cd CARLA_0.9.16

# Start the server
./CarlaUE4.sh

# In another terminal, run your client
cd /path/to/libcarla/examples/simple_client
./build-docker.sh
docker run --rm --network host carla-simple-client:latest localhost 2000
```

## Troubleshooting

### Connection refused
- Make sure the CARLA server is running on the specified host and port
- Check firewall settings if connecting to a remote server
- For WSL-to-Windows connections, ensure you've configured the Windows Firewall as described above
- Verify you're using the correct Windows IP address (from `ipconfig`, not WSL's gateway IP)

### Version mismatch warnings
- LibCarla API version may differ from the CARLA server version
- These are usually just warnings and the client should still work
- For full compatibility, use matching versions of libcarla and CARLA server

### Port already in use
- Only one CARLA server can run on a given port
- Use `--rpc-port=XXXX` when starting CarlaUE4 to use a different port
- Update your client connection accordingly
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
