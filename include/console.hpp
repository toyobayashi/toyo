#ifndef __CONSOLE_HPP__
#define __CONSOLE_HPP__

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "Windows.h"
#endif

#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include "string.hpp"

namespace toyo {

class console {
private:
  console();

  static std::string _format(char c) {
    char buf[2] = { c, '\0' };
    return std::string(buf);
  }

  static std::string _format(bool b) {
    return b ? "true" : "false";
  }

  static std::string _format(const char* cstr) {
    toyo::string tstr(cstr);

    char* tmp = tstr.c_stro();
    std::string res(tmp);
    toyo::free(tmp);

    return res;
  }

  static std::string _format(const std::vector<unsigned char>& buf) {
    char _map[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    std::ostringstream oss;
    oss << "<Buffer ";
    for (size_t i = 0; i < buf.size(); i++) {
      oss << _map[buf[i] >> 4];
      oss << _map[buf[i] & 0x0f];
      if (i != buf.size() - 1) {
        oss << " ";
      }
    }
    oss << ">";
    return oss.str();
  }

  static std::string _format(const std::vector<std::string>& arr) {
    size_t len = arr.size();
    std::ostringstream oss;
    if (len == 0) {
      oss << "[]";
      return oss.str();
    }
    oss << "[ ";
    for (size_t i = 0; i < len; i++) {
      toyo::string tstr(arr[i].c_str());

      char* tmp = tstr.c_stro();
      std::string res(tmp);
      toyo::free(tmp);

      oss << "\"" << res << "\"";
      if (i != len - 1) {
        oss << ", ";
      }
    }
    oss << " ]";
    return oss.str();
  }

  static std::string _format(const std::vector<toyo::string>& arr) {
    size_t len = arr.size();
    std::ostringstream oss;
    if (len == 0) {
      oss << "[]";
      return oss.str();
    }
    oss << "[ ";
    for (size_t i = 0; i < len; i++) {
      char* tmp = arr[i].c_stro();
      std::string res(tmp);
      toyo::free(tmp);
      oss << "\"" << res << "\"";
      if (i != len - 1) {
        oss << ", ";
      }
    }
    oss << " ]";
    return oss.str();
  }

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

  static std::string _format(const std::string& str) {
    toyo::string tstr(str.c_str());

    char* tmp = tstr.c_stro();
    std::string res(tmp);
    toyo::free(tmp);

    return res;
  }

  static std::string _format(const toyo::string& str) {
    return str.c_str();
  }

  template <typename T>
  static std::string _format(const T& arg) {
    std::ostringstream oss;
    oss << arg;
    return oss.str();
  }

public:
  static void write(const char* arg) {
    std::cout << _format(arg);
  }

  static void write(const std::string& arg) {
    std::cout << _format(arg);
  }

  template <typename... Args>
  static void write(const std::string& format, Args... args) {
    toyo::string tstr(format.c_str());
    char* tmp = tstr.c_stro();
    printf(tmp, args...);
    toyo::free(tmp);
  }

  template <typename T>
  static void write(const T& arg) {
    std::cout << _format(arg);
  }

  static void log(const char* arg) {
    std::cout << _format(arg) << std::endl;
  }

  static void log(const std::string& arg) {
    std::cout << _format(arg) << std::endl;
  }

  template <typename... Args>
  static void log(const std::string& format, Args... args) {
    write(format, args...);
    printf("\n");
  }

  template <typename T>
  static void log(const T& arg) {
    std::cout << _format(arg) << std::endl;
  }

  static void clear();
}; // class console

void console::clear() {
#ifdef _WIN32
  HANDLE _consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD coordScreen = { 0, 0 };
  DWORD cCharsWritten;
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  DWORD dwConSize;

  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) {
    return;
  }

  dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

  if (!FillConsoleOutputCharacter(_consoleHandle,
    (TCHAR) ' ',
    dwConSize,
    coordScreen,
    &cCharsWritten))
  {
    return;
  }

  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) {
    return;
  }

  if (!FillConsoleOutputAttribute(_consoleHandle,
    csbi.wAttributes,
    dwConSize,
    coordScreen,
    &cCharsWritten))
  {
    return;
  }

  SetConsoleCursorPosition(_consoleHandle, coordScreen);
#else
  std::cout << "\033[2J\033[1;1H";
#endif
}

} // namespace toyo

#endif
