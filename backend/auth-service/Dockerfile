# Multi-stage build for smaller production image
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libssl-dev \
    libpq-dev \
    nlohmann-json3-dev \
    && rm -rf /var/lib/apt/lists/*

# Install Crow framework
RUN git clone https://github.com/CrowCpp/Crow.git /tmp/crow && \
    cd /tmp/crow && \
    mkdir build && cd build && \
    cmake .. -DCROW_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc) && \
    make install && \
    ldconfig

# Copy source code
WORKDIR /app
COPY backend/common ./backend/common
COPY backend/auth-service ./backend/auth-service

# Build the application
WORKDIR /app/backend/auth-service
RUN mkdir build && cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release && \
    make -j$(nproc)

# Production stage
FROM ubuntu:22.04 AS production

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libssl3 \
    libpq5 \
    && rm -rf /var/lib/apt/lists/*

# Create app user
RUN groupadd -r ummah && useradd -r -g ummah ummah

# Copy binary
COPY --from=builder /app/backend/auth-service/build/ummah_auth_service /usr/local/bin/

# Set ownership
RUN chown ummah:ummah /usr/local/bin/ummah_auth_service

# Switch to app user
USER ummah

# Expose port
EXPOSE 8080

# Health check
HEALTHCHECK --interval=30s --timeout=5s --start-period=5s --retries=3 \
    CMD curl -f http://localhost:8080/health || exit 1

# Set environment variables
ENV API_PORT=8080
ENV API_HOST=0.0.0.0

# Run the application
CMD ["/usr/local/bin/ummah_auth_service"]