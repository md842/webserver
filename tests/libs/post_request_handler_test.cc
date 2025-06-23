#include <boost/filesystem.hpp> // boost::filesystem::current_path()
#include <boost/filesystem/fstream.hpp>
#include <memory> // std::unique_ptr

#include "post_request_handler.h"
#include "gtest/gtest.h"
#include "nginx_config_parser.h" // Config::inst()


std::string get_content_length(Response res); // Helper function
std::string get_content_type(Response res); // Helper function


class PostRequestHandlerTest : public ::testing::Test{
protected:
  std::unique_ptr<PostRequestHandler> post_request_handler;
  std::string default_payload_output = "{"
    R"("cout":"Register a0 (x10): 0\nRegister a1 (x11): 0\nTotal clock cycles: 1\n",)"
    R"("cerr":"PC: 4     Instruction: (nop)   \n")"
  "}"; // This is used several times, so store it as a member field

  Request req;

  void SetUp() override{ // Set up test fixture
    post_request_handler = std::make_unique<PostRequestHandler>();

    // Find root dir from cwd, not ideal but no access to argv[0] here
    // OK since tests are only called from within a subdirectory of webserver
    std::string binary_path = boost::filesystem::current_path().string();

    std::string target_dir = "webserver"; // Project directory name
    size_t found = binary_path.find("webserver"); // Search for substring
    std::string root_dir = binary_path.substr(0, found + target_dir.length());

    // Config's root dir is relative, so provide absolute root_dir found above.
    ConfigParser::inst().set_absolute_root(root_dir);

    // Use the local config since tests rely on files in the frontend
    ConfigParser::inst().parse(root_dir + "/configs/local_config.conf");

    // Initialize config object for the post request handler
    post_request_handler->init_config(ConfigParser::inst().configs().at(1));
    
    // POST / HTTP/1.1
    req.method(boost::beast::http::verb::post);
    req.version(11);
    req.body() = 
    R"({
          "input":"0",
          "input_as_file":true,
          "source":"cpu-simulator"
      })"; // Default payload (produces a valid response)
    req.prepare_payload();
  }
  void TearDown() override{ // Clean up test fixture once done
    post_request_handler.reset(); // Free memory used by unique_ptr
  }
};


TEST_F(PostRequestHandlerTest, ConnectionClose){ // Uses test fixture
  req.set("Connection", "close"); // All other tests use Keep-Alive

  Response* res = post_request_handler->handle_request(req);
  
  EXPECT_EQ(res->result_int(), 200); // 200 OK
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_FALSE(res->keep_alive()); // Connection: Close

  // For default payload, body should contain default payload output
  EXPECT_EQ(res->body(), default_payload_output);
  EXPECT_EQ(get_content_length(*res),
    std::to_string(default_payload_output.length()));
  EXPECT_EQ(get_content_type(*res), "application/json");

  free(res); // Free memory used by created response
}


TEST_F(PostRequestHandlerTest, Create){ // Uses test fixture
  // Create a PostRequestHandler from its factory
  std::unique_ptr<PostRequestHandlerFactory> factory =
    std::make_unique<PostRequestHandlerFactory>();
  RequestHandler* created_handler = factory->create();
  // Initialize config object for the created post request handler
  created_handler->init_config(ConfigParser::inst().configs().at(1));
  
  Response* res = created_handler->handle_request(req);
  
  EXPECT_EQ(res->result_int(), 200); // 200 OK
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive
  
  // For default payload, body should contain default payload output
  EXPECT_EQ(res->body(), default_payload_output);
  EXPECT_EQ(get_content_length(*res),
    std::to_string(default_payload_output.length()));
  EXPECT_EQ(get_content_type(*res), "application/json");

  free(res); // Free memory used by created response
  free(created_handler); // Free memory used by created FileRequestHandler
  factory.reset(); // Free memory used by unique_ptr
}


TEST_F(PostRequestHandlerTest, DirectoryExitAttack){ // Uses test fixture
  req.body() = 
  R"({
        "input":"0",
        "input_as_file":false,
        "source":"../build/bin/server"
     })"; // Attempts to exit the simulations folder and run unintended binary
  req.prepare_payload();

  Response* res = post_request_handler->handle_request(req);
  
  EXPECT_EQ(res->result_int(), 403); // 403 Forbidden
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  std::string expected_output = 
  "{"\
    R"("cout":"Error 403: Forbidden","cerr":"")"\
  "}"; // JSON is used for error reporting to the client
  
  EXPECT_EQ(res->body(), expected_output);
  EXPECT_EQ(get_content_length(*res),
    std::to_string(expected_output.length()));
  EXPECT_EQ(get_content_type(*res), "application/json");

  free(res); // Free memory used by created response
}


TEST_F(PostRequestHandlerTest, InvalidExecutable){ // Uses test fixture
  req.body() = 
  R"({
        "input":"0",
        "input_as_file":false,
        "source":"invalid_source"
     })"; // Invalid source, process_error will be thrown
  req.prepare_payload();

  Response* res = post_request_handler->handle_request(req);
  
  EXPECT_EQ(res->result_int(), 404); // 500 Internal Server Error
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  std::string expected_output = 
  "{"\
    R"("cout":"Error 404: Not Found","cerr":"")"\
  "}"; // JSON is used for error reporting to the client
  
  EXPECT_EQ(res->body(), expected_output);
  EXPECT_EQ(get_content_length(*res),
    std::to_string(expected_output.length()));
  EXPECT_EQ(get_content_type(*res), "application/json");

  free(res); // Free memory used by created response
}


TEST_F(PostRequestHandlerTest, JsonParserError){ // Uses test fixture
  req.body() = 
  R"({
        "input_as_file":false,
     })"; // Invalid extra comma, json_parser_error will be thrown
  req.prepare_payload();

  Response* res = post_request_handler->handle_request(req);
  
  EXPECT_EQ(res->result_int(), 400); // 400 Bad Request
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  std::string expected_output = 
  "{"\
    R"("cout":"Error 400: Bad Request","cerr":"")"\
  "}"; // JSON is used for error reporting to the client
  
  EXPECT_EQ(res->body(), expected_output);
  EXPECT_EQ(get_content_length(*res),
    std::to_string(expected_output.length()));
  EXPECT_EQ(get_content_type(*res), "application/json");

  free(res); // Free memory used by created response
}


TEST_F(PostRequestHandlerTest, PtreeError){ // Uses test fixture
  req.body() = 
  R"({
        "input_as_file":false
     })"; // Missing nodes, ptree_error will be thrown
  req.prepare_payload();

  Response* res = post_request_handler->handle_request(req);
  
  EXPECT_EQ(res->result_int(), 400); // 400 Bad Request
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  std::string expected_output = 
  "{"\
    R"("cout":"Error 400: Bad Request","cerr":"")"\
  "}"; // JSON is used for error reporting to the client
  
  EXPECT_EQ(res->body(), expected_output);
  EXPECT_EQ(get_content_length(*res),
    std::to_string(expected_output.length()));
  EXPECT_EQ(get_content_type(*res), "application/json");

  free(res); // Free memory used by created response
}


TEST_F(PostRequestHandlerTest, RawInput){ // Uses test fixture
  req.body() = 
  R"({
        "input":"0",
        "input_as_file":false,
        "source":"cpu-simulator"
     })";
  req.prepare_payload();

  Response* res = post_request_handler->handle_request(req);
  
  EXPECT_EQ(res->result_int(), 200); // 200 OK
  EXPECT_EQ(res->version(), 11); // HTTP/1.1
  EXPECT_TRUE(res->keep_alive()); // Connection: Keep-Alive

  std::string expected_output = 
  "{"\
    R"("cout":"Error opening file!\n","cerr":"")"\
  "}"; // This error comes from cpu-simulator; it expects a file as input

  EXPECT_EQ(res->body(), expected_output);
  EXPECT_EQ(get_content_length(*res),
    std::to_string(expected_output.length()));
  EXPECT_EQ(get_content_type(*res), "application/json");

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