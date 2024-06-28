# Max's Personal Web Server
This repository contains the source code and documentation for my personal web server, which I am using to host my personal website.

## Dependencies

#### cmake
Arch Linux:
```console
$ sudo pacman -S cmake
```

#### Boost C++ libraries (version >= 1.50)
Arch Linux:
```console
$ sudo pacman -S boost
```

## Build Instructions
1. Install dependencies from previous section if not already installed.
2. Clone this repository.
```console
$ git clone https://github.com/md842/webserver
```
3. Navigate to the project directory. Create a build directory and navigate to it.
```console
$ cd webserver
webserver$ mkdir build
webserver$ cd build
```
4. Perform an out-of-source build using cmake.
```console
webserver/build$ cmake ..
webserver/build$ make
```

## Testing Instructions

After building the server, start it with the following command. Any config file may be used, this is just an example:
```console
webserver/build$ ./bin/server ../configs/server_config.conf
```

The server may now be tested with a local browser. For example, `http://localhost:80/` will display the home page. 

If the server is running within a Docker container, replace `localhost` with the `IPv4Address` of the Docker container, e.g., `172.17.0.2`. The address can be found by running `docker network inspect bridge` from a local terminal outside of the Docker container. For example, visiting `http://172.17.0.2:80/` will display the home page. 