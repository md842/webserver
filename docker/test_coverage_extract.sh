#! /bin/bash
# A script that automates extracting test logs from the Docker image built by test_coverage.Dockerfile.
sudo systemctl start docker.service # Start docker daemon
cd ..

# Prepare build environment
docker build -f docker/base.Dockerfile -t webserver:base .
# Testing and coverage report; tag as webserver:coverage to be able to run in a container
docker build -f docker/test_coverage.Dockerfile -t webserver:coverage .
# Run container to extract test logs from
docker run -p 8080:80 --name coverage webserver:coverage

# Extract Google Test logs and integration test logs to home directory on host machine
docker cp coverage:/webserver/build_coverage/Testing/Temporary/LastTest.log ~/LastTest.log
docker cp coverage:/webserver/tests/integration/last_integration_test_result.log ~/last_integration_test_result.log

# Clean up coverage container and webserver:coverage image, as both are unused outside of this context
docker rm coverage
docker image rm webserver:coverage