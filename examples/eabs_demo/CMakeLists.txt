cmake_minimum_required(VERSION 3.22)
project(eabs_demo)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Mosquitto MQTT library
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system filesystem)

# Try to find libcarla package config first
find_package(libcarla QUIET)

if(libcarla_FOUND)
    message(STATUS "Found libcarla package")
    set(LIBCARLA_LIBRARIES libcarla::carla_client)
else()
    message(STATUS "Using system-installed libcarla")
    set(LIBCARLA_INCLUDE_DIRS /usr/include/carla)
    
    find_library(LIBCARLA_CLIENT_LIBRARY NAMES carla_client REQUIRED)
    find_library(RPC_LIBRARY NAMES rpc REQUIRED)
    find_library(RECAST_LIBRARY NAMES Recast REQUIRED)
    find_library(DETOUR_LIBRARY NAMES Detour REQUIRED)
    find_library(DETOURCROWD_LIBRARY NAMES DetourCrowd REQUIRED)
    
    if(NOT LIBCARLA_CLIENT_LIBRARY)
        message(FATAL_ERROR "Could not find carla_client library")
    endif()
    
    set(LIBCARLA_LIBRARIES 
        ${LIBCARLA_CLIENT_LIBRARY}
        ${RPC_LIBRARY}
        ${RECAST_LIBRARY}
        ${DETOUR_LIBRARY}
        ${DETOURCROWD_LIBRARY}
    )
endif()

# Create executables
add_executable(eabs_demo
    cpp_eabs_datacarrier.cpp
    config.cpp
)

add_executable(eabs_demo_nonrt
    cpp_eabs_datacarrier_legacy.cpp
    config.cpp
)

# Link libraries
if(libcarla_FOUND)
    # When using CMake package, use IMPORTED targets
    target_link_libraries(eabs_demo
        PRIVATE
            ${LIBCARLA_LIBRARIES}
            ${MOSQUITTO_LIBRARIES}
            Boost::system
            Boost::filesystem
            pthread
    )
    
    target_link_libraries(eabs_demo_nonrt
        PRIVATE
            ${LIBCARLA_LIBRARIES}
            ${MOSQUITTO_LIBRARIES}
            Boost::system
            Boost::filesystem
            pthread
    )
else()
    # When using system libraries, link explicitly with IMPORTED targets
    # to ensure we link against the same Boost version as libcarla
    target_include_directories(eabs_demo PRIVATE ${LIBCARLA_INCLUDE_DIRS})
    target_include_directories(eabs_demo PRIVATE ${MOSQUITTO_INCLUDE_DIRS})
    
    target_link_libraries(eabs_demo
        PRIVATE
            ${LIBCARLA_LIBRARIES}
            ${MOSQUITTO_LIBRARIES}
            Boost::system
            Boost::filesystem
            pthread
    )
    
    target_link_libraries(eabs_demo_nonrt
        PRIVATE
            ${LIBCARLA_LIBRARIES}
            ${MOSQUITTO_LIBRARIES}
            Boost::system
            Boost::filesystem
            pthread
    )
endif()

# Include directories for both executables
target_include_directories(eabs_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${MOSQUITTO_INCLUDE_DIRS}
)

target_include_directories(eabs_demo_nonrt
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${MOSQUITTO_INCLUDE_DIRS}
)

# Compiler flags for both executables
target_compile_options(eabs_demo
    PRIVATE
        ${MOSQUITTO_CFLAGS_OTHER}
)

target_compile_options(eabs_demo_nonrt
    PRIVATE
        ${MOSQUITTO_CFLAGS_OTHER}
)

# Link directories for both executables
target_link_directories(eabs_demo
    PRIVATE
        ${MOSQUITTO_LIBRARY_DIRS}
)

target_link_directories(eabs_demo_nonrt
    PRIVATE
        ${MOSQUITTO_LIBRARY_DIRS}
)

# Installation
install(TARGETS eabs_demo eabs_demo_nonrt
    RUNTIME DESTINATION bin
)

# Install configuration file
install(FILES eabs_demo.ini
    DESTINATION etc
)

# Install README if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
    install(FILES README.md
        DESTINATION share/doc/libcarla-example-eabs-demo
    )
endif()

# CPack configuration for Debian package
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "libcarla-example-eabs-demo")
set(CPACK_PACKAGE_VERSION "0.0.19")
set(CPACK_PACKAGE_CONTACT "the78mole <the78mole@users.noreply.github.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "LibCarla Example - EABS Demo")
set(CPACK_PACKAGE_DESCRIPTION "Emergency Automatic Braking System demonstration application using LibCarla client library and MQTT metrics.")
set(CPACK_DEBIAN_PACKAGE_SECTION "devel")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libcarla (>= 0.0.19), libboost-filesystem1.74.0 | libboost-filesystem1.83.0, libboost-system1.74.0 | libboost-system1.83.0, libmosquitto1")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://github.com/the78mole/libcarla")

# Set package file name
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}-${CMAKE_SYSTEM_NAME}_${CMAKE_SYSTEM_PROCESSOR}")

include(CPack)
