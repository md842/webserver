# Stage 1: Build base image (used as build environment for stages 2, 3)
FROM archlinux AS base

# Update base image and install necessary components for our build environment.
# pacman --noconfirm option allows unattended package installation.
RUN pacman -Syu --noconfirm && pacman -S --noconfirm \
    base-devel \
    boost \
    cmake \
    curl \
    gcovr \
    gtest \
    netcat \
    openssl