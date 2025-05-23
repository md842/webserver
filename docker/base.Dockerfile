# Stage 1: Base image stage.
FROM ubuntu:oracular AS base

# Update base image and install necessary components for our build environment.
# ARG DEBIAN_FRONTEND=noninteractive allows unattended package installation.
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    boost \
    build-essential \
    cmake \
    curl \
    libboost-log-dev \
    libboost-system-dev \
    libgmock-dev \
    libgtest-dev \
    netcat-traditional \
    gcovr