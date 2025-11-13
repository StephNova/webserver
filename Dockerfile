# Use Debian as base image for better compatibility
FROM debian:bullseye-slim

# Install dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    php-cgi \
    python3 \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy source code
COPY . .

# Build the webserver
RUN make

# Expose ports (8080 and 9090 based on default config)
EXPOSE 8080 9090

# Run the webserver with Docker-specific configuration (uses 0.0.0.0 instead of 127.0.0.1)
CMD ["./webserv", "server.docker.conf"]

