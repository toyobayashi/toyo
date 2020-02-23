#include "win.h"

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#else
#include <unistd.h>
#endif

#include <cstdlib>

#include "process.hpp"
#include "charset.hpp"

namespace toyo {

namespace process {

std::string cwd() {
#ifdef _WIN32
  wchar_t* buf;
  if ((buf = _wgetcwd(nullptr, 0)) == nullptr) {
    return "";
  }
  std::wstring res = buf;
  free(buf);
  return toyo::charset::w2a(res);
#else
  char* buf;
  if ((buf = getcwd(nullptr, 0)) == nullptr) {
    return "";
  }
  std::string res = buf;
  free(buf);
  return res;
#endif
}

int pid() {
#ifdef _WIN32
  return _getpid();
#else
  return getpid();
#endif
}

} // process

} // toyo
