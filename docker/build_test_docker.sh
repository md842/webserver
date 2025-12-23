#! /bin/bash
# A script that automates local multi-stage Dockerfile building for convenience.
sudo systemctl start docker.service # Start docker daemon
cd .. # Navigate to desired build context
docker build -f docker/base.Dockerfile -t webserver:base .
docker build -f docker/build.Dockerfile -t webserver:build .
# Copy the frontend production build into the local build context, as the remaining stages rely on it.
# Note: cloudbuild.yaml handles this for the actual production build.
mkdir ./frontend
cp -r ../personal-website/build ./frontend/build
docker build -f docker/test_coverage.Dockerfile .
docker build -f docker/deploy.Dockerfile -t webserver:latest .
# Remove the frontend production build after build completes.
rm -r ./frontend