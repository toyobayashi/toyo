#include "win.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <wchar.h>
#else
#include <unistd.h>
#include <dirent.h>
#endif

#include <cerrno>

#include "fs.hpp"
#include "path.hpp"
#include "charset.hpp"
#include "cerror.hpp"

namespace toyo {
namespace fs {

std::vector<std::string> readdir(const std::string& p) {
#ifdef _WIN32
  struct _wfinddata_t file;
  intptr_t hFile;
  std::string newPath = toyo::path::normalize(p);

  hFile = _wfindfirst(toyo::charset::a2w(toyo::path::win32::join(newPath, "*.*")).c_str(), &file);
  if (hFile == -1) {
    throw cerror(errno, std::string("scandir \"") + newPath + "\"");
  }

  std::vector<std::string> res;
  std::wstring item = file.name;

  if (item != L"." && item != L"..") {
    res.push_back(toyo::charset::w2a(item));
  }

  while (_wfindnext(hFile, &file) == 0) {
    item = file.name;
    if (item != L"." && item != L"..") {
      res.push_back(toyo::charset::w2a(item));
    }
  }
  _findclose(hFile);

  return res;
#else
  std::string newPath = toyo::path::normalize(p);
  DIR *dirp;
  struct dirent *direntp;
  int stats;
  if ((dirp = opendir(newPath.c_str())) == nullptr) {
    throw cerror(errno, std::string("scandir \"") + newPath + "\"");
  }
  std::vector<std::string> res;
  std::string item;
  while ((direntp = readdir(dirp)) != NULL) {
    item = direntp->d_name;
    if (item != "." && item != "..") {
      res.push_back(item);
    }
  }
  closedir(dirp);
  return res;
#endif
}

bool exists(const std::string& p) {
#ifdef _WIN32
  std::wstring path = toyo::charset::a2w(p);
  return (_waccess(path.c_str(), 00) == 0);
#else
  return (access(p.c_str(), F_OK) == 0);
#endif
}

}
}
