#! /bin/bash
# A script that automates local dockerfile building for convenience.
sudo systemctl start docker.service # Start docker daemon
cd ..
docker build -f docker/base.Dockerfile -t webserver:base .
docker build -f docker/build.Dockerfile -t webserver:build .
docker build -f docker/coverage.Dockerfile .
docker build -f docker/deploy.Dockerfile -t webserver:latest .