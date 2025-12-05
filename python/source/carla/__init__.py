"""
CARLA Python API

This package provides Python bindings for the CARLA simulator library.
"""

import os

__version__ = os.environ.get('LIBCARLA_VERSION', '0.9.16')

# Version information
VERSION_MAJOR = 0
VERSION_MINOR = 9
VERSION_PATCH = 16

__all__ = [
    '__version__',
    'VERSION_MAJOR',
    'VERSION_MINOR', 
    'VERSION_PATCH',
]
