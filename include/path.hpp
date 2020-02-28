#ifndef __PATH_HPP__
#define __PATH_HPP__

#include "string.hpp"

namespace toyo {

namespace path {

class path {
private:
  toyo::string dir_;
  toyo::string root_;
  toyo::string base_;
  toyo::string name_;
  toyo::string ext_;

  toyo::string _format(const toyo::string& sep) const;

  path(const toyo::string&, bool);
public:
  static path parse_win32(const toyo::string&);
  static path parse_posix(const toyo::string&);
  static path parse(const toyo::string&);

  path();
  path(const toyo::string&);
  path(const char*);

  toyo::string dir() const;
  toyo::string root() const;
  toyo::string base() const;
  toyo::string name() const;
  toyo::string ext() const;

  path& dir(const toyo::string&);
  path& root(const toyo::string&);
  path& base(const toyo::string&);
  path& name(const toyo::string&);
  path& ext(const toyo::string&);

  path operator+(const path& p) const;
  path& operator+=(const path& p);
  bool operator==(const path& other);
  toyo::string format_win32() const;
  toyo::string format_posix() const;
  toyo::string format() const;
};

namespace win32 {
  const toyo::string sep = "\\";
  const toyo::string delimiter = ";";

  toyo::string normalize(const toyo::string&);

  toyo::string resolve(const toyo::string& arg = "", const toyo::string& arg1 = "");

  template <typename... Args>
  inline toyo::string resolve(const toyo::string& arg1, const toyo::string& arg2, Args... args) {
    toyo::string tmp = win32::resolve(arg1, arg2);
    return win32::resolve(tmp, args...);
  }

  toyo::string join(const toyo::string&, const toyo::string& arg2 = "");

  template <typename... Args>
  inline toyo::string join(const toyo::string& arg1, const toyo::string& arg2, Args... args) {
    toyo::string tmp = win32::join(arg1, arg2);
    return win32::join(tmp, args...);
  }

  bool is_absolute(const toyo::string&);
  toyo::string dirname(const toyo::string&);
  toyo::string to_namespaced_path(const toyo::string&);
  toyo::string basename(const toyo::string&);
  toyo::string basename(const toyo::string&, const toyo::string&);
  toyo::string extname(const toyo::string&);
  toyo::string relative(const toyo::string&, const toyo::string&);
} // win32

namespace posix {
  const toyo::string sep = "/";
  const toyo::string delimiter = ":";

  toyo::string normalize(const toyo::string&);

  toyo::string resolve(const toyo::string& arg = "", const toyo::string& arg1 = "");

  template <typename... Args>
  inline toyo::string resolve(const toyo::string& arg1, const toyo::string& arg2, Args... args) {
    toyo::string tmp = posix::resolve(arg1, arg2);
    return posix::resolve(tmp, args...);
  }

  toyo::string join(const toyo::string&, const toyo::string& arg2 = "");

  template <typename... Args>
  inline toyo::string join(const toyo::string& arg1, const toyo::string& arg2, Args... args) {
    toyo::string tmp = posix::join(arg1, arg2);
    return posix::join(tmp, args...);
  }

  bool is_absolute(const toyo::string&);
  toyo::string dirname(const toyo::string&);
  toyo::string to_namespaced_path(const toyo::string&);
  toyo::string basename(const toyo::string&);
  toyo::string basename(const toyo::string&, const toyo::string&);
  toyo::string extname(const toyo::string&);
  toyo::string relative(const toyo::string&, const toyo::string&);
} // posix

#ifdef _WIN32
  const toyo::string sep = win32::sep;
  const toyo::string delimiter = win32::delimiter;
#else
  const toyo::string sep = posix::sep;
  const toyo::string delimiter = posix::delimiter;
#endif

toyo::string normalize(const toyo::string&);

template <typename... Args>
inline toyo::string resolve(Args... args) {
#ifdef _WIN32
  return win32::resolve(args...);
#else
  return posix::resolve(args...);
#endif
}

template <typename... Args>
inline toyo::string join(Args... args) {
#ifdef _WIN32
  return win32::join(args...);
#else
  return posix::join(args...);
#endif
}

bool is_absolute(const toyo::string&);
toyo::string dirname(const toyo::string&);
toyo::string to_namespaced_path(const toyo::string&);

toyo::string basename(const toyo::string&);
toyo::string basename(const toyo::string&, const toyo::string&);

toyo::string extname(const toyo::string&);
toyo::string relative(const toyo::string&, const toyo::string&);

toyo::string __filename();
toyo::string __dirname();

} // path

} // toyo

#endif
