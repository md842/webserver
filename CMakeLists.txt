cmake_minimum_required(VERSION 3.10.0)
project(webserver)

# Make sure developers do not run cmake in the main project directory, to keep
# build artifacts from becoming clutter
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds not allowed.
    Please make a new directory (called a build directory) and run CMake from there.
    You may need to remove CMakeCache.txt." )
endif()

# Turn on debug builds
set(CMAKE_BUILD_TYPE Debug)

# Output binaries to a sub directory "bin"
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Enable Boost
# Use static libraries so binaries can be deployed without a full Boost install
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.50 REQUIRED COMPONENTS system log_setup log)
message(STATUS "Boost version: ${Boost_VERSION}")

include_directories(include)

# Add libraries for source files
add_library(session_lib src/session.cc)
add_library(server_lib src/server.cc src/session.cc)
add_library(log_lib src/log.cc)
add_library(request_handler_registry_lib src/request_handler_registry.cc)

# Compile request handlers as object libraries to allow self-registration
add_library(file_request_handler_lib OBJECT src/file_request_handler.cc)

# Link Boost::log libraries to log_lib
target_link_libraries(log_lib Boost::log_setup Boost::log)

# Link log library to other libraries that use it
target_link_libraries(server_lib log_lib)
target_link_libraries(session_lib log_lib)
target_link_libraries(file_request_handler_lib log_lib)
target_link_libraries(request_handler_registry_lib log_lib)

# Compile server_main.cc and link with required libraries
add_executable(server src/server_main.cc)

target_link_libraries(server
  $<TARGET_OBJECTS:file_request_handler_lib>
  server_lib
  session_lib
  request_handler_registry_lib
  Boost::system
)
