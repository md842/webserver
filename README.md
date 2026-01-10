# Max's Personal Web Server
This repository contains the source code and documentation for my personal web server, which I am using to host my personal website.

![Coverage](https://img.shields.io/badge/Test_Coverage-98.94%25-brightgreen)

## Building From Source

1. Install required build dependencies:
    - C++ compiler (version >= C++17)
    - Boost C++ libraries (version >= 1.87, required components: context, log, process)
    - CMake (version >= 3.30.0)
    - OpenSSL development libraries (version >= 3.0.0)

    ### Arch Linux
    ```console
    $ pacman -S gcc boost cmake openssl
    ```

    ### Debian
    **Note:** At the time of writing, `libboost-process1.88-dev` is only available with `debian:forky` or later.
    ```console
    $ apt-get install g++ cmake \
        libboost-context1.88-dev \
        libboost-log1.88-dev \
        libboost-process1.88-dev \
        libssl-dev
    ```

2. Clone this repository.
    ```console
    $ git clone git@github.com:md842/webserver.git
    ```

3. A build script `build.sh` is provided which takes the CMake build type as an optional argument. For an optimized production binary without additional dependencies, specify build type "Release".
    ```console
    $ cd webserver
    webserver$ ./build.sh Release
    ```

    **Note:** If no argument is specified, `build.sh` defaults to Debug build type, which will fail without additional testing dependencies installed (See section "Running Unit and Integration Tests").


## Running the Web Server

1. Follow the steps in the previous section to build the web server binary.

2. A run script `run.sh` is provided which runs the web server binary with the config file `configs/local_config.conf`.
    ```console
    webserver$ ./run.sh
    ```
    ---
    Optionally, start the web server with a different config file using the following command:
    ```console
    webserver$ ./build/bin/server <path/to/config.conf>
    ```
---
The web server will now serve requests from a local browser. For example, if the config's `listen` directive specifies `8080 ssl`, `https://localhost:8080/` will display the home page.

**Note:** The web server serves files from the path(s) specified by the config's `root` directive(s). For `configs/local_config.conf`, the root path is `../personal-website/build`. The home page will display Internal Server Error (Error 500) if no front end is present at the location specified by the config. This is expected behavior; `configs/local_config.conf` is configured to serve a React Router page, which serves the index page as the 404 fallback instead of having a "real" 404 fallback. Thus, if the index page is not present, the server returns Error 500 instead of Error 404.


## Running Unit and Integration Tests (optional)

1. Install required testing dependencies.
    - curl, netcat (used during integration testing)
    - GoogleTest (version >= 1.17.0, unit testing framework)

    ### Arch Linux
    ```console
    $ pacman -S curl gtest netcat
    ```

    ### Debian
    **Note:** At the time of writing, `libgtest-dev` version 1.17 is only available with `debian:forky` or later.
    ```console
    $ apt-get install curl libgtest-dev netcat-openbsd
    ```

2. A test script `ctest.sh` is provided which builds the project with Debug build type, then invokes `ctest` to run all unit and integration tests.
    ```console
    $ cd webserver
    webserver$ ./ctest.sh
    ```
    ---
    Optionally, to run integration tests only, navigate to `webserver/tests/integration` and run the integration test script `integration_tests.sh`.
    ```console
    $ cd webserver/tests/integration
    webserver/tests/integration$ ./integration_tests.sh
    ```

    The detailed results of the integration tests are written to `webserver/tests/integration/last_test_result.txt`.


## Generating a Coverage Report (optional)

1. Install required dependency `gcovr`.
    ### Arch Linux
    ```console
    $ pacman -S gcovr
    ```

    ### Debian/Ubuntu
    ```console
    $ apt-get install gcovr
    ```

2. A coverage report script `coverage.sh` is provided which builds the project with Coverage build type, then invokes `make coverage` to run all unit and integration tests and generate a coverage report.
    ```console
    $ cd webserver
    webserver$ ./coverage.sh
    ```

    The detailed coverage report is written to `webserver/build_coverage/report/index.html`.