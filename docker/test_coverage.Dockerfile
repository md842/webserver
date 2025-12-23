# Stage 3: Testing stage (runs unit tests and generates a coverage report)
FROM webserver:base AS coverage

WORKDIR /webserver/build_coverage
COPY . /webserver

# Copy frontend production build directory to where the integration test script expects it to be
COPY frontend/build /personal-website/build

# Generate coverage report
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage
