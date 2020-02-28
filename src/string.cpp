#include <cstddef>
#include <cstring>
#include <cstdlib>
#include <clocale>
#include <stdexcept>
#include <regex>

#include "string.hpp"
#include "win.h"

namespace toyo {

void free(void* block) {
  ::free(block);
}

void* malloc(size_t size) {
  return ::malloc(size);
}

wchar_t* string::utf82w(const char* utf8) {
  std::string target_locale = "en_US.utf8";
  const char* c_locale = std::setlocale(LC_CTYPE, nullptr);
  std::string locale(c_locale ? c_locale : "");

  if (locale != target_locale) {
    std::setlocale(LC_CTYPE, target_locale.c_str());
  }

  size_t len = std::mbstowcs(nullptr, utf8, 0);
  wchar_t* buf = (wchar_t*)malloc((len + 1) * sizeof(wchar_t));
  if (!buf) {
    if (locale != "") {
      std::setlocale(LC_CTYPE, locale.c_str());
    }
    throw std::runtime_error(strerror(errno));
  }
  memset(buf, 0, (len + 1) * sizeof(wchar_t));
  std::mbstowcs(buf, utf8, len + 1);

  if (locale != "") {
    std::setlocale(LC_CTYPE, locale.c_str());
  }
  return buf;
}

char* string::w2utf8(const wchar_t* unicode) {
  std::string target_locale = "en_US.utf8";
  const char* c_locale = std::setlocale(LC_CTYPE, nullptr);
  std::string locale(c_locale ? c_locale : "");

  if (locale != target_locale) {
    std::setlocale(LC_CTYPE, target_locale.c_str());
  }

  size_t len = std::wcstombs(nullptr, unicode, 0);
  char* buf = (char*)malloc((len + 1) * sizeof(char));
  if (!buf) {
    if (locale != "") {
      std::setlocale(LC_CTYPE, locale.c_str());
    }
    throw std::runtime_error(strerror(errno));
  }
  memset(buf, 0, (len + 1) * sizeof(char));
  std::wcstombs(buf, unicode, len + 1);

  if (locale != "") {
    std::setlocale(LC_CTYPE, locale.c_str());
  }
  return buf;
}

string::string(): _str(""), _wstr(L"") {}

string::string(char c) {
  char str[2] = { c, '\0' };
  _str = str;
  wchar_t wstr[2] = { (wchar_t)(c), L'\0' };
  _wstr = wstr;
}

string::string(const char* str) {
  wchar_t* unicode = utf82w(str);
  _wstr = unicode;
  free(unicode);
  this->_sync();
}

string::string(wchar_t c) {
  wchar_t wstr[2] = { c, L'\0' };
  _wstr = wstr;
  this->_sync();
}

string::string(const wchar_t* wstr): _wstr(wstr) {
  this->_sync();
}

void string::_sync() {
  char* utf8 = w2utf8(this->_wstr.c_str());
  this->_str = utf8;
  free(utf8);
}

const char* string::c_str() const {
  return _str.c_str();
}
const wchar_t* string::c_strw() const {
  return _wstr.c_str();
}

bool string::operator==(const string& other) const {
  return _wstr == other.c_strw();
}

bool string::operator!=(const string& other) const {
  return _wstr != other.c_strw();
}

string string::operator+(const string& other) const {
  string res;
  res._wstr = _wstr + other._wstr;
  res._sync();
  return res;
}
string string::operator+(char other) const {
  string res;
  wchar_t wstr[2] = { (wchar_t)other, L'\0' };
  res._wstr = this->_wstr + wstr;
  res._sync();
  return res;
}
string string::operator+(const char* other) const {
  string res;
  wchar_t* tmp = string::utf82w(other);
  res._wstr = this->_wstr + tmp;
  free(tmp);
  res._sync();
  return res;
}
string string::operator+(wchar_t other) const {
  string res;
  wchar_t wstr[2] = { other, L'\0' };
  res._wstr = this->_wstr + wstr;
  res._sync();
  return res;
}
string string::operator+(const wchar_t* other) const {
  string res;
  res._wstr = this->_wstr + other;
  res._sync();
  return res;
}

string& string::operator+=(const string& other) {
  _wstr += other._wstr;
  this->_sync();
  return *this;
}

bool string::operator<(const string& other) const {
  return _wstr < other._wstr;
}

bool string::operator<=(const string& other) const {
  return _wstr <= other._wstr;
}

bool string::operator>(const string& other) const {
  return _wstr > other._wstr;
}

bool string::operator>=(const string& other) const {
  return _wstr >= other._wstr;
}

wchar_t string::operator[](int index) const {
  return _wstr[index];
}

int string::length() const {
  return (int)_wstr.length();
}

int string::byte_length() const {
  return (int)_str.length();
}

char* string::c_strcp(int code_page) const {
#ifdef _WIN32
  int len = WideCharToMultiByte(code_page, 0, _wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
  if (len == -1) {
    int size = 0;
    get_last_error(nullptr, &size);
    char* buf = new char[size];
    get_last_error(buf, &size);
    toyo::string res(buf);
    delete buf;
    throw std::runtime_error(res.c_str());
  }
  char* buf = (char*)malloc(len);
  memset(buf, 0, len);
  WideCharToMultiByte(code_page, 0, _wstr.c_str(), -1, buf, len, nullptr, nullptr);
  return buf;
#else
  size_t size = (byte_length() + 1) * sizeof(char);
  char* buf = (char*)malloc(size);
  if (!buf) {
    throw std::runtime_error(strerror(errno));
  }
  memset(buf, 0, size);
  strcpy(buf, _str.c_str());
  return buf;
#endif
}

char* string::c_stro() const {
#ifdef _WIN32
  return this->c_strcp(GetConsoleOutputCP());
#else
  return this->c_strcp(0);
#endif
}

string string::char_at(int index) const {
  return _wstr[index];
}

unsigned short string::char_code_at(int index) const {
  return (unsigned short)(_wstr[index]);
}

string string::substring(int indexStart) const {
  int l = this->length();
  if (l == 0)
    return "";
  if (indexStart >= l) {
    indexStart = l;
  } else if (indexStart < 0) {
    indexStart = 0;
  }

  string res = "";

  for (int i = indexStart; i < l; i++) {
    res += (*this)[i];
  }

  return res;
}

string string::substring(int indexStart, int indexEnd) const {
  int l = this->length();
  if (l == 0)
    return "";
  if (indexStart >= l) {
    indexStart = l;
  } else if (indexStart < 0) {
    indexStart = 0;
  }

  if (indexEnd >= l) {
    indexEnd = l;
  } else if (indexEnd < 0) {
    indexEnd = 0;
  }

  if (indexStart == indexEnd) return "";

  if (indexEnd < indexStart) {
    int tmp = indexStart;
    indexStart = indexEnd;
    indexEnd = tmp;
  }

  string res = "";

  for (int i = indexStart; i < indexEnd; i++) {
    res += (*this)[i];
  }

  return res;
}

bool string::ends_with(const string& searchString) const {
  int position = length();
  return substring(position - searchString.length(), position) == searchString;
}

bool string::ends_with(const string& searchString, int position) const {
  return substring(position - searchString.length(), position) == searchString;
}

int string::index_of(const string& searchValue) const {
  return this->index_of(searchValue, 0);
}

int string::index_of(const string& searchValue, int fromIndex) const {
  int thisLength = this->length();
  if (searchValue == "") {
    return fromIndex <= 0 ? 0 : (fromIndex <= thisLength ? fromIndex : thisLength);
  }

  if (fromIndex >= thisLength) {
    return -1;
  }

  if (fromIndex < 0) {
    fromIndex = 0;
  }

  int len = searchValue.length();
  for (int i = fromIndex; i < this->length(); i++) {
    if (searchValue == this->substring(i, i + len)) {
      return i;
    }
  }
  return -1;
}

int string::last_index_of(const string& searchValue) const {
  return this->last_index_of(searchValue, this->length());
}
int string::last_index_of(const string& searchValue, int fromIndex) const {
  int thisLength = this->length();
  if (fromIndex < 0) {
    fromIndex = 0;
  } else if (fromIndex > thisLength) {
    fromIndex = thisLength;
  }

  int len = searchValue.length();
  for (int i = fromIndex - 1; i >= 0; i--) {
    if (searchValue == this->substring(i, i + len)) {
      return i;
    }
  }
  return -1;
}

bool string::includes(const string& searchString) const {
  return this->includes(searchString, 0);
}

bool string::includes(const string& searchString, int position) const {
  if (position + searchString.length() > this->length()) {
    return false;
  } else {
    return -1 != this->index_of(searchString, position);
  }
}

string string::replace(const string& searchString, const string& replace) const {
  std::wstring wstr = _wstr;
  int i = this->index_of(searchString);
  if (i == -1) {
    return *this;
  }

  return wstr.replace(i, searchString.length(), replace._wstr).c_str();
}

string string::slice(int start) const {
  return slice(start, length());
}
string string::slice(int start, int end) const {
  int _length = this->length();
  end--;
  start = start < 0 ? (_length + (start % _length)) : start % _length;
  end = end < 0 ? (_length + (end % _length)) : end % _length;
  if (end < start) {
    int tmp = end;
    end = start;
    start = tmp;
  }

  int len = end - start + 1;

  if (len <= 0) return L"";

  return substring(start, end + 1);
}

string string::repeat(int n) const {
  if (n <= 0) {
    return "";
  }

  string res = "";
  for (int i = 0; i < n; i++) {
    res += (*this);
  }

  return res;
}

string string::to_lower_case() const {
  int bl = length();
  wchar_t* res = new wchar_t[bl + 1];
  memset(res, 0, (bl + 1) * sizeof(wchar_t));
  for (int i = 0; i < bl; i++) {
    if (_wstr[i] >= 65 && _wstr[i] <= 90) {
      res[i] = _wstr[i] + 32;
    } else {
      res[i] = _wstr[i];
    }
  }
  std::wstring ret(res);
  delete[] res;
  return ret.c_str();
}

string string::to_upper_case() const {
  int bl = length();
  wchar_t* res = new wchar_t[bl + 1];
  memset(res, 0, (bl + 1) * sizeof(wchar_t));
  for (int i = 0; i < bl; i++) {
    if (_wstr[i] >= 97 && _wstr[i] <= 122) {
      res[i] = _wstr[i] - 32;
    } else {
      res[i] = _wstr[i];
    }
  }
  std::wstring ret(res);
  delete[] res;
  res = nullptr;
  return ret.c_str();
}

string string::trim() const {
  std::string str = _str;
  return std::regex_replace(str, std::regex("^[\\s\\xA0]+|[\\s\\xA0]+$"), std::string("")).c_str();
}
string string::trim_right() const {
  std::string str = _str;
  return std::regex_replace(str, std::regex("[\\s\\xA0]+$"), std::string("")).c_str();
}
string string::trim_left() const {
  std::string str = _str;
  return std::regex_replace(str, std::regex("^[\\s\\xA0]+"), std::string("")).c_str();
}

string string::concat(const string& str) const {
  return (*this + str);
}

string string::from_char_code(unsigned short num) {
  return static_cast<wchar_t>(num);
}

string operator+(const char* left, const string& right) {
  return (std::string(left) + std::string(right.c_str())).c_str();
}

string operator+(const wchar_t* left, const string& right) {
  return (std::wstring(left) + std::wstring(right.c_strw())).c_str();
}

}
