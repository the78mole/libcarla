#!/bin/bash
# Script to extract build dependencies from CMakeLists.txt
# Usage: extract-cmake-deps.sh <path-to-CMakeLists.txt>

CMAKE_FILE="$1"

if [ ! -f "$CMAKE_FILE" ]; then
    echo "Error: CMakeLists.txt not found at $CMAKE_FILE" >&2
    exit 1
fi

PACKAGES=""

# Extract Boost components
BOOST_COMPONENTS=$(grep -oP 'find_package\(Boost[^)]*COMPONENTS\s+\K[^)]+' "$CMAKE_FILE" | tr '\n' ' ')
if [ -n "$BOOST_COMPONENTS" ]; then
    for component in $BOOST_COMPONENTS; do
        # Boost packages follow the pattern libboost-<component>-dev
        PACKAGES="$PACKAGES libboost-${component}-dev"
    done
fi

# Extract pkg-config modules
PKG_MODULES=$(grep -oP 'pkg_check_modules\([^)]*REQUIRED\s+\K[^)]+' "$CMAKE_FILE" | tr '\n' ' ')
if [ -n "$PKG_MODULES" ]; then
    for module in $PKG_MODULES; do
        # Try to find the package using apt-cache
        if apt-cache search --names-only "^${module}-dev$" 2>/dev/null | grep -q "${module}-dev"; then
            PACKAGES="$PACKAGES ${module}-dev"
        elif apt-cache search --names-only "^lib${module}-dev$" 2>/dev/null | grep -q "lib${module}-dev"; then
            PACKAGES="$PACKAGES lib${module}-dev"
        else
            # Fallback: assume lib prefix
            PACKAGES="$PACKAGES lib${module}-dev"
        fi
    done
fi

# Extract other find_package calls (excluding our own and common infrastructure)
OTHER_PACKAGES=$(grep -oP 'find_package\(\K[A-Za-z0-9_]+' "$CMAKE_FILE" | grep -v -E '^(libcarla|Boost|PkgConfig|CMake)$' | tr '\n' ' ')
if [ -n "$OTHER_PACKAGES" ]; then
    for pkg in $OTHER_PACKAGES; do
        # Convert to lowercase for apt search
        pkg_lower=$(echo "$pkg" | tr '[:upper:]' '[:lower:]')
        
        # Try different naming patterns
        if apt-cache search --names-only "^${pkg_lower}-dev$" 2>/dev/null | grep -q "${pkg_lower}-dev"; then
            PACKAGES="$PACKAGES ${pkg_lower}-dev"
        elif apt-cache search --names-only "^lib${pkg_lower}-dev$" 2>/dev/null | grep -q "lib${pkg_lower}-dev"; then
            PACKAGES="$PACKAGES lib${pkg_lower}-dev"
        fi
    done
fi

# Remove duplicates and extra spaces
PACKAGES=$(echo "$PACKAGES" | tr ' ' '\n' | sort -u | grep -v '^$' | tr '\n' ' ')

echo "$PACKAGES"
