#include "win.h"

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#include <wchar.h>
#else

#endif

#include <utility>
#include <stdexcept>
#include <cerrno>
#include <cstring>

#include "fs.hpp"
#include "path.hpp"
#include "charset.hpp"
#include "cerror.hpp"

namespace toyo {
namespace fs {

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

#ifdef _WIN32
dirent::dirent(): dirent_(nullptr) {}
dirent::~dirent() {
  if (dirent_) {
    delete dirent_;
    dirent_ = nullptr;
  }
}
dirent::dirent(struct _wfinddata_t* d): dirent() {
  if (d) {
    dirent_ = new struct _wfinddata_t;
    memcpy(dirent_, d, sizeof(struct _wfinddata_t));
  }
}

dirent::dirent(const dirent& d): dirent(d.dirent_) {}

dirent::dirent(dirent&& d) {
  dirent_ = d.dirent_;
  d.dirent_ = nullptr;
}

bool dirent::is_empty() const { return dirent_ == nullptr; }

const struct _wfinddata_t* dirent::data() { return dirent_; }

std::string dirent::name() const {
  if (dirent_) {
    return toyo::charset::w2a(dirent_->name);
  } else {
    return "";
  }
}

bool dirent::is_file() const {
  if (dirent_) {
    return ((dirent_->attrib & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) && ((dirent_->attrib & FILE_ATTRIBUTE_REPARSE_POINT) != FILE_ATTRIBUTE_REPARSE_POINT);
  } else {
    return false;
  }
}
bool dirent::is_directory() const {
  if (dirent_) {
    return ((dirent_->attrib & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
  } else {
    return false;
  }
}
bool dirent::is_fifo() const {
  return false;
}
bool dirent::is_character_device() const {
  return false;
}
bool dirent::is_symbolic_link() const {
  if (dirent_) {
    return ((dirent_->attrib & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT);
  } else {
    return false;
  }
}
bool dirent::is_block_device() const {
  return false;
}
bool dirent::is_socket() const {
  return false;
}

dirent& dirent::operator=(const dirent& d) {
  if (&d == this) return *this;

  if (d.dirent_) {
    if (!dirent_) {
      dirent_ = new struct _wfinddata_t;
    }
    memcpy(dirent_, d.dirent_, sizeof(struct _wfinddata_t));
  } else {
    if (dirent_) {
      delete dirent_;
      dirent_ = nullptr;
    }
  }
  return *this;
}

dirent& dirent::operator=(dirent&& d) {
  if (dirent_) {
    delete dirent_;
    dirent_ = nullptr;
  }
  dirent_ = d.dirent_;
  d.dirent_ = nullptr;
  return *this;
}

dir::dir(const dir& d) {
  dir_ = d.dir_;
  path_ = d.path_;
  if (d.first_data_) {
    first_data_ = new struct _wfinddata_t;
    memcpy(first_data_, d.first_data_, sizeof(struct _wfinddata_t));
  } else {
    first_data_ = nullptr;
  }
}

dir::dir(dir&& d) {
  dir_ = std::exchange(d.dir_, -1);
  path_ = std::move(d.path_);
  first_data_ = d.first_data_;
  d.first_data_ = nullptr;
}

dir& dir::operator=(const dir& d) {
  if (&d == this) return *this;
  dir_ = d.dir_;
  path_ = d.path_;

  if (first_data_) {
    delete first_data_;
    first_data_ = nullptr;
  }

  if (d.first_data_) {
    first_data_ = new struct _wfinddata_t;
    memcpy(first_data_, d.first_data_, sizeof(struct _wfinddata_t));
  } else {
    first_data_ = nullptr;
  }

  return *this;
}

dir& dir::operator=(dir&& d) {
  dir_ = std::exchange(d.dir_, -1);
  path_ = std::move(d.path_);
  if (first_data_) {
    delete first_data_;
    first_data_ = nullptr;
  }
  first_data_ = d.first_data_;
  d.first_data_ = nullptr;
  return *this;
}

dir::dir(const std::string& p): dir_(-1), path_(p), first_data_(nullptr) {
  std::string path = toyo::path::normalize(p);
  std::wstring wpath = toyo::charset::a2w(path);
  first_data_ = new struct _wfinddata_t;
  dir_ = _wfindfirst(toyo::charset::a2w(path::win32::join(path, "*")).c_str(), first_data_);
  if (dir_ == -1) {
    delete first_data_;
    throw cerror(errno, std::string("opendir \"") + p + "\"");
  }
}
dir::~dir() {
  if (first_data_) {
    delete first_data_;
    first_data_ = nullptr;
  }
  if (dir_ != -1) {
    _findclose(dir_);
    dir_ = -1;
  }
}

std::string dir::path() const { return path_; }

void dir::close() {
  if (first_data_) {
    delete first_data_;
    first_data_ = nullptr;
  }
  if (dir_ != -1) {
    if (0 != _findclose(dir_)) {
      throw cerror(errno, std::string("closedir \"") + path_ + "\""); 
    };
    dir_ = -1;
  }
}

fs::dirent dir::read() {
  if (first_data_) {
    fs::dirent tmp(first_data_);
    delete first_data_;
    first_data_ = nullptr;
    return tmp;
  }
  struct _wfinddata_t* file = new struct _wfinddata_t;
  int ret = _wfindnext(dir_, file);
  if (ret == 0) {
    fs::dirent tmp(file);
    delete file;
    return tmp;
  } else {
    delete file;
    return nullptr;
  }
}
#else
dirent::dirent(): dirent_(nullptr) {}
dirent::dirent(struct ::dirent* d): dirent_(d) {}

bool dirent::is_empty() const { return dirent_ == nullptr; }

const struct ::dirent* dirent::data() { return dirent_; }

std::string dirent::name() const {
  if (dirent_) {
    return dirent_->d_name;
  } else {
    return "";
  }
}

bool dirent::is_file() const {
  if (dirent_) {
    return dirent_->d_type == DT_REG;
  } else {
    return false;
  }
}
bool dirent::is_directory() const {
  if (dirent_) {
    return dirent_->d_type == DT_DIR;
  } else {
    return false;
  }
}
bool dirent::is_fifo() const {
  if (dirent_) {
    return dirent_->d_type == DT_FIFO;
  } else {
    return false;
  }
}
bool dirent::is_character_device() const {
  if (dirent_) {
    return dirent_->d_type == DT_FIFO;
  } else {
    return false;
  }
}
bool dirent::is_symbolic_link() const {
  if (dirent_) {
    return dirent_->d_type == DT_LNK;
  } else {
    return false;
  }
}
bool dirent::is_block_device() const {
  if (dirent_) {
    return dirent_->d_type == DT_BLK;
  } else {
    return false;
  }
}
bool dirent::is_socket() const {
  if (dirent_) {
    return dirent_->d_type == DT_SOCK;
  } else {
    return false;
  }
}

dir::dir(const std::string& p): dir_(nullptr), path_(p) {
  std::string path = toyo::path::normalize(p);
  if ((dir_ = ::opendir(path.c_str())) == nullptr) {
    throw cerror(errno, std::string("opendir \"") + p + "\"");
  }
}
dir::~dir() {
  if (dir_) {
    closedir(dir_);
    dir_ = nullptr;
  }
}

std::string dir::path() const { return path_; }

void dir::close() {
  if (dir_) {
    if (0 != closedir(dir_)) {
      throw cerror(errno, std::string("closedir \"") + path_ + "\""); 
    }
    dir_ = nullptr;
  }
}

fs::dirent dir::read() const {
  struct ::dirent *direntp = ::readdir(dir_);
  return direntp;
}

#endif

fs::dir opendir(const std::string& p) {
  return fs::dir(p);
}

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
  std::string newPath = toyo::path::normalize(p);
  fs::dir dir = fs::opendir(newPath);

  std::vector<std::string> res;
  std::string item;

  fs::dirent dirent;

  while (!((dirent = dir.read()).is_empty())) {
    item = dirent.name();
    if (item != "." && item != "..") {
      res.push_back(item);
    }
  }

  dir.close();
  return res;
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
static std::string get_last_error_message() {
  int size = 0;
  get_last_error(nullptr, &size);
  char* buf = new char[size];
  get_last_error(buf, &size);
  std::string res(buf);
  delete buf;
  return res;
}
#endif

void symlink(const std::string& o, const std::string& n, symlink_type type) {
  std::string oldpath = path::normalize(o);
  std::string newpath = path::normalize(n);
#ifdef _WIN32
  if (type == symlink_type_directory) {
    if (!CreateSymbolicLinkW(toyo::charset::a2w(newpath).c_str(), toyo::charset::a2w(oldpath).c_str(), SYMBOLIC_LINK_FLAG_DIRECTORY)) {
      throw std::runtime_error(get_last_error_message() + " symlink \"" + o + "\" -> \"" + n + "\"");
    }
  } else if (type == symlink_type_file) {
    if (!CreateSymbolicLinkW(toyo::charset::a2w(newpath).c_str(), toyo::charset::a2w(oldpath).c_str(), 0x0)) {
      throw std::runtime_error(get_last_error_message() + " symlink \"" + o + "\" -> \"" + n + "\"");
    }
  } else if (type == symlink_type_junction) {
    oldpath = path::resolve(oldpath);
    try {
      fs::symlink(oldpath, n);
    } catch (const std::exception&) {
      throw std::runtime_error(get_last_error_message() + " symlink \"" + o + "\" -> \"" + n + "\"");
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
