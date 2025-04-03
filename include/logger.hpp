#pragma once

#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <mutex>
#include <string>
#include <cstdarg>
#include <cstring>
#include <cstdio>
#include <filesystem>

#define BUFFER_SIZE 256
#define TIME_STAMP_BUFFER 32
#define MAX_FILE_SIZE 32768
#define LOG_FILE_PATH "./logs/logger.log"
#define LOG_BUFFER_CAPACITY 100
#define FLUSH_INTERVAL_MS 1000

typedef std::mutex MutexType;

enum class LogLevel
{
  ERR = 0,
  WARNING,
  INFO,
  DEBUG
};

class Logger
{
public:
  static Logger &getInstance();

  Logger(const Logger &) = delete;
  Logger &operator=(const Logger &) = delete;
  bool init(const char *logFilePath, LogLevel level = LogLevel::INFO,
            bool consoleOutput = true, size_t maxFileSize = MAX_FILE_SIZE);

  void log(LogLevel level, const char *format, ...);
  void error(const char *format, ...);
  void warning(const char *format, ...);
  void info(const char *format, ...);
  void debug(const char *format, ...);
  void setLevel(LogLevel level);
  void setConsoleOutput(bool enable);
  void flush();

private:
  Logger();
  ~Logger();

  void lockMutex();
  void unlockMutex();
  bool openLogFile();
  void closeLogFile();
  void flushBuffer();
  bool createLogDirectory(const std::string &filePath);
  void checkRotation(size_t messageSize);
  void rotateLogFile();
  void getTimestamp(char *buffer, size_t bufferSize);
  const char *logLevelToString(LogLevel level);

  LogLevel m_currentLevel;
  std::string m_logFilePath;
  size_t m_maxFileSize;
  bool m_initialized;
  bool m_consoleOutput;
  MutexType m_logMutex;
  static const size_t m_bufferSize = BUFFER_SIZE;
  std::ofstream m_logFile;
  std::vector<std::string> m_messageBuffer;
  size_t m_currentFileSize;
  std::chrono::steady_clock::time_point m_lastFlushTime;
};

#define LOG_ERROR(...) Logger::getInstance().error(__VA_ARGS__)
#define LOG_WARNING(...) Logger::getInstance().warning(__VA_ARGS__)
#define LOG_INFO(...) Logger::getInstance().info(__VA_ARGS__)
#define LOG_DEBUG(...) Logger::getInstance().debug(__VA_ARGS__)
