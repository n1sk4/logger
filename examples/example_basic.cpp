#include "logger.hpp"

int main()
{
  // Initialize the logger
  Logger::getInstance().init("./logs/application.log");

  // Log messages with different levels
  LOG_INFO("Application started");
  LOG_DEBUG("Debug information: %d", 42);
  LOG_WARNING("This is a warning message");
  LOG_ERROR("Error occurred: %s", "file not found");

  return 0;
}