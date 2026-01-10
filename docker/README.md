# Max's Personal Web Server: Docker Image
This document assumes you have already cloned the top level `webserver` repository.


## Building the Docker Image From Source

1. Install required build dependencies `docker` and `docker-buildx`.

    ### Arch Linux
    ```console
    $ pacman -S docker docker-buildx
    ```

    ### Debian
    ```console
    $ apt-get install docker docker-buildx
    ```

2. A build script `build_test_docker.sh` is provided which builds the multi-stage Docker image and assigns the tag `webserver:latest` to the final stage. It also tags intermediate stages `webserver:base` (which contains the build environment) and `webserver:build` (which contains the production backend binary, but no frontend). The config file used at the container entrypoint (specified in `deploy.Dockerfile`) is `../configs/production_config.conf` by default.
    ```console
    $ cd webserver/docker
    webserver/docker$ ./build_test_docker.sh
    ```
    
    ---
    
    Optionally, after building `webserver:latest` at least once, `redeploy_test_docker.sh` can be used to test the fast frontend redeploy feature which skips the backend binary build/test process.
    ```console
    $ cd webserver/docker
    webserver/docker$ ./redeploy_test_docker.sh
    ```


## Running the Web Server Docker Container

1. Follow the steps in the previous section to build the `webserver:latest` Docker image.

2. A run script `run_test_docker.sh` is provided.
    ```console
    webserver/docker$ ./run_test_docker.sh
    ```

    By default, the run script mounts HTTPS certificate files from `../../certbot/letsencrypt` on the host system to `/etc/letsencrypt` on the Docker container's file system, maps ports `8080` and `8081` from the host machine to ports `80` and `443` respectively on the Docker container, then runs the Docker container. 
    ```console
    webserver/docker$ docker run --rm \
        -v ../../certbot/letsencrypt:/etc/letsencrypt:ro \
        -p 8080:80 -p 8081:443 \
        --name webserver webserver:latest
    ```

    The run script may require modification depending on the host machine environment.
    ```console
    webserver/docker$ docker run --rm \
        -v <path/to/host/cert/dir>:<path/to/config/ssl_certificate/dir>:ro \
        -p <host port>:<container port> ... \
        --name webserver webserver:latest
    ```

    1. Set the port argument(s) `-p` as desired; the left side of the `:` specifies a port on the host machine, and the right side is the mapped port on the Docker container.

    2. If the entrypoint config file specified in `deploy.Dockerfile` (default `../configs/production_config.conf`) does not include an HTTPS server, the entire line containing the `-v` argument may be removed. Otherwise, replace `<path/to/host/cert/dir>` with a directory on the host machine in which certificate files are located, and `<path/to/config/ssl_certificate/dir>` with the parent directory of the `ssl_certificate` and `ssl_certificate_key` directives in the config file.

    **Note:** I cannot provide my production HTTPS certificate files in this repository for obvious security reasons, but self-signed certificates such as the ones located at `../tests/certs/` are sufficient for testing. 
---
The web server will now serve requests from a local browser. For example, if the config's `listen` directive specifies `443 ssl`, `https://<DockerIPv4Address>/` will display the home page. There may be a certificate error, as the IPv4 address of the Docker container may not match the host name of the certificate. In order to test file serving from the Docker container, the certificate error can be ignored.

The `IPv4Address` of the Docker container, e.g., `172.17.0.2`, can be found by running `docker network inspect bridge` from a local terminal outside of the Docker container.

## How to Debug Tests Failing During Docker Build
`test_coverage.Dockerfile` is intended for use with the remote hosting platform's cloud build pipeline to catch potential issues before live deployment. Normally, there is no reason to run it locally when `../coverage.sh` can be used. In the case where a test fails in `test_coverage.Dockerfile` but not in the local environment (using `../coverage.sh`), follow the steps below to extract test logs:

1. Modify the last line of `test_coverage.Dockerfile` from `RUN make coverage` to `RUN make coverage; exit 0`. By default, the build fails if any unit or integration test fails; this modification allows the image to be built even if testing fails.

2. An extraction script `test_coverage_extract.sh` is provided which builds `test_coverage.Dockerfile`, then extracts the Google Test logs to `~/LastTest.log` and the integration test logs to `~/last_integration_test_result.log` on the host machine.
    ```console
    webserver/docker$ ./test_coverage_extract.sh
    ```