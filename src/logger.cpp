#include "logger.hpp"

Logger &Logger::getInstance()
{
  static Logger instance;
  return instance;
}

Logger::Logger()
    : m_currentLevel(LogLevel::INFO),
      m_logFilePath(LOG_FILE_PATH),
      m_maxFileSize(MAX_FILE_SIZE),
      m_initialized(false),
      m_consoleOutput(true),
      m_lastFlushTime(std::chrono::steady_clock::now())
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
  m_messageBuffer.reserve(LOG_BUFFER_CAPACITY);

  if (!createLogDirectory(m_logFilePath))
  {
    std::cerr << "Failed to create log directory for: " << m_logFilePath << "\n";
    return false;
  }

  if (!openLogFile())
  {
    std::cerr << "Failed to initialize logger file: " << m_logFilePath << "\n";
    return false;
  }

  m_currentFileSize = static_cast<size_t>(m_logFile.tellp());

  char timestampBuffer[64];
  getTimestamp(timestampBuffer, sizeof(timestampBuffer));

  std::string initMessage = "[" + std::string(timestampBuffer) + "] [INFO] Logger initialized\n";
  m_logFile << initMessage;
  m_currentFileSize += initMessage.size();
  m_logFile.flush();

  m_initialized = true;
  return true;
}

Logger::~Logger()
{
  if (m_initialized)
  {
    char timestampBuffer[TIME_STAMP_BUFFER];
    getTimestamp(timestampBuffer, TIME_STAMP_BUFFER);

    flushBuffer();

    std::string shutdownMsg = "[" + std::string(timestampBuffer) + "] [INFO] Logger shutdown\n";
    m_logFile << shutdownMsg;
    m_logFile.flush();

    closeLogFile();
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

  std::ostringstream logStream;
  char buffer[m_bufferSize];
  char timestampBuffer[TIME_STAMP_BUFFER];

  getTimestamp(timestampBuffer, sizeof(timestampBuffer));

  va_list args;
  va_start(args, format);
  vsnprintf(buffer, m_bufferSize, format, args);
  va_end(args);

  lockMutex();

  logStream << "[" << timestampBuffer << "] [" << logLevelToString(level) << "] " << buffer << "\n";
  std::string logEntry = logStream.str();

  if (m_consoleOutput)
  {
    std::cout << logEntry;
  }

  if (!openLogFile())
  {
    unlockMutex();
    return;
  }

  checkRotation(logEntry.size());

  m_messageBuffer.push_back(std::move(logEntry));
  m_currentFileSize += logEntry.size();

  auto now = std::chrono::steady_clock::now();
  auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastFlushTime).count();

  if (m_messageBuffer.size() >= LOG_BUFFER_CAPACITY || elapsedMs >= FLUSH_INTERVAL_MS)
  {
    flushBuffer();
  }

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

bool Logger::openLogFile()
{
  if (m_logFile.is_open())
  {
    return true;
  }

  m_logFile.open(m_logFilePath, std::ios_base::app | std::ios_base::out);

  if (!m_logFile.is_open())
  {
    m_logFile.open(m_logFilePath, std::ios_base::out);
    if (!m_logFile.is_open())
      return false;
  }

  m_logFile.seekp(0, std::ios_base::end);
  m_currentFileSize = static_cast<size_t>(m_logFile.tellp());

  return true;
}

void Logger::closeLogFile()
{
  if (m_logFile.is_open())
    m_logFile.close();
}

void Logger::flushBuffer()
{
  for (const auto &message : m_messageBuffer)
  {
    m_logFile << message;
  }

  m_logFile.flush();
  m_messageBuffer.clear();
}

void Logger::flush()
{
  lockMutex();
  flushBuffer();
  unlockMutex();
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
  if (std::filesystem::exists(backupFileName))
  {
    std::error_code ec;
    std::filesystem::remove(backupFileName, ec);
    if (ec)
    {
      std::cerr << "Failed to remove backup file: " << ec.message() << "\n";
    }
  }

  std::error_code ec;
  std::filesystem::rename(m_logFilePath, backupFileName, ec);
  if (ec)
  {
    std::cerr << "Failed to rename log file: " << ec.message() << "\n";
  }
}

void Logger::checkRotation(size_t messageSize)
{
  if (m_currentFileSize + messageSize > m_maxFileSize)
  {
    for (const auto &message : m_messageBuffer)
    {
      m_logFile << message;
    }
    m_messageBuffer.clear();

    closeLogFile();
    rotateLogFile();
    openLogFile();

    m_currentFileSize = 0;
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
  localtime_r(&now_time_t, &now_tm);
#endif
  snprintf(buffer, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
           now_tm.tm_year + 1900,
           now_tm.tm_mon + 1,
           now_tm.tm_mday,
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