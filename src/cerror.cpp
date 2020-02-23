#include <cerrno>
#include <cstring>

#include "cerror.hpp"

cerror::cerror(int code, const std::string& message): code_(code) {
  if (message != "") {
    this->message_ = std::string(strerror(errno)) + ", " + message;
  } else {
    this->message_ = std::string(strerror(errno)) + ".";
  }
}
const char* cerror::what() const noexcept {
  return this->message_.c_str();
}
