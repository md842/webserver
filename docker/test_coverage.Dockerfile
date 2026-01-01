# Stage 3: Testing and coverage report
FROM webserver:base

# Copy frontend production build directory to where the integration test script expects it to be
COPY frontend/build /personal-website/build

# Build the web server binary with Coverage build type
WORKDIR /webserver/build_coverage
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..

# Run unit/integration tests and generate coverage report
RUN make coverage