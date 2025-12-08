# Running CARLA Server

## Important Note

**CARLA requires a GPU for rendering.** The Unreal Engine 4 that CARLA is built on cannot run in true headless/software-only mode. You need one of the following:

1. **NVIDIA GPU with nvidia-docker** (Recommended for Docker)
2. **Native installation** on a system with GPU
3. **Cloud GPU instance** (AWS, Azure, GCP with GPU)

## Option 1: Docker with GPU (nvidia-docker)

### Prerequisites

Install NVIDIA Container Toolkit:

```bash
# Ubuntu/Debian
distribution=$(. /etc/os-release;echo $ID$VERSION_ID)
curl -s -L https://nvidia.github.io/nvidia-docker/gpgkey | sudo apt-key add -
curl -s -L https://nvidia.github.io/nvidia-docker/$distribution/nvidia-docker.list | \
  sudo tee /etc/apt/sources.list.d/nvidia-docker.list

sudo apt-get update
sudo apt-get install -y nvidia-container-toolkit
sudo systemctl restart docker
```

### Run CARLA Server

```bash
# Using official image
docker run --gpus all -p 2000:2000 \
  carlasim/carla:0.9.16 \
  ./CarlaUE4.sh -RenderOffScreen -quality-level=Low -carla-rpc-port=2000

# Or build and use the Dockerfile.server
docker build -f Dockerfile.server -t carla-server .
docker run --gpus all -p 2000:2000 carla-server
```

### Test with Client

```bash
# In another terminal
docker run --rm --network host \
  carla-simple-client:latest localhost 2000
```

## Option 2: Native Installation

### Download CARLA

```bash
# Download from official website
wget https://carla-releases.s3.us-east-005.backblazeb2.com/Linux/CARLA_0.9.16.tar.gz
tar -xzf CARLA_0.9.16.tar.gz
cd CARLA_0.9.16
```

### Run Server

```bash
# With display
./CarlaUE4.sh

# Headless (no window, but still needs GPU)
./CarlaUE4.sh -RenderOffScreen -quality-level=Low -nosound

# On a specific port
./CarlaUE4.sh -carla-rpc-port=2000
```

### Test with Client

```bash
# Using Docker client
docker run --rm --network host \
  carla-simple-client:latest localhost 2000

# Or native client
./simple_client localhost 2000
```

## Option 3: Docker Compose with GPU

Use the provided `docker-compose.yml`:

```bash
# Edit docker-compose.yml to ensure GPU is enabled
docker compose up
```

The compose file includes:
- CARLA server with GPU support
- Automatic health checks
- Network configuration
- Client container that waits for server

## Troubleshooting

### "GameThread timed out waiting for RenderThread"

This means CARLA cannot initialize the GPU renderer. Solutions:
- Ensure you have a GPU
- Use `--gpus all` flag with docker run
- Install nvidia-container-toolkit
- Check: `docker run --rm --gpus all ubuntu nvidia-smi`

### Container immediately exits

Check logs:
```bash
docker logs carla-server
```

Common issues:
- Missing `--gpus all` flag
- No GPU available
- nvidia-docker not installed

### Port already in use

```bash
# Find and stop conflicting process
sudo lsof -i :2000
docker stop $(docker ps -q --filter "expose=2000")
```

## Performance Tips

1. **Use lower quality**: `-quality-level=Low`
2. **Disable rendering**: `-RenderOffScreen` (still needs GPU)
3. **Disable sound**: `-nosound`
4. **Use fixed time-step**: `-benchmark -fps=10`

## Cloud GPU Options

### AWS EC2
- Instance type: g4dn.xlarge or higher
- AMI: Deep Learning AMI with NVIDIA drivers

### Google Cloud
- Instance type: n1-standard-4 with NVIDIA T4
- Image: Deep Learning VM with GPU support

### Azure
- Instance type: NC-series or NV-series
- Image: Data Science VM with GPU

All require nvidia-docker installation after VM creation.
