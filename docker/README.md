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
webserver/docker$ ./local_dockerfile_test.sh
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

## How to Run
After building the deployment Docker container, run it with the following command: 
```console
webserver$ docker run --rm -p 8080:80 --name webserver webserver:latest
```

Note: The first port specified is the port on the local machine, and the second port specified is the port in the Docker container to map it to. deploy.Dockerfile exposes port 80, so the second port specified should be 80.

The server will now serve requests from a local browser. For example, if the config specifies port 80, `http://<DockerIPv4Address>/` will display the home page.

The `IPv4Address` of the Docker container, e.g., `172.17.0.2`, can be found by running `docker network inspect bridge` from a local terminal outside of the Docker container.

## How to Debug Tests Failing in Docker Container
coverage.Dockerfile is intended for use with the remote hosting platform to catch potential issues before pushing to the live server. Normally, there is no reason to run it locally when `./coverage.sh` can be used. In the case where a test fails on the coverage Docker container but not on the local `./coverage.sh`, output files can be extracted with the following commands:
```console
webserver$ docker build -f docker/coverage.Dockerfile -t webserver:coverage .
webserver$ docker run -p 8080:80 --name coverage webserver:coverage
webserver$ docker cp coverage:/webserver/build_coverage/Testing/Temporary/LastTest.log <destination on host>
webserver$ docker cp coverage:/webserver/tests/integration/results.txt <destination on host>
```
Remember to remove the container afterwards!
```console
webserver$ docker rm coverage
```