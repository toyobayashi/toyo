#ifndef __STRING_HPP__
#define __STRING_HPP__

#include <string>

namespace toyo {

namespace string {

std::wstring wsubstring(const std::wstring& self, int indexStart, int indexEnd);

std::string substring(const std::string& self, int indexStart, int indexEnd);

std::wstring wslice(const std::wstring& self, int start, int end);

std::wstring wslice(const std::wstring& self, int start);

std::string slice(const std::string& self, int start, int end);

std::string slice(const std::string& self, int start);

std::wstring wto_lower_case(const std::wstring& self);

std::string to_lower_case(const std::string& self);

int wlast_index_of(const std::wstring& self, const std::wstring& searchValue, int fromIndex);

int wlast_index_of(const std::wstring& self, const std::wstring& searchValue);

}

}

#endif
