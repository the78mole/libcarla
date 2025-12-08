cmake_minimum_required(VERSION 3.22)
project(libcarla-examples)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost
find_package(Boost REQUIRED COMPONENTS filesystem system)

# Try to find libcarla package config first
find_package(libcarla QUIET)

if(libcarla_FOUND)
    message(STATUS "Found libcarla package")
    # Use the package targets
    set(LIBCARLA_LIBRARIES carla::client)
else()
    message(STATUS "Using system-installed libcarla")
    # Use system-installed libcarla (from .deb package)
    set(LIBCARLA_INCLUDE_DIRS /usr/include/carla)
    
    # Find libraries in system directories
    find_library(LIBCARLA_CLIENT_LIBRARY NAMES carla_client REQUIRED)
    find_library(RPC_LIBRARY NAMES rpc REQUIRED)
    find_library(RECAST_LIBRARY NAMES Recast REQUIRED)
    find_library(DETOUR_LIBRARY NAMES Detour REQUIRED)
    find_library(DETOURCROWD_LIBRARY NAMES DetourCrowd REQUIRED)
    
    if(NOT LIBCARLA_CLIENT_LIBRARY)
        message(FATAL_ERROR "Could not find carla_client library")
    endif()
    
    message(STATUS "Found libcarla_client: ${LIBCARLA_CLIENT_LIBRARY}")
    message(STATUS "Found rpc: ${RPC_LIBRARY}")
    message(STATUS "Found Recast: ${RECAST_LIBRARY}")
    message(STATUS "Found Detour: ${DETOUR_LIBRARY}")
    message(STATUS "Found DetourCrowd: ${DETOURCROWD_LIBRARY}")
    
    set(LIBCARLA_LIBRARIES 
        ${LIBCARLA_CLIENT_LIBRARY}
        ${RPC_LIBRARY}
        ${RECAST_LIBRARY}
        ${DETOUR_LIBRARY}
        ${DETOURCROWD_LIBRARY}
    )
endif()

# Simple client example
add_executable(simple_client main.cpp)

if(libcarla_FOUND)
    target_link_libraries(simple_client
        PRIVATE
            ${LIBCARLA_LIBRARIES}
            Boost::filesystem
            Boost::system
    )
else()
    target_include_directories(simple_client PRIVATE ${LIBCARLA_INCLUDE_DIRS})
    target_link_libraries(simple_client
        PRIVATE
            ${LIBCARLA_LIBRARIES}
            Boost::filesystem
            Boost::system
            pthread
    )
endif()

# Install the example
install(TARGETS simple_client
    RUNTIME DESTINATION bin
)
