# LibCarla Python Package

This package provides Python bindings for the CARLA simulator library (LibCarla).

## Installation

```bash
pip install carla
```

## Usage

```python
import carla

print(f"CARLA version: {carla.__version__}")
```

## Building from Source

```bash
cd python
pip install build
python -m build --wheel
```

## License

MIT License - see the LICENSE file for details.
