# Max's Personal Web Server: Docker Container

All console commands given in this document are for Arch Linux kernel 6.9.6-arch1-1 and assume you have already cloned the top level repository.

## Dependencies (required)

#### Docker (docker-1:28.1.1-1 used)
```console
$ sudo pacman -S docker
```

#### Docker Buildx (docker-buildx-0.23.0-1 used)
```console
$ sudo pacman -S docker-buildx
```

## How to Build the Multi-Stage Docker Container
1. Install required dependencies from previous section.
2. Navigate to the Dockerfile directory and run the build script.
```console
$ cd webserver/docker
webserver/docker$ ./build_test_docker.sh
```
The build script invokes the following commands to build the multi-stage Docker container.
```console
webserver/docker$ sudo systemctl start docker
webserver/docker$ cd ..
webserver$ docker build -f docker/base.Dockerfile -t webserver:base .
webserver$ docker build -f docker/build.Dockerfile -t webserver:build .
webserver$ docker build -f docker/coverage.Dockerfile .
webserver$ docker build -f docker/deploy.Dockerfile -t webserver:latest .
```

## How to Test
After building the Docker container in the previous section, run the test script in the same directory: 
```console
webserver/docker$ ./run_test_docker.sh
```

The test script invokes the following command to mount certificate files from the host system to the container's file system, map ports from the local machine to the Docker container, then run the Docker container.
```console
webserver$ docker run --rm \
    -v [path to certificate files on host]:/etc/letsencrypt:ro \
    -p 8080:80 -p 8081:443 \
    --name webserver webserver:latest
```

Note: `[path to certificate files on host]` should be replaced with the actual directory in which certificate files are located on the host machine. I cannot provide my production certificate files in this repository for obvious security reasons, but self-signed certificates such as the ones located at `./tests/certs/` are sufficient for testing.

The server will now serve requests from a local browser. For example, if the config specifies port 443, `https://<DockerIPv4Address>/` will display the home page. There may be a certificate error, as the IPv4 address of the Docker container may not match the host name of the certificate. In order to test file serving from the Docker container, the certificate error can be ignored.

The `IPv4Address` of the Docker container, e.g., `172.17.0.2`, can be found by running `docker network inspect bridge` from a local terminal outside of the Docker container.

## How to Debug Tests Failing in Docker Container
coverage.Dockerfile is intended for use with the remote hosting platform to catch potential issues before pushing to the live server. Normally, there is no reason to run it locally when `./coverage.sh` can be used. In the case where a test fails on the coverage Docker container but not on the local `./coverage.sh`, output files can be extracted with the following commands:
```console
webserver$ docker build -f docker/coverage.Dockerfile -t webserver:coverage .
webserver$ docker run -p 8080:80 --name coverage webserver:coverage
webserver$ docker cp coverage:/webserver/build_coverage/Testing/Temporary/LastTest.log <destination on host>
webserver$ docker cp coverage:/webserver/tests/integration/last_test_result.txt <destination on host>
```
Remember to remove the container afterwards!
```console
webserver$ docker rm coverage
```