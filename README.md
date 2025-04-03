[![Logger Multiplatform Build](https://github.com/n1sk4/logger/actions/workflows/main_build.yml/badge.svg)](https://github.com/n1sk4/logger/actions/workflows/main_build.yml)

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

# Build with examples
cmake -DBUILD_EXAMPLES=ON ..

# Build with tests
cmake -DBUILD_TESTS=ON ..
```

### Running Tests
```bash
# Configure and build with tests enabled
mkdir build && cd build
cmake -DBUILD_TESTS=ON ..
cmake --build .

# Run the tests
ctest
# or run the test executable directly
./tests/logger_test
```

### Integration into Your Project
Add the library to your CMake project:
```cmake
add_subdirectory(path/to/logger)
target_link_libraries(YourProject PRIVATE Logger)
```

## Usage
### Basic Usage
https://github.com/n1sk4/logger/blob/f065a857ee806e8134fa44eee852730b9eb1b4d6/examples/example_basic.cpp#L1-L15

### Advanced Configuration
https://github.com/n1sk4/logger/blob/f065a857ee806e8134fa44eee852730b9eb1b4d6/examples/example_advanced.cpp#L1-L22

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
[YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] Message
```

Example:
```
[2022-02-19 23:45:12.023] [INFO ] Application started
[2022-02-19 23:45:20.023] [ERROR] Failed to open file: data.txt
```
