# Stage 2: Build stage
FROM webserver:base AS build

WORKDIR /webserver/build
COPY . /webserver

# Build the project
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN make