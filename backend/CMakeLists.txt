cmake_minimum_required(VERSION 3.10)

# Don't hardcode the vcpkg path, it will be provided during build
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

project(wormhole_tracker)

message(STATUS "CMAKE_TOOLCHAIN_FILE = ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "Looking for CPR...")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Release)

# Find required packages
find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# Add include directories for uWebSockets and uSockets
include_directories(${CMAKE_PREFIX_PATH}/include)

# Find uSockets library
find_library(USOCKETS_LIB uSockets PATHS "${CMAKE_PREFIX_PATH}/lib" REQUIRED)

# Add your executable
add_executable(wormhole_tracker main.cpp)

# Link libraries
target_link_libraries(wormhole_tracker PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    ${USOCKETS_LIB}
    pthread
)