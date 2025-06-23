#include <boost/filesystem.hpp> // current_path, path
#include <iomanip> // put_time
#include <memory> // std::unique_ptr

#include "file_request_handler.h"
#include "gtest/gtest.h"
#include "nginx_config_parser.h" // Config, ConfigParser


std::string get_content_length(Response res); // Helper function
std::string get_content_type(Response res); // Helper function
// Borrow implementation from file_request_handler.cc
std::string last_modified_time(boost::filesystem::path* file_obj);


class FileRequestHandlerTest : public ::testing::Test{
protected:
  std::unique_ptr<FileRequestHandler> file_request_handler;
  std::string index_contents = 
  "<!DOCTYPE html>\n"
  "<html lang=\"en\">\n"
  "  <head>\n"
  "    <meta charset=\"utf-8\">\n"
  "  </head>\n"
  "  <body>\n"
  "    <h1>This is a placeholder HTML file for testing purposes.</h1>\n"
  "  </body>\n"
  "</html>"; // This is used several times, so store it as a member field
  Request req;

  void SetUp() override{ // Set up test fixture
    file_request_handler = std::make_unique<FileRequestHandler>();

    // Find root dir from cwd, not ideal but no access to argv[0] here
    // OK since tests are only called from within a subdirectory of webserver
    std::string binary_path = boost::filesystem::current_path().string();

    std::string target_dir = "webserver"; // Project directory name
    size_t found = binary_path.find("webserver"); // Search for substring
    std::string root_dir = binary_path.substr(0, found + target_dir.length());

    // Set the absolute root and parse the test config
    // test_file_config specifies small.html as index and /tests/inputs as root
    ConfigParser::inst().set_absolute_root(root_dir);
    ConfigParser::inst().parse(root_dir +
                               "/tests/inputs/configs/test_config.conf");
    // Initialize config object for the file request handler
    file_request_handler->init_config(ConfigParser::inst().configs().at(0));

    // GET / HTTP/1.1 (will serve small.html by default)
    req.method(boost::beast::http::verb::get);
    req.target("/");
    req.version(11);
  }
  void TearDown() override{ // Clean up test fixture once done
    file_request_handler.reset(); // Free memory used by unique_ptr
  }
};


TEST_F(FileRequestHandlerTest, ConnectionClose){ // Uses test fixture
  req.set("Connection", "close"); // All other tests use Keep-Alive

  Response* res = file_request_handler->handle_request(req);

  EXPECT_EQ(res->result_int(), 200); // 200 OK
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_FALSE(res->keep_alive()); // Connection: Close

  // For default target /, body should contain index (small.html)
  EXPECT_EQ(res->body(), index_contents);
  EXPECT_EQ(get_content_length(*res),
    std::to_string(index_contents.length()));
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res); // Free memory used by created response
}


TEST_F(FileRequestHandlerTest, Create){ // Uses test fixture
  // Create a FileRequestHandler from its factory
  std::unique_ptr<FileRequestHandlerFactory> factory =
    std::make_unique<FileRequestHandlerFactory>();
  RequestHandler* created_handler = factory->create();
  // Initialize config object for the created file request handler
  created_handler->init_config(ConfigParser::inst().configs().at(0));

  Response* res = created_handler->handle_request(req);

  EXPECT_EQ(res->result_int(), 200); // 200 OK
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  // For default target /, body should contain index (small.html)
  EXPECT_EQ(res->body(), index_contents);
  EXPECT_EQ(get_content_length(*res),
    std::to_string(index_contents.length()));
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res); // Free memory used by created response
  free(created_handler); // Free memory used by created FileRequestHandler
  factory.reset(); // Free memory used by unique_ptr
}


TEST_F(FileRequestHandlerTest, ServeDir){ // Uses test fixture
  req.target("/configs"); // Set target to a directory

  Response* res = file_request_handler->handle_request(req);

  EXPECT_EQ(res->result_int(), 404); // 404 Not Found
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  // For React Router, body should contain index (small.html) for 404 response
  EXPECT_EQ(res->body(), index_contents);
  EXPECT_EQ(get_content_length(*res),
    std::to_string(index_contents.length()));
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res); // Free memory used by created response
}


/* Commented out for now due to difficulties with Docker and chmod.
TEST_F(FileRequestHandlerTest, ServeInaccessible){ // Uses test fixture
  std::string file_path = Config::inst().root() + "small.html";
  chmod(file_path.c_str(), 0000); // Make inaccessible by changing permissions
  
  Response* res = file_request_handler->handle_request(req);

  EXPECT_EQ(res->result_int(), 500); // 500 Internal Server Error
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  // Body should contain error HTML for 500 response. Commenting out for now
  // since I haven't finalized the 500 response HTML.
  //EXPECT_EQ(res->body(), "");
  //EXPECT_EQ(get_content_length(*res), "");
  //EXPECT_EQ(get_content_type(*res), "text/html");

  chmod(file_path.c_str(), 0644); // Make file accessible again
  free(res); // Free memory used by created response
}
*/


TEST_F(FileRequestHandlerTest, ServeLarge){ // Uses test fixture
  req.target("/large.html"); // Set target to desired file

  Response* res = file_request_handler->handle_request(req);

  EXPECT_EQ(res->result_int(), 200); // 200 OK
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  // Skip checking body contents due to size
  EXPECT_EQ(get_content_length(*res), "1068184"); // Length of large.html
  EXPECT_EQ(get_content_type(*res), "text/html"); // Type of large.html

  free(res); // Free memory used by created response
}


TEST_F(FileRequestHandlerTest, ServeNonexistent){ // Uses test fixture
  req.target("/thisdoesnotexist.html"); // Set target to nonexistent file

  Response* res = file_request_handler->handle_request(req);

  EXPECT_EQ(res->result_int(), 404); // 404 Not Found
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  // For React Router, body should contain index (small.html) for 404 response
  EXPECT_EQ(res->body(), index_contents);
  EXPECT_EQ(get_content_length(*res),
    std::to_string(index_contents.length()));
  EXPECT_EQ(get_content_type(*res), "text/html");

  free(res); // Free memory used by created response
}


TEST_F(FileRequestHandlerTest, ServeOctetStream){ // Uses test fixture
  req.target("/octet_stream"); // Set target to desired file

  Response* res = file_request_handler->handle_request(req);

  EXPECT_EQ(res->result_int(), 200); // 200 OK
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  EXPECT_EQ(res->body(), "This file has no extension!"); // Contents of octet_stream
  EXPECT_EQ(get_content_length(*res), "27"); // Length of octet_stream
  EXPECT_EQ(get_content_type(*res), "application/octet-stream"); // Type of octet_stream

  free(res); // Free memory used by created response
}


TEST_F(FileRequestHandlerTest, ValidateCache){ // Uses test fixture
  // Set If-Modified-Since to last_modified_time to produce a 304 response.
  Config* config = ConfigParser::inst().configs().at(0);
  boost::filesystem::path* file_obj =
    new boost::filesystem::path(config->root + config->index);
  std::string lm_time = last_modified_time(file_obj);
  req.set(boost::beast::http::field::if_modified_since, lm_time);

  Response* res = file_request_handler->handle_request(req);

  EXPECT_EQ(res->result_int(), 304); // 304 Not Modified
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  // Body should be empty for 304 response
  EXPECT_EQ(res->body(), "");
  // Content-Length should not be set when body is empty
  EXPECT_EQ(get_content_length(*res), "");
  // Content-Type should not be set when body is empty
  EXPECT_EQ(get_content_type(*res), "");

  free(res); // Free memory used by created response
}


// Helper function to extract Content-Type header
std::string get_content_length(Response res){
  try{
    return res.at(boost::beast::http::field::content_length);
  }
  catch (std::out_of_range){
    return "";
  }
}


// Helper function to extract Content-Type header
std::string get_content_type(Response res){
  try{
    return res.at(boost::beast::http::field::content_type);
  }
  catch (std::out_of_range){
    return "";
  }
}