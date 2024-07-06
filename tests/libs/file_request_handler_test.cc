#include <boost/filesystem.hpp> // boost::filesystem::current_path()
#include <boost/filesystem/fstream.hpp>
#include <memory> // std::unique_ptr

#include "file_request_handler.h"
#include "gtest/gtest.h"

std::string get_content_length(Response res); // Helper
std::string get_content_type(Response res); // Helper

class FileRequestHandlerTest : public ::testing::Test {
protected:
  Request req;
  std::string root_dir;

  void SetUp() override { // Setup test fixture
    // Find root dir from cwd, not ideal but no access to argv[0] here
    // OK since tests are only called from within a subdirectory of webserver
    std::string binary_path = boost::filesystem::current_path().string();
    std::string target_dir = "webserver"; // Project directory name
    size_t found = binary_path.find(target_dir); // Search for substring
    if (found != std::string::npos) // Found, extract root dir
      root_dir = binary_path.substr(0, found + target_dir.length());

    // GET / HTTP/1.1
    req.method(boost::beast::http::verb::get);
    req.version(11);
  }
};

TEST_F(FileRequestHandlerTest, ConnectionClose){ // Uses test fixture
  std::string full_path = root_dir + "/tests/inputs/small.html";
  std::unique_ptr<FileRequestHandler> file_request_handler =
    std::make_unique<FileRequestHandler>(full_path);

  req.set("Connection", "close");

  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 200);
  EXPECT_FALSE(res->keep_alive());
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res);
  file_request_handler.reset();
}

TEST_F(FileRequestHandlerTest, Create){
  std::string full_path = root_dir + "/tests/inputs/small.html";
  std::unique_ptr<FileRequestHandlerFactory> factory =
    std::make_unique<FileRequestHandlerFactory>();
  RequestHandler* file_request_handler = factory->create(full_path);

  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->result_int(), 200);

  free(res);
  free(file_request_handler);
  factory.reset();
}

TEST_F(FileRequestHandlerTest, ServeDir){ // Uses test fixture
  std::string full_path = root_dir + "/tests/inputs";
  std::unique_ptr<FileRequestHandler> file_request_handler =
    std::make_unique<FileRequestHandler>(full_path);

  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 404);
  EXPECT_TRUE(res->keep_alive());
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res);
  file_request_handler.reset();
}

TEST_F(FileRequestHandlerTest, ServeHTML){ // Uses test fixture
  std::unique_ptr<FileRequestHandler> file_request_handler =
    std::make_unique<FileRequestHandler>(root_dir +
      "/tests/inputs/small.html");
  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 200);
  EXPECT_TRUE(res->keep_alive());
  EXPECT_EQ(res->body(), "<!DOCTYPE html>\n"
                         "<html lang=\"en\">\n"
                         "  <head>\n"
                         "    <meta charset=\"utf-8\">\n"
                         "  </head>\n"
                         "  <body>\n"
                         "    <h1>This is a placeholder HTML file for testing purposes.</h1>\n"
                         "  </body>\n"
                         "</html>"
  );
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res);
  file_request_handler.reset();
}

TEST_F(FileRequestHandlerTest, ServeInaccessible){ // Uses test fixture
  std::string full_path = root_dir + "/tests/inputs/no_permission.html";
  std::unique_ptr<FileRequestHandler> file_request_handler =
    std::make_unique<FileRequestHandler>(full_path);

  // Make the file inaccessible by changing its permissions
  chmod(full_path.c_str(), 0000);
  
  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 500);
  EXPECT_TRUE(res->keep_alive());
  EXPECT_EQ(get_content_type(*res), "text/plain");

  chmod(full_path.c_str(), 0755); // Make the file accessible again
  free(res);
  file_request_handler.reset();
}

TEST_F(FileRequestHandlerTest, ServeLarge){ // Uses test fixture
  std::unique_ptr<FileRequestHandler> file_request_handler =
    std::make_unique<FileRequestHandler>(root_dir +
      "/tests/inputs/large.html");
  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 200);
  EXPECT_TRUE(res->keep_alive());
  EXPECT_EQ(get_content_length(*res), "1068184");
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res);
  file_request_handler.reset();
}

TEST_F(FileRequestHandlerTest, ServeNonexistent){ // Uses test fixture
  std::string full_path = root_dir + "/tests/inputs/thisdoesnotexist.html";
  std::unique_ptr<FileRequestHandler> file_request_handler =
    std::make_unique<FileRequestHandler>(full_path);

  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 404);
  EXPECT_TRUE(res->keep_alive());
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res);
  file_request_handler.reset();
}

TEST_F(FileRequestHandlerTest, ServeOctetStream){ // Uses test fixture
  std::string full_path = root_dir + "/tests/inputs/octet_stream";
  std::unique_ptr<FileRequestHandler> file_request_handler =
    std::make_unique<FileRequestHandler>(full_path);

  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 200);
  EXPECT_TRUE(res->keep_alive());
  EXPECT_EQ(res->body(), "This file has no extension!");
  EXPECT_EQ(get_content_type(*res), "application/octet-stream");

  free(res);
  file_request_handler.reset();
}

// Helper function to extract Content-Type header
std::string get_content_length(Response res){
  for (auto& header : res.base()){
    if (header.name_string() == "Content-Length")
      return header.value();
  }
  return "";
}

// Helper function to extract Content-Type header
std::string get_content_type(Response res){
  for (auto& header : res.base()){
    if (header.name_string() == "Content-Type")
      return header.value();
  }
  return "";
}