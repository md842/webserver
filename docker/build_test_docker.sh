#! /bin/bash
# A script that automates local dockerfile building for convenience.
sudo systemctl start docker.service # Start docker daemon
cd ..

# Copy local frontend production build to where Dockerfiles expect it to be (emulates cloud build steps 2-4)
mkdir ./frontend
cp -r ../personal-website/build ./frontend/build

# Prepare build environment (cloud build step 5)
docker build -f docker/base.Dockerfile -t webserver:base .
# Prepare production build image (cloud build step 6)
docker build -f docker/build.Dockerfile -t webserver:build .
# Testing and coverage report (cloud build step 7)
docker build -f docker/test_coverage.Dockerfile .
# Prepare deployment image (cloud build step 8)
docker build -f docker/deploy.Dockerfile -t webserver:latest .

# Clean up frontend production build copy after deployment image build completes
rm -r ./frontend