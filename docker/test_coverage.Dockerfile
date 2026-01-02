# Stage 3: Testing and coverage report
FROM webserver:base

# Build the web server binary with Coverage build type
WORKDIR /webserver/build_coverage
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..

# Run unit/integration tests and generate coverage report
RUN make coverage