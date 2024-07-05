#include <boost/filesystem.hpp> // boost::filesystem::current_path()
#include <memory> // std::unique_ptr

#include "file_request_handler.h"
#include "gtest/gtest.h"

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

TEST_F(FileRequestHandlerTest, ServeHTML){ // Uses test fixture
  std::unique_ptr<FileRequestHandler> file_request_handler =
    std::make_unique<FileRequestHandler>(root_dir + "/tests/inputs/small.html");
  Response* res = file_request_handler->handle_request(req);
  EXPECT_EQ(res->version(), 11);
  EXPECT_EQ(res->result_int(), 200);
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
  free(res);
  file_request_handler.reset();
}