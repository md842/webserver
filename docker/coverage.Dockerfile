# Stage 3: Coverage test stage
FROM webserver:base AS coverage

WORKDIR /webserver/build_coverage
COPY . /webserver

# Generate coverage report
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..
RUN make coverage
