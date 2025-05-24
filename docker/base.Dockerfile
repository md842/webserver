# Stage 1: Base image stage.
FROM ubuntu:questing AS base

# Update base image and install necessary components for our build environment.
# ARG DEBIAN_FRONTEND=noninteractive allows unattended package installation.
ARG DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    curl \
    gcovr \
    libgmock-dev \
    libgtest-dev \
    netcat-traditional

# There is no package for ubuntu:questing with a working Boost 1.88 that
# includes Boost::process, so we must build the required libraries ourselves.
# This takes a while, but luckily is cached and shouldn't change too often.
RUN curl https://archives.boost.io/release/1.88.0/source/boost_1_88_0.tar.gz -o boost_1_88_0.tar.gz
RUN tar xf boost_1_88_0.tar.gz
WORKDIR boost_1_88_0
RUN ./bootstrap.sh
# Build and install Boost::process only, debug level 0 to reduce output
RUN ./b2 -d0 --with-process
RUN ./b2 -d0 install