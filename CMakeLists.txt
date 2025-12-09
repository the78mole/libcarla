cmake_minimum_required(VERSION 3.16)

# Set CMake policies
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)  # Use BoostConfig.cmake if available
endif()

# Set version from environment variable or default
if(DEFINED ENV{LIBCARLA_VERSION})
    set(LIBCARLA_VERSION_STRING $ENV{LIBCARLA_VERSION})
elseif(DEFINED LIBCARLA_VERSION)
    set(LIBCARLA_VERSION_STRING ${LIBCARLA_VERSION})
else()
    set(LIBCARLA_VERSION_STRING "0.9.16")
endif()

# Extract numeric version for CMake (strip any suffix like .devX)
string(REGEX REPLACE "\\.(dev|rc|alpha|beta)[0-9]*$" "" LIBCARLA_VERSION_NUMERIC "${LIBCARLA_VERSION_STRING}")

project(libcarla VERSION ${LIBCARLA_VERSION_NUMERIC})

# Store the full version string for use in code
set(LIBCARLA_FULL_VERSION "${LIBCARLA_VERSION_STRING}")

message(STATUS "Building LibCarla version: ${LIBCARLA_FULL_VERSION} (CMake version: ${PROJECT_VERSION})")

# C++ Standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Suppress Boost deprecation messages by defining the silence macro
add_compile_definitions(BOOST_ALLOW_DEPRECATED_HEADERS)

# Force static linking for self-contained builds
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

# Prefer static libraries when finding packages
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".so")
if(UNIX AND NOT APPLE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

# Build options
option(LIBCARLA_BUILD_RELEASE "Build release configuration" ON)
option(LIBCARLA_BUILD_DEBUG "Build debug configuration" OFF)
option(LIBCARLA_BUILD_PYTHON "Build Python bindings" OFF)

# Include FetchContent for external dependencies
include(FetchContent)

# Suppress warnings from external dependencies during CMake configuration
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "Disable deprecation warnings for external dependencies" FORCE)

# Fetch rpclib (provides msgpack)
FetchContent_Declare(
    rpclib
    GIT_REPOSITORY https://github.com/carla-simulator/rpclib.git
    GIT_TAG        v2.2.1_c5
    GIT_SHALLOW    TRUE
)

# Fetch Recast/Detour for navigation
FetchContent_Declare(
    recast
    GIT_REPOSITORY https://github.com/carla-simulator/recastnavigation.git
    GIT_TAG        carla
    GIT_SHALLOW    TRUE
)

# Set recast options
set(RECASTNAVIGATION_DEMO OFF CACHE BOOL "" FORCE)
set(RECASTNAVIGATION_TESTS OFF CACHE BOOL "" FORCE)
set(RECASTNAVIGATION_EXAMPLES OFF CACHE BOOL "" FORCE)

# Make dependencies available
FetchContent_MakeAvailable(rpclib recast)

# Re-enable deprecation warnings for our own project
set(CMAKE_WARN_DEPRECATED ON CACHE BOOL "Re-enable deprecation warnings for project code" FORCE)

# Find required packages - prefer static libraries
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost REQUIRED COMPONENTS filesystem system)

# For PNG, JPEG, ZLIB - try to find static versions
set(PNG_STATIC ON)
set(ZLIB_USE_STATIC_LIBS ON)
find_package(ZLIB REQUIRED)
find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(TIFF QUIET)

# Include directories
set(LIBCARLA_SOURCE_PATH "${CMAKE_SOURCE_DIR}/source")
set(LIBCARLA_THIRDPARTY_PATH "${LIBCARLA_SOURCE_PATH}/third-party")

include_directories(${LIBCARLA_SOURCE_PATH})
include_directories(${LIBCARLA_THIRDPARTY_PATH})
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
include_directories(SYSTEM ${rpclib_SOURCE_DIR}/include)

# Create recast include directory structure that LibCarla expects
set(RECAST_INCLUDE_DIR "${CMAKE_BINARY_DIR}/recast-include")
file(MAKE_DIRECTORY "${RECAST_INCLUDE_DIR}/recast")
file(GLOB RECAST_HEADERS "${recast_SOURCE_DIR}/Recast/Include/*.h")
file(GLOB DETOUR_HEADERS "${recast_SOURCE_DIR}/Detour/Include/*.h")
file(GLOB DETOURCROWD_HEADERS "${recast_SOURCE_DIR}/DetourCrowd/Include/*.h")
file(GLOB DETOURTILECACHE_HEADERS "${recast_SOURCE_DIR}/DetourTileCache/Include/*.h")

foreach(header ${RECAST_HEADERS} ${DETOUR_HEADERS} ${DETOURCROWD_HEADERS} ${DETOURTILECACHE_HEADERS})
    get_filename_component(header_name ${header} NAME)
    configure_file(${header} "${RECAST_INCLUDE_DIR}/recast/${header_name}" COPYONLY)
endforeach()

include_directories(SYSTEM ${RECAST_INCLUDE_DIR})

# Set CARLA_VERSION for Version.h template (use full version string)
set(CARLA_VERSION "${LIBCARLA_FULL_VERSION}")

# Generate Version.h
configure_file(
    ${LIBCARLA_SOURCE_PATH}/carla/Version.h.in
    ${LIBCARLA_SOURCE_PATH}/carla/Version.h
)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(
        -Wall
        -Wextra
        -Wpedantic
        -fPIC
    )
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O2 -DNDEBUG)
    endif()
endif()

add_compile_definitions(BOOST_ERROR_CODE_HEADER_ONLY)

# Collect source files (excluding pytorch and ros2 which require external dependencies)
file(GLOB_RECURSE LIBCARLA_SOURCES
    "${LIBCARLA_SOURCE_PATH}/carla/*.cpp"
    "${LIBCARLA_SOURCE_PATH}/carla/*.h"
)

# Remove pytorch and ros2 files from the sources
list(FILTER LIBCARLA_SOURCES EXCLUDE REGEX ".*/pytorch/.*")
list(FILTER LIBCARLA_SOURCES EXCLUDE REGEX ".*/ros2/.*")
list(FILTER LIBCARLA_SOURCES EXCLUDE REGEX ".*/rss/.*")

file(GLOB_RECURSE LIBCARLA_THIRDPARTY_SOURCES
    "${LIBCARLA_THIRDPARTY_PATH}/pugixml/*.cpp"
    "${LIBCARLA_THIRDPARTY_PATH}/odrSpiral/*.cpp"
)

# Create library
if(LIBCARLA_BUILD_RELEASE)
    add_library(carla_client STATIC
        ${LIBCARLA_SOURCES}
        ${LIBCARLA_THIRDPARTY_SOURCES}
    )

    target_include_directories(carla_client PUBLIC
        $<BUILD_INTERFACE:${LIBCARLA_SOURCE_PATH}>
        $<INSTALL_INTERFACE:include>
    )

    target_include_directories(carla_client SYSTEM PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${PNG_INCLUDE_DIRS}
        ${JPEG_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIRS}
        ${rpclib_SOURCE_DIR}/include
        ${RECAST_INCLUDE_DIR}
    )

    target_link_libraries(carla_client PUBLIC
        ${Boost_LIBRARIES}
        ${PNG_LIBRARIES}
        ${JPEG_LIBRARIES}
        ${ZLIB_LIBRARIES}
        rpc
        Recast
        Detour
        DetourCrowd
    )

    if(TIFF_FOUND)
        target_include_directories(carla_client SYSTEM PRIVATE ${TIFF_INCLUDE_DIRS})
        target_link_libraries(carla_client PUBLIC ${TIFF_LIBRARIES})
    endif()

    # Set properties for static library
    set_target_properties(carla_client PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    )
endif()

# Installation
include(GNUInstallDirs)

# Install main carla_client library
install(TARGETS carla_client
    EXPORT libcarla-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install dependency static libraries (rpclib, recast, detour)
install(TARGETS rpc Recast Detour DetourCrowd
    EXPORT libcarla-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# Install headers
install(DIRECTORY ${LIBCARLA_SOURCE_PATH}/carla
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

install(DIRECTORY ${LIBCARLA_THIRDPARTY_PATH}/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# Install rpclib headers
install(DIRECTORY ${rpclib_SOURCE_DIR}/include/rpc
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp" PATTERN "*.inl"
)

# Install recast headers  
install(DIRECTORY ${recast_SOURCE_DIR}/Recast/Include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/recast
    FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${recast_SOURCE_DIR}/Detour/Include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/recast
    FILES_MATCHING PATTERN "*.h"
)
install(DIRECTORY ${recast_SOURCE_DIR}/DetourCrowd/Include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/recast
    FILES_MATCHING PATTERN "*.h"
)

# Export targets
install(EXPORT libcarla-targets
    FILE libcarla-targets.cmake
    NAMESPACE libcarla::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libcarla
)

# Create config file
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_SOURCE_DIR}/cmake/libcarla-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/libcarla-config.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libcarla
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/libcarla-config-version.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/libcarla-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/libcarla-config-version.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/libcarla
)

# Copy headers to build directory for easy access
file(COPY ${LIBCARLA_SOURCE_PATH}/carla DESTINATION ${CMAKE_BINARY_DIR}/include)
