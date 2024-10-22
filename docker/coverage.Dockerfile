# Stage 3: Coverage test stage
FROM webserver:build AS test

WORKDIR /webserver/build_coverage
COPY . /webserver

# Generate coverage report
RUN cmake -DCMAKE_BUILD_TYPE=Coverage ..

ENTRYPOINT ["make"]
CMD ["coverage"]
