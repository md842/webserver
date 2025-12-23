# Stage 4: Deployment stage (builds the production deployment container)
FROM webserver:build AS deploy

# Copy only the files necessary for deployment from build stage to deploy stage
# Copy server binary
COPY --from=webserver:build /webserver/build/bin/server /webserver/build/bin/server
# Copy production config
COPY --from=webserver:build /webserver/configs/production_config.conf /webserver/configs/production_config.conf
# Copy frontend production build directory to the root specified in production_config.conf
COPY frontend/build /webserver/frontend

# Expose port 80 for HTTP and port 443 for HTTPS
EXPOSE 80
EXPOSE 443

# Specify entry point (server binary) and arg (config file)
ENTRYPOINT ["/webserver/build/bin/server"]
CMD ["configs/production_config.conf"]
