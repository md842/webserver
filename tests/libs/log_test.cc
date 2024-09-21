#include "log.h"
#include "gtest/gtest.h"

// Standardized log prefix for this source
#define LOG_PRE ""

TEST(LogTest, LogDebug){
  testing::internal::CaptureStdout();
  Log::debug(LOG_PRE, "Debug");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[debug]   Debug\n");
}

TEST(LogTest, LogError){
  testing::internal::CaptureStdout();
  Log::error(LOG_PRE, "Error");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[error]   Error\n");
}

TEST(LogTest, LogFatal){
  testing::internal::CaptureStdout();
  Log::fatal(LOG_PRE, "Fatal");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[fatal]   Fatal\n");
}

TEST(LogTest, LogInfo){
  testing::internal::CaptureStdout();
  Log::info(LOG_PRE, "Info");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[info]    Info\n");
}

TEST(LogTest, LogTraceEnabled){
  Log::enable_trace(); // Enable trace logging
  testing::internal::CaptureStdout();
  Log::trace(LOG_PRE, "Trace");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[trace]   Trace\n");
}

TEST(LogTest, LogTraceSuppressed){
  testing::internal::CaptureStdout();
  Log::trace(LOG_PRE, "Suppressed");
  std::string stdout = testing::internal::GetCapturedStdout();
  EXPECT_EQ(stdout, "");
}

TEST(LogTest, LogWarn){
  testing::internal::CaptureStdout();
  Log::warn(LOG_PRE, "Warn");
  std::string stdout = testing::internal::GetCapturedStdout();
  // Cut off the timestamp and prefix of the log, as it can vary
  EXPECT_EQ(stdout.substr(50, stdout.length()), "[warning] Warn\n");
}