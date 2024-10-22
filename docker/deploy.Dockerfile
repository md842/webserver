# Stage 4: Deployment stage
FROM webserver:build AS build
FROM ubuntu:oracular AS deploy

# Copy only the files necessary for deployment from build stage to deploy stage
# Copy server binary
COPY --from=build /webserver/build/bin/server /webserver/build/bin/server
# Copy docker config
COPY --from=build /webserver/configs/docker.conf /webserver/configs/docker.conf
# Copy frontend production build directory
COPY --from=build /webserver/frontend/build /webserver/frontend/build

# Expose a port
EXPOSE 80

# Specify entry point (server binary) and arg (config)
ENTRYPOINT ["/webserver/build/bin/server"]
CMD ["configs/docker.conf"]
