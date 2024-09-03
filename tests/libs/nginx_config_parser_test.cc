#include <boost/filesystem.hpp> // boost::filesystem::current_path()
#include <boost/filesystem/fstream.hpp>
#include <memory> // std::unique_ptr

#include "gtest/gtest.h"
#include "nginx_config_parser.h" // Config::inst()


class NginxConfigParserTest : public ::testing::Test{
protected:
  std::string root_dir;

  void SetUp() override{ // Setup test fixture
    // Find root dir from cwd, not ideal but no access to argv[0] here
    // OK since tests are only called from within a subdirectory of webserver
    std::string binary_path = boost::filesystem::current_path().string();
    std::string target_dir = "webserver"; // Project directory name
    size_t found = binary_path.find(target_dir); // Search for substring
    if (found != std::string::npos) // Found, extract root dir
      root_dir = binary_path.substr(0, found + target_dir.length());

    root_dir += "/tests/inputs/configs/"; // Add test configs folder to root_dir
  }
};

// Basic testing

TEST_F(NginxConfigParserTest, BasicGood){ // Uses test fixture
  EXPECT_TRUE(Config::inst().parse(root_dir + "basic_good.conf"));

  EXPECT_EQ(Config::inst().index(), "files_to_serve/index.html");
  EXPECT_EQ(Config::inst().port(), 8080);
  EXPECT_EQ(Config::inst().root(), "/");

  Config::inst().set_absolute_root(root_dir);
  EXPECT_EQ(Config::inst().root(), root_dir);
}


TEST_F(NginxConfigParserTest, BasicEmpty){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "basic_empty.conf"));
}


TEST_F(NginxConfigParserTest, BasicFileNonexistent){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "nonexistent_file.conf"));
}


TEST_F(NginxConfigParserTest, BasicFileInaccessible){ // Uses test fixture
  // Make the file inaccessible by changing its permissions
  std::string file_path = root_dir + "basic_good.conf";
  chmod(file_path.c_str(), 0000);
  
  EXPECT_FALSE(Config::inst().parse(file_path));

  chmod(file_path.c_str(), 0644); // Make the file accessible again
}


TEST_F(NginxConfigParserTest, BasicNoIndex){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "basic_no_index.conf"));
}


TEST_F(NginxConfigParserTest, BasicNoPort){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "basic_no_port.conf"));
}


TEST_F(NginxConfigParserTest, BasicNoRoot){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "basic_no_root.conf"));
}

// Argument testing

TEST_F(NginxConfigParserTest, ArgsInHTTPContext){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "args_in_http.conf"));
}


TEST_F(NginxConfigParserTest, ArgsInMainContext){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "args_in_main.conf"));
}


TEST_F(NginxConfigParserTest, ArgsInLocationInvalid){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "args_in_location_invalid.conf"));
}


TEST_F(NginxConfigParserTest, ArgsUnimplemented){ // Uses test fixture
  EXPECT_TRUE(Config::inst().parse(root_dir + "args_unimplemented.conf"));
}

// Comments testing

TEST_F(NginxConfigParserTest, CommentGood){ // Uses test fixture
  EXPECT_TRUE(Config::inst().parse(root_dir + "comment_good.conf"));
}


TEST_F(NginxConfigParserTest, CommentInBlockend){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "comment_in_blockend.conf"));
}


TEST_F(NginxConfigParserTest, CommentInBlockstart){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "comment_in_blockstart.conf"));
}


TEST_F(NginxConfigParserTest, CommentInIndex){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "comment_in_index.conf"));
}


TEST_F(NginxConfigParserTest, CommentInListen){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "comment_in_listen.conf"));
}


TEST_F(NginxConfigParserTest, CommentInLocation){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "comment_in_location.conf"));
}


TEST_F(NginxConfigParserTest, CommentInRoot){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "comment_in_root.conf"));
}


TEST_F(NginxConfigParserTest, CommentInTryfiles){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "comment_in_tryfiles.conf"));
}

// Context transition testing

TEST_F(NginxConfigParserTest, ContextInvalidSwitch){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "context_invalid_switch.conf"));
}

// Escape character testing

TEST_F(NginxConfigParserTest, EscapeQuotes){ // Uses test fixture
  EXPECT_TRUE(Config::inst().parse(root_dir + "escape_quotes.conf"));
  EXPECT_EQ(Config::inst().index(), "files_to_serve/index.html\\\"\'");
  EXPECT_EQ(Config::inst().root(), "/\\\"\'");
}


TEST_F(NginxConfigParserTest, EscapeWords){ // Uses test fixture
  EXPECT_TRUE(Config::inst().parse(root_dir + "escape_words.conf"));
  EXPECT_EQ(Config::inst().index(), "f\\iles_to_serve/index.html");
  EXPECT_EQ(Config::inst().root(), "\\/");
}

// Quote word testing

TEST_F(NginxConfigParserTest, QuoteDoubleIndex){ // Uses test fixture
  EXPECT_TRUE(Config::inst().parse(root_dir + "quote_double_index.conf"));
  EXPECT_EQ(Config::inst().index(), "files_to_serve/index.html");
}


TEST_F(NginxConfigParserTest, QuoteInvalidEnd){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "quote_invalid_end.conf"));
}


TEST_F(NginxConfigParserTest, QuoteSingleIndex){ // Uses test fixture
  EXPECT_TRUE(Config::inst().parse(root_dir + "quote_single_index.conf"));
  EXPECT_EQ(Config::inst().index(), "files_to_serve/index.html");
}


TEST_F(NginxConfigParserTest, QuoteValidEnds){ // Uses test fixture
  EXPECT_TRUE(Config::inst().parse(root_dir + "quote_valid_ends.conf"));
  EXPECT_EQ(Config::inst().index(), "files_to_serve/index.html");
  EXPECT_EQ(Config::inst().root(), "/");
}


TEST_F(NginxConfigParserTest, QuoteUnterminated){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "quote_unterminated.conf"));
}

// Structure testing

TEST_F(NginxConfigParserTest, StructureExtraBlockEnd){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "structure_extra_block_end.conf"));
}


TEST_F(NginxConfigParserTest, StructureMalformedLocation){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "structure_malformed_location.conf"));
}

// Token transition testing

TEST_F(NginxConfigParserTest, TransitionInvalidBlockstart){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "transition_invalid_blockstart.conf"));
}


TEST_F(NginxConfigParserTest, TransitionInvalidEOF){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "transition_invalid_eof.conf"));
}


TEST_F(NginxConfigParserTest, TransitionInvalidSemicolon){ // Uses test fixture
  EXPECT_FALSE(Config::inst().parse(root_dir + "transition_invalid_semicolon.conf"));
}
