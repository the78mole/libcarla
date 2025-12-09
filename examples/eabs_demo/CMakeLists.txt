cmake_minimum_required(VERSION 3.22)
project(eabs_demo)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LibCarla
find_package(libcarla REQUIRED)

# Find Mosquitto MQTT library
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

# Find Boost
find_package(Boost REQUIRED COMPONENTS system filesystem)

# Create executable
add_executable(eabs_demo
    cpp_eabs_datacarrier.cpp
)

# Link libraries
target_link_libraries(eabs_demo
    PRIVATE
        libcarla::carla_client
        ${MOSQUITTO_LIBRARIES}
        Boost::system
        Boost::filesystem
)

# Include directories
target_include_directories(eabs_demo
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${MOSQUITTO_INCLUDE_DIRS}
)

# Compiler flags
target_compile_options(eabs_demo
    PRIVATE
        ${MOSQUITTO_CFLAGS_OTHER}
)

# Link directories
target_link_directories(eabs_demo
    PRIVATE
        ${MOSQUITTO_LIBRARY_DIRS}
)

# Installation
install(TARGETS eabs_demo
    RUNTIME DESTINATION bin
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
