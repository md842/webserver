# Start from latest deployment image
FROM webserver:latest

# Remove previous frontend production build
RUN rm -rf /webserver/frontend
# Copy frontend production build directory to the root specified in production_config.conf
COPY frontend/build /webserver/frontend

# Expose port 80 for HTTP and port 443 for HTTPS
EXPOSE 80
EXPOSE 443

# Specify entry point (server binary) and arg (config)
ENTRYPOINT ["/webserver/build/bin/server"]
CMD ["configs/production_config.conf"]
