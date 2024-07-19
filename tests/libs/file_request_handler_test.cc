#include <boost/filesystem.hpp> // boost::filesystem::current_path()
#include <boost/filesystem/fstream.hpp>
#include <memory> // std::unique_ptr

#include "file_request_handler.h"
#include "gtest/gtest.h"
#include "nginx_config_parser.h" // Config::inst()

std::string get_content_length(Response res); // Helper
std::string get_content_type(Response res); // Helper

class FileRequestHandlerTest : public ::testing::Test {
protected:
  std::unique_ptr<FileRequestHandler> file_request_handler;
  Request req;
  std::string root_dir;

  void SetUp() override { // Setup test fixture
    file_request_handler = std::make_unique<FileRequestHandler>();

    // Find root dir from cwd, not ideal but no access to argv[0] here
    // OK since tests are only called from within a subdirectory of webserver
    std::string binary_path = boost::filesystem::current_path().string();
    std::string target_dir = "webserver"; // Project directory name
    size_t found = binary_path.find(target_dir); // Search for substring
    if (found != std::string::npos) // Found, extract root dir
      root_dir = binary_path.substr(0, found + target_dir.length());

    // Parse the test config and set absolute root
    Config::inst().parse(root_dir + "/tests/inputs/configs/test_config.conf");
    Config::inst().set_absolute_root(root_dir);

    // GET / HTTP/1.1
    req.method(boost::beast::http::verb::get);
    req.version(11);
  }
};

TEST_F(FileRequestHandlerTest, ConnectionClose){ // Uses test fixture
  req.target("/small.html");
  req.set("Connection", "close");

  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 200);
  EXPECT_FALSE(res->keep_alive());
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res);
}

TEST_F(FileRequestHandlerTest, Create){
  req.target("/small.html");
  std::unique_ptr<FileRequestHandlerFactory> factory =
    std::make_unique<FileRequestHandlerFactory>();
  RequestHandler* file_request_handler = factory->create();

  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->result_int(), 200);

  free(res);
  free(file_request_handler);
  factory.reset();
}

TEST_F(FileRequestHandlerTest, ServeDir){ // Uses test fixture
  req.target("/configs");

  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 404);
  EXPECT_TRUE(res->keep_alive());
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res);
}

TEST_F(FileRequestHandlerTest, ServeHTML){ // Uses test fixture
  req.target("/small.html");

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
}

TEST_F(FileRequestHandlerTest, ServeInaccessible){ // Uses test fixture
  req.target("/small.html");

  // Make the file inaccessible by changing its permissions
  std::string file_path = Config::inst().root() + std::string(req.target());
  chmod(file_path.c_str(), 0000);
  
  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 500);
  EXPECT_TRUE(res->keep_alive());
  EXPECT_EQ(get_content_type(*res), "text/plain");

  chmod(file_path.c_str(), 0644); // Make the file accessible again
  free(res);
}

TEST_F(FileRequestHandlerTest, ServeIndex){ // Uses test fixture
  req.target("/");

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
}

TEST_F(FileRequestHandlerTest, ServeLarge){ // Uses test fixture
  req.target("/large.html");

  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 200);
  EXPECT_TRUE(res->keep_alive());
  EXPECT_EQ(get_content_length(*res), "1068184");
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res);
}

TEST_F(FileRequestHandlerTest, ServeNonexistent){ // Uses test fixture
  req.target("/thisdoesnotexist.html");

  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 404);
  EXPECT_TRUE(res->keep_alive());
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res);
}

TEST_F(FileRequestHandlerTest, ServeOctetStream){ // Uses test fixture
  req.target("/octet_stream");

  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 200);
  EXPECT_TRUE(res->keep_alive());
  EXPECT_EQ(res->body(), "This file has no extension!");
  EXPECT_EQ(get_content_type(*res), "application/octet-stream");

  free(res);
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