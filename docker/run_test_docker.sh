#! /bin/bash
# A script that automates local dockerfile testing for convenience.
docker run --rm \
    -v ../../certbot/letsencrypt:/etc/letsencrypt:ro \
    -p 8080:80 -p 8081:443 \
    --name webserver webserver:latest