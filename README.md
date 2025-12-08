# LibCarla APT Repository

## Installation

1. **Add the repository GPG key:**

```bash
curl -fsSL https://the78mole.github.io/libcarla/public.key | sudo gpg --dearmor -o /usr/share/keyrings/libcarla-archive-keyring.gpg
```

2. **Add the repository:**

```bash
echo "deb [arch=amd64,arm64 signed-by=/usr/share/keyrings/libcarla-archive-keyring.gpg] https://the78mole.github.io/libcarla stable main" | sudo tee /etc/apt/sources.list.d/libcarla.list
```

3. **Update and install:**

```bash
sudo apt-get update
sudo apt-get install libcarla
```

## Available Packages

- **libcarla** - CARLA client library with headers and development files

## Supported Distributions

- Ubuntu 22.04 (amd64, arm64)
- Ubuntu 24.04 (amd64, arm64)

## Repository Information

- **Origin:** the78mole/libcarla
- **Components:** main
- **Architectures:** amd64, arm64

## Manual Download

You can also download .deb packages directly from the [GitHub Releases](https://github.com/the78mole/libcarla/releases) page.
