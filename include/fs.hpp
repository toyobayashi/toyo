#ifndef __FS_HPP__
#define __FS_HPP__

#include <string>
#include <vector>

namespace toyo {

namespace fs {

std::vector<std::string> readdir(const std::string&);
bool exists(const std::string&);

}

}

#endif
