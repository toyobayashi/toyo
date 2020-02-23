#ifndef __CHARSET_HPP__
#define __CHARSET_HPP__

#include <string>

namespace toyo {

namespace charset {

std::wstring a2w(const std::string& str);

std::string w2a(const std::wstring& wstr);

std::string w2acp(const std::wstring& wstr);

std::string a2acp(const std::string& str);

} // charset

} // toyo

#endif
