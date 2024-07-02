#! /bin/bash
mkdir build_coverage
cd build_coverage
cmake -DCMAKE_BUILD_TYPE=Coverage ..
make coverage