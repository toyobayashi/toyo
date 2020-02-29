#ifndef __PROCESS_HPP__
#define __PROCESS_HPP__

#include <string>

namespace toyo {

namespace process {

std::string cwd();
int pid();
std::string platform();

void* dlopen(const std::string& file, int mode);
int dlclose(void* handle);
void* dlsym(void* handle, const std::string& name);
std::string dlerror();

} // process

} // toyo

#endif
