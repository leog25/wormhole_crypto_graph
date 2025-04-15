# Build stage
FROM ubuntu:22.04 as builder

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
    python3 \
    libssl-dev \
    zlib1g-dev \
    && rm -rf /var/lib/apt/lists/*

# Install vcpkg
WORKDIR /opt
RUN git clone https://github.com/Microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh

# Install required packages with vcpkg
RUN ./vcpkg/vcpkg install cpr:x64-linux && \
    ./vcpkg/vcpkg install nlohmann-json:x64-linux && \
    ./vcpkg/vcpkg install openssl:x64-linux && \
    ./vcpkg/vcpkg install zlib:x64-linux && \
    ./vcpkg/vcpkg install uwebsockets:x64-linux

# Set working directory for the application
WORKDIR /app

# Copy source files
COPY backend/ .

# Build the application using vcpkg toolchain
RUN cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
    -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build

# Runtime stage
FROM ubuntu:22.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    zlib1g \
    && rm -rf /var/lib/apt/lists/*

# Create non-root user
RUN useradd -m -u 1000 appuser

# Copy built application from builder stage
COPY --from=builder /app/build/wormhole_tracker /app/wormhole_tracker
COPY --from=builder /opt/vcpkg/installed/x64-linux/lib/* /usr/local/lib/

# Set proper permissions
RUN chown -R appuser:appuser /app

# Switch to non-root user
USER appuser

# Set working directory
WORKDIR /app

# Expose the WebSocket port
EXPOSE 9001

# Health check
HEALTHCHECK --interval=30s --timeout=3s \
    CMD curl -f http://localhost:9001/health || exit 1

# Run the application
CMD ["./wormhole_tracker"] 