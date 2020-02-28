#ifndef __PROCESS_HPP__
#define __PROCESS_HPP__

#include "string.hpp"

namespace toyo {

namespace process {

toyo::string cwd();
int pid();
toyo::string platform();

} // process

} // toyo

#endif
