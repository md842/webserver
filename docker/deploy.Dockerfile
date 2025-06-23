# Stage 4: Deployment stage
FROM webserver:build AS build
FROM ubuntu:oracular AS deploy

# Copy only the files necessary for deployment from build stage to deploy stage
# Copy server binary
COPY --from=build /webserver/build/bin/server /webserver/build/bin/server
# Copy production config
COPY --from=build /webserver/configs/production_config.conf /webserver/configs/production_config.conf
# Copy frontend production build directory
COPY --from=build /webserver/frontend/build /webserver/frontend/build
# Copy test certificates
COPY --from=build /webserver/tests/certs /webserver/tests/certs

# Expose port 80 for HTTP and port 443 for HTTPS
EXPOSE 80
EXPOSE 443

# Specify entry point (server binary) and arg (config)
ENTRYPOINT ["/webserver/build/bin/server"]
CMD ["configs/production_config.conf"]
