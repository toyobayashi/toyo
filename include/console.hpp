#ifndef __CONSOLE_HPP__
#define __CONSOLE_HPP__

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include "Windows.h"

  #define COLOR_RED_BRIGHT (FOREGROUND_RED | FOREGROUND_INTENSITY)
  #define COLOR_YELLOW_BRIGHT (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
  #define COLOR_GREEN_BRIGHT (FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#else
#define COLOR_RED_BRIGHT ("\x1b[31;1m")
#define COLOR_YELLOW_BRIGHT ("\x1b[33;1m")
#define COLOR_GREEN_BRIGHT ("\x1b[32;1m")
#define COLOR_RESET ("\x1b[0m")
#endif

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include "charset.hpp"

namespace toyo {

class console {
private:
#ifdef _WIN32
  static WORD _set_console_text_attribute(HANDLE, WORD);
#endif

  static std::string _format(char);

  static std::string _format(bool);

  static std::string _format(const char*);

  static std::string _format(const std::vector<unsigned char>&);

  static std::string _format(const std::vector<std::string>&);

  static std::string _format(const std::string&);

  template <typename T>
  static std::string _format(const std::vector<T>& arr) {
    size_t len = arr.size();
    std::ostringstream oss;
    if (len == 0) {
      oss << "[]";
      return oss.str();
    }
    oss << "[ ";
    for (size_t i = 0; i < arr.size(); i++) {
      oss << arr[i];
      if (i != arr.size() - 1) {
        oss << ", ";
      }
    }
    oss << " ]";
    return oss.str();
  }

  template <typename T>
  static std::string _format(const T& arg) {
    std::ostringstream oss;
    oss << arg;
    return oss.str();
  }

  static void _logerror(const char*);

  static void _logerror(const std::string&);

  template <typename... Args>
  static void _logerror(const std::string& format, Args... args) {
    std::string f = toyo::charset::a2ocp(format) + "\n";
    fprintf(stderr, f.c_str(), args...);
  }

  template <typename T>
  static void _logerror(const T& arg) {
    std::cerr << _format(arg) << std::endl;
  }

public:
  console() = delete;
  console(const console&) = delete;

  static void write(const char*);

  static void write(const std::string&);

  template <typename... Args>
  static void write(const std::string& format, Args... args) {
    printf(toyo::charset::a2ocp(format).c_str(), args...);
  }

  template <typename T>
  static void write(const T& arg) {
    std::cout << _format(arg);
  }

  static void log(const char* arg);

  static void log(const std::string& arg);

  template <typename... Args>
  static void log(const std::string& format, Args... args) {
    std::string f = toyo::charset::a2ocp(format) + "\n";
    printf(f.c_str(), args...);
  }

  template <typename T>
  static void log(const T& arg) {
    std::cout << _format(arg) << std::endl;
  }

  template <typename T>
  static void info(const T& arg) {
#ifdef _WIN32
    HANDLE hconsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD original = _set_console_text_attribute(hconsole, COLOR_GREEN_BRIGHT);
    log(arg);
    _set_console_text_attribute(hconsole, original);
#else
    std::cout << COLOR_GREEN_BRIGHT;
    log(arg);
    std::cout << COLOR_RESET;
#endif
  }

  template <typename T>
  static void warn(const T& arg) {
#ifdef _WIN32
    HANDLE hconsole = GetStdHandle(STD_ERROR_HANDLE);
    WORD original = _set_console_text_attribute(hconsole, COLOR_YELLOW_BRIGHT);
    _logerror(arg);
    _set_console_text_attribute(hconsole, original);
#else
    std::cerr << COLOR_YELLOW_BRIGHT;
    _logerror(arg);
    std::cerr << COLOR_RESET;
#endif
  }

  template <typename T>
  static void error(const T& arg) {
#ifdef _WIN32
    HANDLE hconsole = GetStdHandle(STD_ERROR_HANDLE);
    WORD original = _set_console_text_attribute(hconsole, COLOR_RED_BRIGHT);
    log(arg);
    _set_console_text_attribute(hconsole, original);
#else
    std::cerr << COLOR_RED_BRIGHT;
    _logerror(arg);
    std::cerr << COLOR_RESET;
#endif
  }

  static void clear();
  static void clear_line(short lineNumber = 0);
}; // class console

} // namespace toyo

#endif
