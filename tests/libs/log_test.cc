#include "log.h"
#include "gtest/gtest.h"

TEST(LogTest, LogDebug){
  testing::internal::CaptureStdout();
  Log::debug("Debug");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[debug]   Debug\n");
}

TEST(LogTest, LogError){
  testing::internal::CaptureStdout();
  Log::error("Error");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[error]   Error\n");
}

TEST(LogTest, LogFatal){
  testing::internal::CaptureStdout();
  Log::fatal("Fatal");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[fatal]   Fatal\n");
}

TEST(LogTest, LogInfo){
  testing::internal::CaptureStdout();
  Log::info("Info");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[info]    Info\n");
}

TEST(LogTest, LogTrace){
  testing::internal::CaptureStdout();
  Log::trace("Trace");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[trace]   Trace\n");
}

TEST(LogTest, LogWarn){
  testing::internal::CaptureStdout();
  Log::warn("Warn");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[warning] Warn\n");
}