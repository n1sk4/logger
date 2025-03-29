# Logger
A lightweight, thread-safe C++ logging library with support for multiple log levels, file rotation, and console output.

## Features
- Singleton design pattern for global access
- Thread-safe logging with mutex protection
- Multiple log levels (ERROR, WARNING, INFO, DEBUG)
- Log file rotation based on file size
- Automatic timestamp generation
- Console output option
- Configurable log file path and maximum file size
- Header-only integration with convenient macros

## Requirements
- C++23 compatible compiler
- CMake 3.31 or higher

## Building the Library
### Using CMake
```bash
# Clone the repository
git clone https://github.com/yourusername/logger.git
cd logger

# Create a build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .
```

### Integration into Your Project
Add the library to your CMake project:
```cmake
add_subdirectory(path/to/logger)
target_link_libraries(YourProject PRIVATE Logger)
```

## Usage
### Basic Usage
```cpp
#include "logger.hpp"

int main() {
    // Initialize the logger
    Logger::getInstance().init("./logs/application.log");
    
    // Log messages with different levels
    LOG_INFO("Application started");
    LOG_DEBUG("Debug information: %d", 42);
    LOG_WARNING("This is a warning message");
    LOG_ERROR("Error occurred: %s", "file not found");
    
    return 0;
}
```

### Advanced Configuration
```cpp
#include "logger.hpp"

int main() {
    // Initialize with custom settings
    Logger::getInstance().init(
        "./logs/application.log",   // Log file path
        LogLevel::DEBUG,            // Log level
        true,                       // Console output enabled
        1024 * 1024                 // 1MB max file size
    );
    
    // Change settings at runtime
    Logger::getInstance().setLevel(LogLevel::WARNING);
    Logger::getInstance().setConsoleOutput(false);
    
    // Continue logging
    LOG_WARNING("This will be logged");
    LOG_INFO("This won't be logged due to level change");
    
    return 0;
}
```

## Log Levels
The library supports the following log levels (in order of severity):

1. `LogLevel::ERR` - Critical errors that might cause application failure
2. `LogLevel::WARNING` - Warning messages that don't affect application flow
3. `LogLevel::INFO` - Informational messages about application state
4. `LogLevel::DEBUG` - Detailed debug information

Only messages with a severity level equal to or less than the current log level will be logged.

## Log Format
Log entries are formatted as:
```
[HH:MM:SS.mmm] [LEVEL] Message
```

Example:
```
[14:32:15.023] [INFO] Application started
[14:32:16.045] [ERROR] Failed to open file: data.txt
```
