#include <boost/filesystem.hpp> // current_path, parent_path, path
#include <memory> // std::unique_ptr

#include "gtest/gtest.h"
#include "nginx_config_parser.h" // Config, ConfigParser


class NginxConfigParserTest : public ::testing::Test{
protected:
  std::string root_dir;
  std::string expected_root;
  std::string expected_index;
  std::string configs_folder;

  void SetUp() override{ // Setup test fixture
    /* Unit test cwd is <root>/build/Testing/Temporary (set in CMakeLists.txt),
       so 3 directories up from current_path lands in the webserver root. */
    root_dir = boost::filesystem::current_path()
      .parent_path().parent_path().parent_path().string();

    // Config may provide relative paths, set working directory as found above.
    ConfigParser::inst().set_working_directory(root_dir);

    expected_root = root_dir + "/tests/inputs/";
    expected_index = "small.html";
    configs_folder = root_dir + "/tests/inputs/configs/";
  }
};


// Basic testing


TEST_F(NginxConfigParserTest, BasicGood){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "test_config.conf"));
  Config* config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config->root, expected_root);
  EXPECT_EQ(config->index, expected_index);
  EXPECT_EQ(config->port, 8080);
}


TEST_F(NginxConfigParserTest, BasicEmpty){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "basic_empty_invalid.conf"));
}


TEST_F(NginxConfigParserTest, BasicFileNonexistent){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "nonexistent_file.conf"));
}


/* Commented out for now due to difficulties with Docker and chmod.
TEST_F(NginxConfigParserTest, BasicFileInaccessible){ // Uses test fixture
  // Make the file inaccessible by changing its permissions
  std::string file_path = configs_folder + "test_config.conf";
  chmod(file_path.c_str(), 0000);
  
  EXPECT_FALSE(ConfigParser::inst().parse(file_path));

  chmod(file_path.c_str(), 0644); // Make the file accessible again
}
*/


TEST_F(NginxConfigParserTest, BasicDefaults){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "basic_defaults.conf"));
  Config* config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config->root, "html"); // If root not specified, default to "html".
  EXPECT_EQ(config->index, "index.html"); // If index not specified, default to "index.html".
  EXPECT_EQ(config->port, 80); // If port not specified, default to 80.
}


// Argument testing


TEST_F(NginxConfigParserTest, ArgsInHTTPContext){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_in_http_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsInLocationInvalid){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_in_location_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsInMainContext){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_in_main_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsLocationOverrides){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "args_location_overrides.conf"));
  Config* config = ConfigParser::inst().configs().at(0); // Extract first parsed config

  EXPECT_EQ(config->type, Config::ServerType::HTTP_SERVER);
  EXPECT_EQ(config->root, expected_root);
  EXPECT_EQ(config->index, expected_index);

  /* Extract first parsed location block from config (locations[3] contains
     location blocks with no modifiers) */
  LocationBlock* location = config->locations[3].at(0);
  EXPECT_EQ(location->root, root_dir + "/html/");
  EXPECT_EQ(location->index, "index.html");
}


TEST_F(NginxConfigParserTest, ArgsPortNegative){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_port_negative_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsPortDecimal){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_port_decimal_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsPortString){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_port_string_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsPortTooLarge){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_port_too_large_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsSSLArgInvalid){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_ssl_arg_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsSSLGood){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "args_ssl_good.conf"));
  Config* config = ConfigParser::inst().configs().at(0); // Extract first parsed config

  EXPECT_EQ(config->type, Config::ServerType::HTTP_SERVER);
  EXPECT_EQ(config->ret, 301);
  EXPECT_EQ(config->ret_val, "https://$host:8080$request_uri");
  EXPECT_EQ(config->host, "localhost");
  EXPECT_EQ(config->port, 8079);

  config = ConfigParser::inst().configs().at(1); // Extract second parsed config

  EXPECT_EQ(config->type, Config::ServerType::HTTPS_SERVER);
  EXPECT_EQ(config->root, expected_root);
  EXPECT_EQ(config->index, expected_index);
  EXPECT_EQ(config->host, "localhost");
  EXPECT_EQ(config->port, 8080);
}


TEST_F(NginxConfigParserTest, ArgsSSLInNonSSL){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_ssl_in_non_ssl_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsSSLMissing){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_ssl_missing_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsUnknownInServer){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_unknown_in_server_invalid.conf"));
}


// Comments testing


TEST_F(NginxConfigParserTest, CommentGood){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "comment_good.conf"));
  Config* config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config->root, expected_root);
  EXPECT_EQ(config->index, expected_index);
  EXPECT_EQ(config->port, 8080);
}


TEST_F(NginxConfigParserTest, CommentInBlockend){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_blockend_invalid.conf"));
}


TEST_F(NginxConfigParserTest, CommentInBlockstart){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_blockstart_invalid.conf"));
}


TEST_F(NginxConfigParserTest, CommentInIndex){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_index_invalid.conf"));
}


TEST_F(NginxConfigParserTest, CommentInListen){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_listen_invalid.conf"));
}


TEST_F(NginxConfigParserTest, CommentInLocation){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_location_invalid.conf"));
}


TEST_F(NginxConfigParserTest, CommentInRoot){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_root_invalid.conf"));
}


TEST_F(NginxConfigParserTest, CommentInTryfiles){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_tryfiles_invalid.conf"));
}


// Context transition testing


TEST_F(NginxConfigParserTest, ContextSwitchOrder){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "context_switch_order_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ContextSwitchUnknown){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "context_switch_unknown_invalid.conf"));
}



// Escape character testing


TEST_F(NginxConfigParserTest, EscapeQuotes){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "escape_quotes.conf"));
  Config* config = ConfigParser::inst().configs().at(0); // Extract parsed config

  // Expected values are different for this test case due to the escapes
  EXPECT_EQ(config->root, expected_root + "\"'/");
  EXPECT_EQ(config->index, "small.html\"'");
  EXPECT_EQ(config->port, 8080);
}


TEST_F(NginxConfigParserTest, EscapeWords){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "escape_words.conf"));
  Config* config = ConfigParser::inst().configs().at(0); // Extract parsed config

  // Expected values are different for this test case due to the escapes
  EXPECT_EQ(config->root, root_dir + "/\\" + "tests/inputs/");
  EXPECT_EQ(config->index, "s\\mall.html");
  EXPECT_EQ(config->port, 8080);
}


// Quote word testing


TEST_F(NginxConfigParserTest, QuoteDoubleIndex){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "quote_double_index.conf"));
  Config* config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config->root, expected_root);
  EXPECT_EQ(config->index, expected_index);
  EXPECT_EQ(config->port, 8080);
}


TEST_F(NginxConfigParserTest, QuoteInvalidEnd){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "quote_end_invalid.conf"));
}


TEST_F(NginxConfigParserTest, QuoteSingleIndex){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "quote_single_index.conf"));
  Config* config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config->root, expected_root);
  EXPECT_EQ(config->index, expected_index);
  EXPECT_EQ(config->port, 8080);
}


TEST_F(NginxConfigParserTest, QuoteUnterminated){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "quote_unterminated_invalid.conf"));
}


TEST_F(NginxConfigParserTest, QuoteValidEnds){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "quote_valid_ends.conf"));
  Config* config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config->root, expected_root);
  EXPECT_EQ(config->index, expected_index);
  EXPECT_EQ(config->port, 8080);
}


// Return directive testing


TEST_F(NginxConfigParserTest, Return3xx){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "return_3xx.conf"));
  Config* config = ConfigParser::inst().configs().at(1); // Extract second parsed config

  EXPECT_EQ(config->ret, 301);
  EXPECT_EQ(config->ret_val, "https://$host:8080$request_uri");
  EXPECT_EQ(config->port, 8081);

  config = ConfigParser::inst().configs().at(2); // Extract third parsed config

  EXPECT_EQ(config->ret, 302);
  EXPECT_EQ(config->ret_val, "https://$host:8080$request_uri");
  EXPECT_EQ(config->port, 8082);

  config = ConfigParser::inst().configs().at(3); // Extract fourth parsed config

  EXPECT_EQ(config->ret, 303);
  EXPECT_EQ(config->ret_val, "https://$host:8080$request_uri");
  EXPECT_EQ(config->port, 8083);

  config = ConfigParser::inst().configs().at(4); // Extract fifth parsed config

  EXPECT_EQ(config->ret, 307);
  EXPECT_EQ(config->ret_val, "https://$host:8080$request_uri");
  EXPECT_EQ(config->port, 8084);

  config = ConfigParser::inst().configs().at(5); // Extract fifth parsed config

  EXPECT_EQ(config->ret, 308);
  EXPECT_EQ(config->ret_val, "https://$host:8080$request_uri");
  EXPECT_EQ(config->port, 8085);
}


TEST_F(NginxConfigParserTest, Return3xxMissingHost){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "return_3xx_missing_host_invalid.conf"));
}


TEST_F(NginxConfigParserTest, Return3xxMissingURL){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "return_3xx_missing_url_invalid.conf"));
}


TEST_F(NginxConfigParserTest, Return302Default){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "return_302_default.conf"));
  Config* config = ConfigParser::inst().configs().at(1); // Extract second parsed config

  EXPECT_EQ(config->ret, 302);
  EXPECT_EQ(config->ret_val, "https://$host:8080$request_uri");
  EXPECT_EQ(config->port, 8081);
}


TEST_F(NginxConfigParserTest, Return304){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "return_304_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ReturnNonInteger){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "return_non_integer_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ReturnOther){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "return_other.conf"));
  Config* config = ConfigParser::inst().configs().at(0); // Extract first parsed config

  EXPECT_EQ(config->ret, 200);
  EXPECT_EQ(config->ret_val, "");
  EXPECT_EQ(config->port, 8081);

  config = ConfigParser::inst().configs().at(1); // Extract second parsed config

  EXPECT_EQ(config->ret, 400);
  EXPECT_EQ(config->ret_val, "Optional text");
  EXPECT_EQ(config->port, 8082);
}


// Structure testing


TEST_F(NginxConfigParserTest, StructureExtraBlockEnd){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "structure_extra_block_end_invalid.conf"));
}


TEST_F(NginxConfigParserTest, StructureLocationModifier){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "structure_location_modifier_invalid.conf"));
}


TEST_F(NginxConfigParserTest, StructureMalformedLocation){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "structure_malformed_location_invalid.conf"));
}

TEST_F(NginxConfigParserTest, StructureMalformedLocationSize){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "structure_malformed_location_size_invalid.conf"));
}


// Token transition testing


TEST_F(NginxConfigParserTest, TransitionInvalidBlockstart){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "transition_blockstart_invalid.conf"));
}


TEST_F(NginxConfigParserTest, TransitionInvalidEOF){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "transition_eof_invalid.conf"));
}


TEST_F(NginxConfigParserTest, TransitionInvalidSemicolon){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "transition_semicolon_invalid.conf"));
}