#ifndef __PROCESS_HPP__
#define __PROCESS_HPP__

#include <string>

namespace toyo {

namespace process {

std::string cwd();
int pid();
std::string platform();

} // process

} // toyo

#endif
