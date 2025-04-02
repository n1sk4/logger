#include "logger.hpp"

Logger &Logger::getInstance()
{
  static Logger instance;
  return instance;
}

Logger::Logger()
    : m_currentLevel(LogLevel::INFO),
      m_logFilePath(""),
      m_maxFileSize(MAX_FILE_SIZE),
      m_initialized(false),
      m_consoleOutput(true)
{
}

bool Logger::init(const char *logFilePath, LogLevel level, bool consoleOutput, size_t maxFileSize)
{
  if (m_initialized)
  {
    return true;
  }

  m_logFilePath = logFilePath;
  m_currentLevel = level;
  m_consoleOutput = consoleOutput;
  m_maxFileSize = maxFileSize;

  if (!createLogDirectory(m_logFilePath))
  {
    std::cerr << "Failed to create log directory for: " << m_logFilePath << std::endl;
    return false;
  }

  std::ofstream logFile(m_logFilePath, std::ios_base::app);
  if (!logFile.is_open())
  {
    std::cerr << "Failed to initialize logger file: " << m_logFilePath << std::endl;
    return false;
  }

  char timestampBuffer[64];
  getTimestamp(timestampBuffer, sizeof(timestampBuffer));
  logFile << "[" << timestampBuffer << "] [INFO] Logger initialized" << std::endl;
  logFile.close();

  m_initialized = true;
  return true;
}

Logger::~Logger()
{
  if (m_initialized)
  {
    char timestampBuffer[TIME_STAMP_BUFFER];
    getTimestamp(timestampBuffer, TIME_STAMP_BUFFER);

    std::ofstream logFile(m_logFilePath, std::ios_base::app);
    if (logFile.is_open())
    {
      logFile << "[" << timestampBuffer << "] [INFO] Logger shutdown" << std::endl;
      logFile.close();
    }
  }
}

void Logger::error(const char *format, ...)
{
  if (!m_initialized || LogLevel::ERR > m_currentLevel)
  {
    return;
  }

  char buffer[m_bufferSize];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, m_bufferSize, format, args);
  va_end(args);

  log(LogLevel::ERR, "%s", buffer);
}

void Logger::warning(const char *format, ...)
{
  if (!m_initialized || LogLevel::WARNING > m_currentLevel)
  {
    return;
  }

  char buffer[m_bufferSize];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, m_bufferSize, format, args);
  va_end(args);

  log(LogLevel::WARNING, "%s", buffer);
}

void Logger::info(const char *format, ...)
{
  if (!m_initialized || LogLevel::INFO > m_currentLevel)
  {
    return;
  }

  char buffer[m_bufferSize];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, m_bufferSize, format, args);
  va_end(args);

  log(LogLevel::INFO, "%s", buffer);
}

void Logger::debug(const char *format, ...)
{
  if (!m_initialized || LogLevel::DEBUG > m_currentLevel)
  {
    return;
  }

  char buffer[m_bufferSize];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, m_bufferSize, format, args);
  va_end(args);

  log(LogLevel::DEBUG, "%s", buffer);
}

void Logger::log(LogLevel level, const char *format, ...)
{
  if (!m_initialized || level > m_currentLevel)
  {
    return;
  }

  char buffer[m_bufferSize];
  char timestampBuffer[TIME_STAMP_BUFFER];

  getTimestamp(timestampBuffer, sizeof(timestampBuffer));

  va_list args;
  va_start(args, format);
  vsnprintf(buffer, m_bufferSize, format, args);
  va_end(args);

  lockMutex();

  char logEntry[m_bufferSize + 64];

  snprintf(
      logEntry, sizeof(logEntry),
      "[%s] [%s] %s\n",
      timestampBuffer, logLevelToString(level), buffer);

  if (m_consoleOutput)
  {
    std::cout << logEntry;
  }

  std::ofstream logFile;
  logFile.open(m_logFilePath, std::ios_base::app);

  if (!logFile.is_open())
  {
    logFile.open(m_logFilePath, std::ios_base::out);
    if (!logFile.is_open())
    {
      unlockMutex();
      return;
    }
  }

  logFile.seekp(0, std::ios_base::end);
  if (static_cast<size_t>(logFile.tellp()) + strlen(logEntry) > m_maxFileSize)
  {
    logFile.close();
    rotateLogFile();
    logFile.open(m_logFilePath, std::ios_base::out);
    if (!logFile.is_open())
    {
      unlockMutex();
      return;
    }
  }

  logFile << logEntry;
  logFile.close();

  unlockMutex();
}

void Logger::lockMutex()
{
  static_cast<std::mutex *>(&m_logMutex)->lock();
}

void Logger::unlockMutex()
{
  static_cast<std::mutex *>(&m_logMutex)->unlock();
}

bool Logger::createLogDirectory(const std::string &filePath)
{
  std::filesystem::path path(filePath);
  std::filesystem::path dir = path.parent_path();

  if (dir.empty())
  {
    return true;
  }

  try
  {
    if (!std::filesystem::exists(dir))
    {
      return std::filesystem::create_directories(dir);
    }
    return true;
  }
  catch (const std::filesystem::filesystem_error &e)
  {
    std::cerr << "Error creating directory: " << e.what() << std::endl;
    return false;
  }
}

void Logger::rotateLogFile()
{
  std::string backupFileName = std::string(m_logFilePath) + ".bak";
  if(std::filesystem::exists(backupFileName))
  {
    std::error_code ec;
    std::filesystem::remove(backupFileName, ec);
    if(ec)
    {
      std::cerr << "Failed to remove backup file: " << ec.message() << "\n";
    }
  }

  std::error_code ec;
  std::filesystem::rename(m_logFilePath, backupFileName, ec);
  if(ec)
  {
    std::cerr << "Failed to rename log file: " << ec.message() << "\n";
  }
}

void Logger::getTimestamp(char *buffer, size_t bufferSize)
{
  auto now = std::chrono::system_clock::now();
  auto now_time_t = std::chrono::system_clock::to_time_t(now);
  auto now_ms = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()) % 1000;
  std::tm now_tm;

#ifdef _WIN32
  localtime_s(&now_tm, &now_time_t);
#else
  localtime_r(&now_tm, &now_time_t);
#endif
  snprintf(buffer, bufferSize, "%02d:%02d:%02d.%03ld",
           now_tm.tm_hour,
           now_tm.tm_min,
           now_tm.tm_sec,
           (long)now_ms.count());
}

const char *Logger::logLevelToString(LogLevel level)
{
  switch (level)
  {
  case LogLevel::ERR:
    return "ERROR";
  case LogLevel::WARNING:
    return "WARN ";
  case LogLevel::INFO:
    return "INFO ";
  case LogLevel::DEBUG:
    return "DEBUG";
  default:
    return "?????";
  }
}

void Logger::setLevel(LogLevel level)
{
  m_currentLevel = level;
}

void Logger::setConsoleOutput(bool enable)
{
  m_consoleOutput = enable;
}