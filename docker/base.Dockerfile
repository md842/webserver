# Stage 1: Prepare build environment
FROM debian:forky-slim AS base

# Update base image and install necessary components for the build environment.
# ARG DEBIAN_FRONTEND=noninteractive allows unattended package installation.
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    curl \
    gcovr \
    libboost-context1.88-dev \
    libboost-log1.88-dev \
    libboost-process1.88-dev \
    libgtest-dev \
    libssl-dev \
    netcat-openbsd

# Copy local build context to webserver directory within container
COPY . /webserver