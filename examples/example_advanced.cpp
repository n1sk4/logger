#include "logger.hpp"

int main()
{
  // Initialize with custom settings
  Logger::getInstance().init(
      "./logs/application.log", // Log file path
      LogLevel::DEBUG,          // Log level
      true,                     // Console output enabled
      1024 * 1024               // 1MB max file size
  );

  // Change settings at runtime
  Logger::getInstance().setLevel(LogLevel::WARNING);
  Logger::getInstance().setConsoleOutput(false);

  // Continue logging
  LOG_WARNING("This will be logged");
  LOG_INFO("This won't be logged due to level change");

  return 0;
}