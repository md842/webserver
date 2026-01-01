# Stage 4: Prepare deployment image
# Pull fresh debian image without build environment to produce minimal deployment image
FROM debian:forky-slim

# Copy only the files necessary for deployment from build stage to deploy stage
# Copy production web server binary
COPY --from=webserver:build /webserver/build/bin/server /webserver/build/bin/server
# Copy production config
COPY --from=webserver:build /webserver/configs/production_config.conf /webserver/configs/production_config.conf
# Copy frontend production build directory to the root specified in production_config.conf
COPY frontend/build /webserver/frontend

# Expose port 80 for HTTP and port 443 for HTTPS
EXPOSE 80
EXPOSE 443

# Specify entry point (server binary) and arg (config)
ENTRYPOINT ["/webserver/build/bin/server"]
CMD ["configs/production_config.conf"]
