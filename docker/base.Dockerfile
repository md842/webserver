# Stage 1: Base image stage.
FROM ubuntu:questing AS base

# Update base image and install necessary components for our build environment.
# ARG DEBIAN_FRONTEND=noninteractive allows unattended package installation.
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    curl \
    libboost1.88-dev \
    libgmock-dev \
    libgtest-dev \
    netcat-traditional \
    gcovr