#include "win.h"

#ifdef _WIN32

#include "string.hpp"
#include <cstring>

int get_last_error(char* message, int* size) {
  int code = GetLastError();
  if (!message && !size) {
    return code;
  }

  LPVOID buf;
  if (FormatMessage(
    FORMAT_MESSAGE_FROM_SYSTEM
    | FORMAT_MESSAGE_IGNORE_INSERTS
    | FORMAT_MESSAGE_ALLOCATE_BUFFER,
    NULL,
    code,
    GetSystemDefaultLangID() /* MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT) */,
    (LPWSTR) &buf,
    0,
    NULL
  )) {
    std::wstring msg = (wchar_t*)buf;
    LocalFree(buf);
    size_t pos = msg.find_last_of(L"\r\n");
    msg = msg.substr(0, pos - 1);
    toyo::string tstr = msg.c_str();

    if (!message) {
      if (size) {
        *size = tstr.byte_length() + 1;
      }
    } else {
      if (size) {
        memcpy(message, tstr.c_str(), *size);
      } else {
        strcpy(message, tstr.c_str());
      }
    }
  }
  return code;
}

#endif
