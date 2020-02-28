#ifndef __FS_HPP__
#define __FS_HPP__

#include "string.hpp"
#include <vector>

#include <ctime>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32

#else
#include <dirent.h>
#include <unistd.h>
#endif

namespace toyo {

namespace fs {

enum access_type {
  f_ok = 0,
#ifdef _WIN32
  x_ok = 0,
#else
  x_ok = 1,
#endif
  w_ok = 2,
  r_ok = 4,
};

enum symlink_type {
  symlink_type_file,
  symlink_type_directory,
  symlink_type_junction
};

class stats {
private:
  bool is_link_;
public:
  stats();
  stats(const toyo::string&, bool follow_link = false);
  stats(const char*, bool follow_link = false);

  unsigned int dev;
  unsigned short ino;
  unsigned short mode;
  short nlink;
  short gid;
  short uid;
  unsigned int rdev;
  long size;
  time_t atime;
  time_t mtime;
  time_t ctime;

  bool is_file() const;
  bool is_directory() const;
  bool is_fifo() const;
  bool is_character_device() const;
  bool is_symbolic_link() const;
  bool is_block_device() const;
  bool is_socket() const;
};

#ifdef _WIN32
class dirent;
class dir;

class dirent {
private:
  struct _wfinddata_t* dirent_;

public:
  dirent();
  ~dirent();
  dirent(const dirent& d);
  dirent(dirent&& d);
  dirent(struct _wfinddata_t* d);

  bool is_empty() const;

  const struct _wfinddata_t* data();

  toyo::string name() const;

  dirent& operator=(const dirent& d);
  dirent& operator=(dirent&& d);

  bool is_file() const;
  bool is_directory() const;
  bool is_fifo() const;
  bool is_character_device() const;
  bool is_symbolic_link() const;
  bool is_block_device() const;
  bool is_socket() const;
};

class dir {
private:
  intptr_t dir_;
  toyo::string path_;
  struct _wfinddata_t* first_data_;
public:
  dir(const dir&);
  dir(dir&&);
  dir(const toyo::string& p);
  ~dir();
  void close();
  toyo::string path() const;
  fs::dirent read();

  dir& operator=(const dir& d);
  dir& operator=(dir&& d);
};
#else
class dirent;
class dir;

class dirent {
private:
  struct ::dirent* dirent_;

public:
  dirent();
  dirent(struct ::dirent* d);

  bool is_empty() const;

  const struct ::dirent* data();

  toyo::string name() const;

  bool is_file() const;
  bool is_directory() const;
  bool is_fifo() const;
  bool is_character_device() const;
  bool is_symbolic_link() const;
  bool is_block_device() const;
  bool is_socket() const;
};

class dir {
private:
  DIR* dir_;
  toyo::string path_;
public:
  dir(const toyo::string& p);
  ~dir();
  void close();
  toyo::string path() const;
  fs::dirent read() const;
};
#endif

fs::dir opendir(const toyo::string&);
std::vector<toyo::string> readdir(const toyo::string&);
bool access(const toyo::string&, int mode = 0);
bool exists(const toyo::string&);
stats stat(const toyo::string&);
stats lstat(const toyo::string&);
void mkdir(const toyo::string&, int mode = 0777);
void mkdirs(const toyo::string&, int mode = 0777);
void unlink(const toyo::string&);
void rmdir(const toyo::string&);
void rename(const toyo::string&, const toyo::string&);
void remove(const toyo::string&);
void symlink(const toyo::string&, const toyo::string&);
void symlink(const toyo::string&, const toyo::string&, symlink_type);
void copy_file(const toyo::string&, const toyo::string&, bool fail_if_exists = false);
std::vector<unsigned char> read_file(const toyo::string&);
toyo::string read_file_to_string(const toyo::string&);
void write_file(const toyo::string&, const std::vector<unsigned char>&);
void write_file(const toyo::string&, const toyo::string&);
void append_file(const toyo::string&, const std::vector<unsigned char>&);
void append_file(const toyo::string&, const toyo::string&);

}

}

#endif
