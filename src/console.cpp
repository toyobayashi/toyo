#include "console.hpp"

namespace toyo {

#ifdef _WIN32
WORD console::_set_console_text_attribute(HANDLE hConsole, WORD wAttr) {
  CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
  if (!GetConsoleScreenBufferInfo(hConsole, &csbiInfo)) return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
  WORD originalAttr = csbiInfo.wAttributes;
  SetConsoleTextAttribute(hConsole, wAttr);
  return originalAttr;
}
#endif

std::string console::_format(char c) {
  char buf[2] = { c, '\0' };
  return std::string(buf);
}

std::string console::_format(bool b) {
  return b ? "true" : "false";
}

std::string console::_format(const char* cstr) {
  return charset::a2ocp(cstr);
}

std::string console::_format(const std::vector<unsigned char>& buf) {
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

std::string console::_format(const std::vector<std::string>& arr) {
  size_t len = arr.size();
  std::ostringstream oss;
  if (len == 0) {
    oss << "[]";
    return oss.str();
  }
  oss << "[ ";
  for (size_t i = 0; i < len; i++) {
    oss << "\"" << charset::a2ocp(arr[i]) << "\"";
    if (i != len - 1) {
      oss << ", ";
    }
  }
  oss << " ]";
  return oss.str();
}

std::string console::_format(const std::map<std::string, std::string>& strobj) {
  size_t len = strobj.size();
  std::ostringstream oss;
  if (len == 0) {
    oss << "{}";
    return oss.str();
  }
  oss << "{\n";
  size_t i = 0;
  for (auto& p : strobj) {
    oss << charset::a2ocp(std::string("  \"") + p.first + "\": \"" + p.second + "\"");
    if (i != len - 1) {
      oss << ",\n";
    } else {
      oss << "\n";
    }
    i++;
  }
  oss << "}";
  return oss.str();
}

std::string console::_format(const std::string& str) {
  return charset::a2ocp(str);
}

void console::_logerror(const char* arg) {
  std::cerr << _format(arg) << std::endl;
}

void console::_logerror(const std::string& arg) {
  std::cerr << _format(arg) << std::endl;
}

void console::log(const char* arg) {
  std::cout << _format(arg) << std::endl;
}

void console::log(const std::string& arg) {
  std::cout << _format(arg) << std::endl;
}

void console::write(const char* arg) {
  std::cout << _format(arg);
}

void console::write(const std::string& arg) {
  std::cout << _format(arg);
}

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

void console::clear_line(short lineNumber) {
#ifdef _WIN32
  HANDLE _consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) return;
  short tmp = csbi.dwCursorPosition.Y - lineNumber;
  COORD targetFirstCellPosition = { 0, tmp < 0 ? 0 : tmp };
  DWORD size = csbi.dwSize.X * (lineNumber + 1);
  DWORD cCharsWritten;

  if (!FillConsoleOutputCharacterW(_consoleHandle, L' ', size, targetFirstCellPosition, &cCharsWritten)) return;
  if (!GetConsoleScreenBufferInfo(_consoleHandle, &csbi)) return;
  if (!FillConsoleOutputAttribute(_consoleHandle, csbi.wAttributes, size, targetFirstCellPosition, &cCharsWritten)) return;
  SetConsoleCursorPosition(_consoleHandle, targetFirstCellPosition);
#else
  for (short i = 0; i < lineNumber; i++) {
    std::cout << "\x1b[666D\x1b[0K\x1b[1A";
  }
  std::cout << "\x1b[666D\x1b[0K";
#endif
}

}
