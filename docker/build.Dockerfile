# Stage 2: Prepare production build image
FROM webserver:base

# Build the web server binary with Release build type
WORKDIR /webserver/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN make