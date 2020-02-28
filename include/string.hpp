#ifndef __STRING_HPP__
#define __STRING_HPP__

// #ifdef _WIN32
// #include <Windows.h>
// #endif

#include <cstddef>
#include <string>
// #include <regex>
// template <typename T> class Array;

namespace toyo {

void free(void*);
void* malloc(size_t);

class string {
 public:
  static wchar_t* utf82w(const char*);
  static char* w2utf8(const wchar_t*);
  string();
  string(char);
  string(const char*);
  string(wchar_t);
  string(const wchar_t*);

  const char* c_str() const;
  const wchar_t* c_strw() const;

  bool operator==(const string&) const;

  bool operator!=(const string&) const;

  string operator+(const string&) const;

  string& operator+=(const string&);

  bool operator<(const string&) const;

  bool operator<=(const string&) const;

  bool operator>(const string&) const;

  bool operator>=(const string&) const;

  wchar_t operator[](int index) const;

  int length() const;
  int byte_length() const;

  string char_at(int index = 0) const;
  unsigned short char_code_at(int index = 0) const;
  string substring(int) const;
  string substring(int, int) const;
  bool ends_with(const string&) const;
  bool ends_with(const string&, int) const;
  int index_of(const string&) const;
  int index_of(const string&, int) const;
  int last_index_of(const string&) const;
  int last_index_of(const string&, int) const;
  bool includes(const string&) const;
  bool includes(const string&, int) const;
  // string replace(const std::regex&, const string&) const;
  string replace(const string&, const string&) const;
  string slice(int) const;
  string slice(int, int) const;
  string repeat(int) const;
  string to_lower_case() const;
  string to_upper_case() const;
  string trim() const;
  string trim_right() const;
  string trim_left() const;
  // Array<String> split() const;
  // Array<String> split(const String& separator, int limit = -1) const;

  string concat(const string& str) const;

  template <typename... Arg>
  string concat(const string& str, Arg... args) const {
    return concat(str).concat(args...);
  }

  static string from_char_code(unsigned short num);

  template <typename... Arg>
  static string from_char_code(unsigned short num, Arg... args) {
    string res = from_char_code(num);
    res += from_char_code(args...);
    return res;
  }

 private:
  std::string _str;
  std::wstring _wstr;
  void _sync();
};

}

#endif  // __STRING_HPP__
