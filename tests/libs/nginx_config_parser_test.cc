#include <boost/filesystem.hpp> // boost::filesystem::current_path()
#include <boost/filesystem/fstream.hpp>
#include <memory> // std::unique_ptr

#include "gtest/gtest.h"
#include "nginx_config_parser.h" // Config, ConfigParser


class NginxConfigParserTest : public ::testing::Test{
protected:
  std::string absolute_root;
  std::string expected_root;
  std::string expected_index;
  std::string configs_folder;

  void SetUp() override{ // Setup test fixture
    // Find root dir from cwd, not ideal but no access to argv[0] here
    // OK since tests are only called from within a subdirectory of webserver
    std::string binary_path = boost::filesystem::current_path().string();
    std::string root_dir = "";
    std::string target_dir = "webserver"; // Project directory name
    size_t found = binary_path.find(target_dir); // Search for substring
    if (found != std::string::npos) // Found, extract root dir
      absolute_root = binary_path.substr(0, found + target_dir.length());

    ConfigParser::inst().set_absolute_root(absolute_root);

    expected_root = absolute_root + "/tests/inputs/";
    expected_index = "small.html";
    configs_folder = absolute_root + "/tests/inputs/configs/";
  }
};

// Basic testing

TEST_F(NginxConfigParserTest, BasicGood){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "test_config.conf"));
  Config config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config.root, expected_root);
  EXPECT_EQ(config.index, expected_index);
  EXPECT_EQ(config.port, 8080);
}


TEST_F(NginxConfigParserTest, BasicEmpty){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "basic_empty.conf"));
}


TEST_F(NginxConfigParserTest, BasicFileNonexistent){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "nonexistent_file.conf"));
}


/* Commented out for now due to difficulties with Docker and chmod.
TEST_F(NginxConfigParserTest, BasicFileInaccessible){ // Uses test fixture
  // Make the file inaccessible by changing its permissions
  std::string file_path = absolute_root + "basic_good.conf";
  chmod(file_path.c_str(), 0000);
  
  EXPECT_FALSE(ConfigParser::inst().parse(file_path));

  chmod(file_path.c_str(), 0644); // Make the file accessible again
}
*/


TEST_F(NginxConfigParserTest, BasicNoIndex){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "basic_no_index.conf"));
}


TEST_F(NginxConfigParserTest, BasicNoPort){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "basic_no_port.conf"));
}


TEST_F(NginxConfigParserTest, BasicNoRoot){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "basic_no_root.conf"));
}

// Argument testing

TEST_F(NginxConfigParserTest, ArgsInHTTPContext){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_in_http.conf"));
}


TEST_F(NginxConfigParserTest, ArgsInMainContext){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_in_main.conf"));
}


TEST_F(NginxConfigParserTest, ArgsInLocationInvalid){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "args_in_location_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsUnimplemented){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "args_unimplemented.conf"));
  Config config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config.root, expected_root);
  EXPECT_EQ(config.index, expected_index);
  EXPECT_EQ(config.port, 8080);
}

// Comments testing

TEST_F(NginxConfigParserTest, CommentGood){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "comment_good.conf"));
  Config config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config.root, expected_root);
  EXPECT_EQ(config.index, expected_index);
  EXPECT_EQ(config.port, 8080);
}


TEST_F(NginxConfigParserTest, CommentInBlockend){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_blockend.conf"));
}


TEST_F(NginxConfigParserTest, CommentInBlockstart){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_blockstart.conf"));
}


TEST_F(NginxConfigParserTest, CommentInIndex){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_index.conf"));
}


TEST_F(NginxConfigParserTest, CommentInListen){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_listen.conf"));
}


TEST_F(NginxConfigParserTest, CommentInLocation){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_location.conf"));
}


TEST_F(NginxConfigParserTest, CommentInRoot){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_root.conf"));
}


TEST_F(NginxConfigParserTest, CommentInTryfiles){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "comment_in_tryfiles.conf"));
}

// Context transition testing

TEST_F(NginxConfigParserTest, ContextInvalidSwitch){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "context_invalid_switch.conf"));
}

// Escape character testing

TEST_F(NginxConfigParserTest, EscapeQuotes){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "escape_quotes.conf"));
  Config config = ConfigParser::inst().configs().at(0); // Extract parsed config

  // Expected values are different for this test case due to the escapes
  EXPECT_EQ(config.root, absolute_root + "/tests/inputs\\\"'/");
  EXPECT_EQ(config.index, "small.html\\\"'");
  EXPECT_EQ(config.port, 8080);
}


TEST_F(NginxConfigParserTest, EscapeWords){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "escape_words.conf"));
  Config config = ConfigParser::inst().configs().at(0); // Extract parsed config

  // Expected values are different for this test case due to the escapes
  EXPECT_EQ(config.root, absolute_root + "\\/tests/inputs/");
  EXPECT_EQ(config.index, "s\\mall.html");
  EXPECT_EQ(config.port, 8080);
}

// Quote word testing

TEST_F(NginxConfigParserTest, QuoteDoubleIndex){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "quote_double_index.conf"));
  Config config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config.root, expected_root);
  EXPECT_EQ(config.index, expected_index);
  EXPECT_EQ(config.port, 8080);
}


TEST_F(NginxConfigParserTest, QuoteInvalidEnd){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "quote_invalid_end.conf"));
}


TEST_F(NginxConfigParserTest, QuoteSingleIndex){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "quote_single_index.conf"));
  Config config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config.root, expected_root);
  EXPECT_EQ(config.index, expected_index);
  EXPECT_EQ(config.port, 8080);
}


TEST_F(NginxConfigParserTest, QuoteValidEnds){ // Uses test fixture
  EXPECT_TRUE(ConfigParser::inst().parse(configs_folder + "quote_valid_ends.conf"));
  Config config = ConfigParser::inst().configs().at(0); // Extract parsed config

  EXPECT_EQ(config.root, expected_root);
  EXPECT_EQ(config.index, expected_index);
  EXPECT_EQ(config.port, 8080);
}


TEST_F(NginxConfigParserTest, QuoteUnterminated){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "quote_unterminated.conf"));
}

// Structure testing

TEST_F(NginxConfigParserTest, StructureExtraBlockEnd){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "structure_extra_block_end.conf"));
}


TEST_F(NginxConfigParserTest, StructureMalformedLocation){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "structure_malformed_location.conf"));
}

// Token transition testing

TEST_F(NginxConfigParserTest, TransitionInvalidBlockstart){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "transition_invalid_blockstart.conf"));
}


TEST_F(NginxConfigParserTest, TransitionInvalidEOF){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "transition_invalid_eof.conf"));
}


TEST_F(NginxConfigParserTest, TransitionInvalidSemicolon){ // Uses test fixture
  EXPECT_FALSE(ConfigParser::inst().parse(configs_folder + "transition_invalid_semicolon.conf"));
}
