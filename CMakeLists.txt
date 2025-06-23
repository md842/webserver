cmake_minimum_required(VERSION 3.30.0...4.0.2)
project(webserver)

# Prevent running cmake in the main project directory to keep build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Set Debug as default build type (Coverage, Debug, Release)
if (CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable cmake testing
include(CTest)
enable_testing()

# Enable GoogleTest. Commit hash in URL must be updated with GoogleTest version updates.
set(CMAKE_CXX_STANDARD 17) # Required by GoogleTest
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Required by GoogleTest
set(GOOGLETEST_VERSION 1.17.0)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/52eb8108c5bdec04579160ae17225d66034bd723.zip
)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# Enable Boost - use static libraries so binaries can be deployed without a full Boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.87 REQUIRED COMPONENTS system process log)
message(STATUS "Boost version: ${Boost_VERSION}")

find_package(OpenSSL REQUIRED) # HTTPS implementation relies on OpenSSL

include_directories(include)

# Add libraries for source files
add_library(server_lib src/server/server.cc)
add_library(http_server_lib src/server/http_server.cc)
add_library(https_server_lib src/server/https_server.cc)
add_library(session_lib src/session/session.cc)
add_library(https_session_lib src/session/https_session.cc)
add_library(log_lib src/log.cc)
add_library(nginx_config_lib src/nginx_config.cc)
add_library(nginx_config_parser_lib src/nginx_config_parser.cc)
add_library(registry_lib src/registry.cc)
add_library(analytics_lib src/analytics.cc)

# Compile request handlers as object libraries to allow self-registration
add_library(file_request_handler_lib OBJECT src/file_request_handler.cc)
add_library(health_request_handler_lib OBJECT src/health_request_handler.cc)
add_library(post_request_handler_lib OBJECT src/post_request_handler.cc)

# Link Boost::log library to log_lib
target_link_libraries(log_lib Boost::log)

# Link required libraries
target_link_libraries(http_server_lib server_lib log_lib)
target_link_libraries(https_server_lib server_lib log_lib)
target_link_libraries(https_session_lib session_lib log_lib)
target_link_libraries(file_request_handler_lib log_lib)
target_link_libraries(post_request_handler_lib log_lib)
target_link_libraries(nginx_config_parser_lib nginx_config_lib log_lib)
target_link_libraries(registry_lib log_lib)

# Compile server_main.cc and link with required libraries
add_executable(server src/server_main.cc)
target_link_libraries(server
  $<TARGET_OBJECTS:file_request_handler_lib>
  $<TARGET_OBJECTS:health_request_handler_lib>
  $<TARGET_OBJECTS:post_request_handler_lib>
  http_server_lib
  https_server_lib
  https_session_lib
  nginx_config_parser_lib
  registry_lib
  analytics_lib
  Boost::process
  Boost::system
  OpenSSL::SSL
)

# Add and link executables for test libraries
add_executable(file_request_handler_test tests/libs/file_request_handler_test.cc)
target_link_libraries(file_request_handler_test
  $<TARGET_OBJECTS:file_request_handler_lib>
  registry_lib
  nginx_config_parser_lib
  gtest_main
  Boost::system
)

add_executable(log_test tests/libs/log_test.cc)
target_link_libraries(log_test
  log_lib
  gtest_main
  Boost::system
)

add_executable(nginx_config_parser_test tests/libs/nginx_config_parser_test.cc)
target_link_libraries(nginx_config_parser_test
  nginx_config_parser_lib
  registry_lib
  gtest_main
  Boost::system
)

add_executable(post_request_handler_test tests/libs/post_request_handler_test.cc)
target_link_libraries(post_request_handler_test
  $<TARGET_OBJECTS:post_request_handler_lib>
  analytics_lib
  registry_lib
  nginx_config_parser_lib
  gtest_main
  Boost::process
  Boost::system
)

add_executable(registry_test tests/libs/registry_test.cc)
target_link_libraries(registry_test
  registry_lib
  gtest_main
  Boost::system
)

# Discover unit tests within test executables compiled above
gtest_discover_tests(file_request_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/libs)
gtest_discover_tests(log_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/libs)
gtest_discover_tests(nginx_config_parser_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/libs)
gtest_discover_tests(post_request_handler_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/libs)
gtest_discover_tests(registry_test WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/libs)

# Add integration tests
add_test(
  NAME
    integration_test
  COMMAND
    ./integration_tests.sh
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/tests/integration)

include(cmake/CodeCoverageReportConfig.cmake)
generate_coverage_report(
  TARGETS
    file_request_handler_lib
    log_lib
    nginx_config_parser_lib
    post_request_handler_lib
    registry_lib
  TESTS
    file_request_handler_test
    log_test
    nginx_config_parser_test
    post_request_handler_test
    registry_test
)