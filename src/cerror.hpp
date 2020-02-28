#ifndef __CERROR_HPP__
#define __CERROR_HPP__

#include <exception>
#include "string.hpp"

class cerror : public std::exception {
public:
  cerror(int code, const toyo::string& message = "");
  virtual const char* what() const noexcept override;
  int code() const;
private:
  int code_;
  toyo::string message_;
};

#endif
