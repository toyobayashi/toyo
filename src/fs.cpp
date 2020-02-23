#include "win.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <wchar.h>
#else
#include <dirent.h>
#endif

#include <cerrno>

#include "fs.hpp"
#include "path.hpp"
#include "charset.hpp"
#include "cerror.hpp"

namespace toyo {
namespace fs {

stats::stats(const char* path): stats(std::string(path)) {}

stats::stats(const std::string& p) {
  std::string path = path::normalize(p);
#ifdef _WIN32
  int code = 0;
  struct _stat info;
  code = _wstat(toyo::charset::a2w(path).c_str(), &info);
  if (code != 0) {
    throw cerror(errno, std::string("stat \"") + p + "\"");
  }
  this->path_ = path;
  this->dev = info.st_dev;
  this->ino = info.st_ino;
  this->mode = info.st_mode;
  this->nlink = info.st_nlink;
  this->gid = info.st_gid;
  this->uid = info.st_uid;
  this->rdev = info.st_rdev;
  this->size = info.st_size;
  this->atime = info.st_atime;
  this->mtime = info.st_mtime;
  this->ctime = info.st_ctime;
#else
  int code = 0;
  struct stat info;
  code = stat(path.c_str(), &info);
  if (code != 0) {
    throw cerror(errno, std::string("stat \"") + p + "\"");
  }
  this->path_ = path;
  this->dev = info.st_dev;
  this->ino = info.st_ino;
  this->mode = info.st_mode;
  this->nlink = info.st_nlink;
  this->gid = info.st_gid;
  this->uid = info.st_uid;
  this->rdev = info.st_rdev;
  this->size = info.st_size;
  this->atime = info.st_atime;
  this->mtime = info.st_mtime;
  this->ctime = info.st_ctime;
#endif
}

bool stats::is_file() const {
#ifdef _WIN32
  return ((this->mode & S_IFMT) == S_IFREG);
#else
  return S_ISREG(this->mode);
#endif
}

bool stats::is_directory() const {
#ifdef _WIN32
  return ((this->mode & S_IFMT) == S_IFDIR);
#else
  return S_ISDIR(this->mode);
#endif
}

bool stats::is_fifo() const {
#ifdef _WIN32
  return ((this->mode & S_IFMT) == _S_IFIFO);
#else
  return S_ISFIFO(this->mode);
#endif
}

bool stats::is_character_device() const {
#ifdef _WIN32
  return ((this->mode & S_IFMT) == S_IFCHR);
#else
  return S_ISCHR(this->mode);
#endif
}

bool stats::is_symbolic_link() const {
#ifdef _WIN32
  DWORD attrs = GetFileAttributesW(toyo::charset::a2w(this->path_).c_str());
  if (attrs == INVALID_FILE_ATTRIBUTES) {
    throw cerror(errno, std::string("stat \"") + this->path_ + "\"");
  }
  return ((attrs & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT);
#else
  return S_ISLNK(this->mode);
#endif
}

bool stats::is_block_device() const {
#ifdef _WIN32
  return false;
#else
  return S_ISBLK(this->mode);
#endif
}

bool stats::is_socket() const {
#ifdef _WIN32
  return false;
#else
  return S_ISSOCK(this->mode);
#endif
}

std::vector<std::string> readdir(const std::string& p) {
#ifdef _WIN32
  struct _wfinddata_t file;
  intptr_t hFile;
  std::string newPath = toyo::path::normalize(p);

  hFile = _wfindfirst(toyo::charset::a2w(toyo::path::win32::join(newPath, "*.*")).c_str(), &file);
  if (hFile == -1) {
    throw cerror(errno, std::string("scandir \"") + p + "\"");
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
    throw cerror(errno, std::string("scandir \"") + p + "\"");
  }
  std::vector<std::string> res;
  std::string item;
  while ((direntp = readdir(dirp)) != nullptr) {
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
  std::string npath = path::normalize(p);
#ifdef _WIN32
  std::wstring path = toyo::charset::a2w(npath);
  return (_waccess(path.c_str(), 00) == 0);
#else
  return (access(npath.c_str(), F_OK) == 0);
#endif
}

stats stat(const std::string& p) {
  return stats(p);
}

void mkdir(const std::string& p, int mode) {
  int code = 0;
  std::string path = path::normalize(p);
#ifdef _WIN32
  code = _wmkdir(toyo::charset::a2w(path).c_str());
#else
  code = ::mkdir(path.c_str(), mode);
#endif
  if (code != 0) {
    throw cerror(errno, "mkdir \"" + p + "\"");
  }
}

void mkdirs(const std::string& p, int mode) {
  if (exists(p)) {
    if (stat(p).is_directory()) {
      return;
    } else {
      throw cerror(EEXIST, "mkdir \"" + p + "\"");
    }
  }

  std::string dir = path::dirname(p);

  if (!exists(dir)) {
    mkdirs(dir);
  }

  if (stat(dir).is_directory()) {
    fs::mkdir(p, mode);
  } else {
    throw cerror(ENOENT, "mkdir \"" + p + "\"");
  }
}

void unlink(const std::string& p) {
  int code = 0;
  std::string path = path::normalize(p);
#ifdef _WIN32
  code = _wunlink(toyo::charset::a2w(path).c_str());
#else
  code = ::unlink(path.c_str());
#endif
  if (code != 0) {
    throw cerror(errno, "unlink \"" + p + "\"");
  }
}

void rmdir(const std::string& p) {
  int code = 0;
  std::string path = path::normalize(p);
#ifdef _WIN32
  code = _wrmdir(toyo::charset::a2w(path).c_str());
#else
  code = ::rmdir(path.c_str());
#endif
  if (code != 0) {
    throw cerror(errno, "rmdir \"" + p + "\"");
  }
}

void rename(const std::string& s, const std::string& d) {
  int code = 0;
  std::string source = path::normalize(s);
  std::string dest = path::normalize(d);
#ifdef _WIN32
  code = _wrename(toyo::charset::a2w(source).c_str(), toyo::charset::a2w(dest).c_str());
#else
  code = ::rename(source.c_str(), dest.c_str());
#endif
  if (code != 0) {
    throw cerror(errno, "rename \"" + s + "\" -> \"" + d + "\"");
  }
}

void remove(const std::string& p) {
  if (!exists(p)) {
    return;
  }

  fs::stats stat = fs::stat(p);
  if (stat.is_directory()) {
    std::vector<std::string> items = fs::readdir(p);
    if (items.size() != 0) {
      for (size_t i = 0; i < items.size(); i++) {
        const std::string& item = items[i];
        fs::remove(path::join(p, item));
      }
      fs::rmdir(p);
    } else {
      fs::rmdir(p);
    }
  } else {
    fs::unlink(p);
  }
}

}
}
