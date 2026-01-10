# Stage 3: Testing and coverage report
FROM webserver:base

# Build the web server binary with Coverage build type
WORKDIR /webserver/build_coverage
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..

# Set permissions on files used by FileRequestHandlerTest.ServeInaccessible
# and NginxConfigParserTest.BasicFileInaccessible unit tests to make them
# inaccessible to an unprivileged user within the Docker image
RUN chmod 000 /webserver/tests/inputs/inaccessible
RUN chmod 000 /webserver/tests/inputs/configs/basic_inaccessible.conf

# Unit tests must be run as an unprivileged user, or the aforementioned unit
# tests will fail (default root user can ignore file permissions).
RUN useradd user
# Give user required permissions to build the project
RUN chown -R user /webserver/build_coverage
# Give user required permissions to manipulate integration test result logs
RUN chown -R user /webserver/tests/integration

# Run unit/integration tests as unprivileged user and generate coverage report
USER user
RUN make coverage