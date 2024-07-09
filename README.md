# Max's Personal Web Server
This repository contains the source code and documentation for my personal web server, which I am using to host my personal website.

All console commands given in this document assume Arch Linux kernel 6.9.6-arch1-1.

## Web Server Dependencies (required)

#### cmake (version >= 3.30.0 required, cmake-3.30.0-1 used)
```console
$ sudo pacman -S cmake
```

#### Boost C++ libraries (version >= 1.70 required, boost-1.83.0-9 used)
```console
$ sudo pacman -S boost
```

## How to Build
1. Install required dependencies from previous section.
2. Clone this repository.
```console
$ git clone https://github.com/md842/webserver
```
3. Navigate to the project directory and run the build script.
```console
$ cd webserver
webserver$ ./build.sh
```
The build script invokes the following commands to perform an out-of-source build using cmake.
```console
webserver$ mkdir build
webserver$ cd build
webserver/build$ cmake ..
webserver/build$ make
```

## How to Test
Navigate to the project directory and run the test script.
```console
$ cd webserver
webserver$ ./ctest.sh
```

The test script builds the project, then invokes the following command to run all unit and integration tests.
```console
webserver/build$ ctest
```

To run the integration tests only, use the following commands:
```console
webserver$ cd tests/integration
webserver/tests/integration$ ./integration_tests.sh
```
The detailed results of the integration tests are written to `webserver/tests/integration/results.txt`.

## How to Run
After building the server, start it using the run script, which uses the default config.
```console
webserver$ ./run.sh
```

The server can be started manually with the following command. Any config file may be used, this is just an example:
```console
webserver$ ./build/bin/server configs/local_config.conf
```

The server will now serve requests from a local browser. For example, if the config specifies port 8080, `http://localhost:8080/` will display the home page. 

If the server is running within a Docker container, replace `localhost` with the `IPv4Address` of the Docker container, e.g., `172.17.0.2`. The address can be found by running `docker network inspect bridge` from a local terminal outside of the Docker container. For example, if the config specifies port 8080, visiting `http://172.17.0.2:8080/` will display the home page. 

## Coverage Reporting Dependencies (optional)

#### gcovr (gcovr-7.2-2 used)
```console
$ sudo pacman -S gcovr
```

#### Python (python-3.12.4-1 used)
```console
$ sudo pacman -S python
```

#### pygments (Optional, used for syntax highlighting. python-pygments-2.17.2-3 used)
```console
$ sudo pacman -S python-pygments
```

## How to Generate a Coverage Report
Navigate to the project directory and run the coverage build script.
```console
$ cd webserver
webserver$ ./coverage.sh
```
The coverage build script invokes the following commands to perform an out-of-source coverage build and generate a coverage report using cmake.
```console
webserver$ mkdir build_coverage
webserver$ cd build_coverage
webserver/build_coverage$ cmake -DCMAKE_BUILD_TYPE=Coverage ..
webserver/build_coverage$ make coverage
```
The detailed coverage report is written to `webserver/build_coverage/report/index.html`.