FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    zip \
    unzip \
    tar \
    pkg-config \
    ninja-build \
    python3

# Install vcpkg
WORKDIR /opt
RUN git clone https://github.com/Microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh

# Install required packages with vcpkg
RUN ./vcpkg/vcpkg install cpr:x64-linux && \
    ./vcpkg/vcpkg install nlohmann-json:x64-linux && \
    ./vcpkg/vcpkg install openssl:x64-linux && \
    ./vcpkg/vcpkg install zlib:x64-linux && \
    ./vcpkg/vcpkg install usockets:x64-linux

# Set working directory for the application
WORKDIR /app

# Copy source files
COPY . .

# Build the application using vcpkg toolchain
RUN cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# Expose the WebSocket port
EXPOSE 9001

# Run the application
CMD ["./build/wormhole_tracker"] 