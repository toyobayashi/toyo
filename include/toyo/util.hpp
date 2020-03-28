#ifndef __UTIL_HPP__
#define __UTIL_HPP__

#include <string>
#include <vector>

namespace toyo {

namespace util {
  std::string to_b64(const std::string& buffer);
  std::string to_b64(const std::vector<unsigned char>& buffer);

  std::vector<unsigned char> b64_to_buffer(const std::string& b64);
  std::string b64_to_string(const std::string& b64);
}

}

#endif
