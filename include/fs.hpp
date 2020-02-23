#ifndef __FS_HPP__
#define __FS_HPP__

#include <string>
#include <vector>

#include <ctime>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32

#else
#include <unistd.h>
#endif

namespace toyo {

namespace fs {

class stats {
private:
  std::string path_;
public:
  stats(const std::string&);
  stats(const char*);

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

std::vector<std::string> readdir(const std::string&);
bool exists(const std::string&);
stats stat(const std::string&);
void mkdir(const std::string&, int mode = 0777);
void mkdirs(const std::string&, int mode = 0777);
void unlink(const std::string&);
void rmdir(const std::string&);
void rename(const std::string&, const std::string&);
void remove(const std::string&);
}

}

#endif
