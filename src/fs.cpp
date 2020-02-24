#include "win.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <wchar.h>
#else
#include <dirent.h>
#endif

#include <stdexcept>
#include <cerrno>

#include "fs.hpp"
#include "path.hpp"
#include "charset.hpp"
#include "cerror.hpp"

namespace toyo {
namespace fs {

// #ifdef _WIN32
// static void win32_throw_stat_error(const std::string& p, const std::string& fn) {
//   LPVOID buf;
//   if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &buf, 0, NULL)) {
//     std::wstring msg = (wchar_t*)buf;
//     LocalFree(buf);
//     size_t pos = msg.find_last_of(L"\r\n");
//     msg = msg.substr(0, pos - 1);
//     throw std::runtime_error(toyo::charset::w2a(msg) + fn + " \"" + p + "\"");
//   } else {
//     throw std::runtime_error(fn + " \"" + p + "\"");
//   }
// }
// #endif

// static bool is_symlink(const std::string& p) noexcept {
//   std::string path = path::normalize(p);
// #ifdef _WIN32
//   std::wstring wpath = toyo::charset::a2w(path);
//   DWORD attrs = GetFileAttributesW(wpath.c_str());
//   if (attrs == INVALID_FILE_ATTRIBUTES) {
//     return false;
//   }
//   return ((attrs & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT);
// #else
//   struct stat info;
//   code = ::lstat(path.c_str(), &info);
//   if (code != 0) {
//     return false;
//   }
//   return S_ISLNK(info.st_mode);
// #endif
// }

stats::stats(const char* path, bool follow_link): stats(std::string(path), follow_link) {}

stats::stats(const std::string& p, bool follow_link) {
  std::string path = path::normalize(p);
#ifdef _WIN32
  int code = 0;
  struct _stat info;
  std::wstring wpath = toyo::charset::a2w(path);

  if (follow_link) { // stat
    code = _wstat(wpath.c_str(), &info);
    if (code != 0) {
      throw cerror(errno, std::string("stat") + " \"" + p + "\"");
    }
    this->is_link_ = false;
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
    return;
  }

  DWORD attrs = GetFileAttributesW(wpath.c_str());
  if (attrs == INVALID_FILE_ATTRIBUTES) {
    throw cerror(ENOENT, std::string("lstat") + " \"" + p + "\"");
    // win32_throw_stat_error(p, "lstat");
    return;
  }
  bool is_link = ((attrs & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT);
  if (is_link) {
    this->is_link_ = true;
    this->dev = 0;
    this->ino = 0;
    this->mode = 0;
    this->nlink = 0;
    this->gid = 0;
    this->uid = 0;
    this->rdev = 0;
    this->size = 0;
    this->atime = 0;
    this->mtime = 0;
    this->ctime = 0;
  } else {
    code = _wstat(wpath.c_str(), &info);
    if (code != 0) {
      throw cerror(errno, std::string("lstat") + " \"" + p + "\"");
    }
    this->is_link_ = false;
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
  }
#else
  int code = 0;
  struct stat info;
  if (follow_link) {
    code = ::stat(path.c_str(), &info);
    if (code != 0) {
      throw cerror(errno, std::string("stat \"") + p + "\"");
    }
  } else {
    code = ::lstat(path.c_str(), &info);
    if (code != 0) {
      throw cerror(errno, std::string("lstat \"") + p + "\"");
    }
  }

  // this->path_ = path;
  this->is_link_ = S_ISLNK(info.st_mode);
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
  return this->is_link_;
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
  bool baccess = false;
#ifdef _WIN32
  std::wstring path = toyo::charset::a2w(npath);
  baccess = (_waccess(path.c_str(), 00) == 0);
#else
  baccess = (access(npath.c_str(), F_OK) == 0);
#endif
  if (baccess) {
    return true;
  }

  try {
    fs::lstat(p);
    return true;
  } catch (const std::exception& err) {
    return false;
  }
}

stats stat(const std::string& p) {
  return stats(p, true);
}

stats lstat(const std::string& p) {
  return stats(p, false);
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
  if (fs::exists(p)) {
    if (fs::lstat(p).is_directory()) {
      return;
    } else {
      throw cerror(EEXIST, "mkdir \"" + p + "\"");
    }
  }

  std::string dir = path::dirname(p);

  if (!fs::exists(dir)) {
    fs::mkdirs(dir);
  }

  if (fs::lstat(dir).is_directory()) {
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
  if (!fs::exists(p)) {
    return;
  }
  // try {
  //   fs::lstat(p);
  // } catch (const cerror& e) {
  //   if (e.code() == ENOENT) {
  //     return;
  //   } else {
  //     throw e;
  //   }
  // }

  fs::stats stat = fs::lstat(p);
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

void symlink(const std::string& o, const std::string& n) {
  std::string oldpath = path::normalize(o);
  std::string newpath = path::normalize(n);
#ifdef _WIN32
  if (!fs::exists(oldpath)) {
    fs::symlink(o, n, symlink_type_file);
  } else if (fs::lstat(oldpath).is_directory()) {
    fs::symlink(o, n, symlink_type_directory);
  } else {
    fs::symlink(o, n, symlink_type_file);
  }
#else
  int code = ::symlink(oldpath.c_str(), newpath.c_str());
  if (code != 0) {
    throw cerror(errno, "symlink \"" + o + "\" -> \"" + n + "\"");
  }
#endif
}

#ifdef _WIN32
static void win32_throw_symlink_error(const std::string& o, const std::string& n) {
  LPVOID buf;
  if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_ALLOCATE_BUFFER, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR) &buf, 0, NULL)) {
    std::wstring msg = (wchar_t*)buf;
    LocalFree(buf);
    size_t pos = msg.find_last_of(L"\r\n");
    msg = msg.substr(0, pos - 1);
    throw std::runtime_error(toyo::charset::w2a(msg) + "symlink \"" + o + "\" -> \"" + n + "\"");
  } else {
    throw std::runtime_error("symlink \"" + o + "\" -> \"" + n + "\"");
  }
}
#endif

void symlink(const std::string& o, const std::string& n, symlink_type type) {
  std::string oldpath = path::normalize(o);
  std::string newpath = path::normalize(n);
#ifdef _WIN32
  if (type == symlink_type_directory) {
    if (!CreateSymbolicLinkW(toyo::charset::a2w(newpath).c_str(), toyo::charset::a2w(oldpath).c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY)) {
      win32_throw_symlink_error(o, n);
    }
  } else if (type == symlink_type_file) {
    if (!CreateSymbolicLinkW(toyo::charset::a2w(newpath).c_str(), toyo::charset::a2w(oldpath).c_str(), 0x0)) {
      win32_throw_symlink_error(o, n);
    }
  } else if (type == symlink_type_junction) {
    oldpath = path::resolve(oldpath);
    try {
      fs::symlink(oldpath, n);
    } catch (const std::exception&) {
      win32_throw_symlink_error(oldpath, n);
    }
  } else {
    throw std:: runtime_error("Error symlink_type, symlink \"" + o + "\" -> \"" + n + "\"");
  }
#else
  int code = ::symlink(oldpath.c_str(), newpath.c_str());
  if (code != 0) {
    throw cerror(errno, "symlink \"" + o + "\" -> \"" + n + "\"");
  }
#endif
}

}
}
