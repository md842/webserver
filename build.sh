#! /bin/bash
# A script that automates build commands for convenience while testing.
mkdir build
cd build
if [ $# -eq 1 ] # Build type provided as argument
then
    cmake -DCMAKE_BUILD_TYPE=$1 .. # Build with specified build type
else # 0 arguments provided
    cmake .. # Defaults to Debug build type
fi
make