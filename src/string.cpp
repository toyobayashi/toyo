#include <cstddef>
#include <cstring>

#include "charset.hpp"
#include "string.hpp"

namespace toyo {

namespace string {

std::wstring wsubstring(const std::wstring& self, int indexStart, int indexEnd) {
  size_t l = self.length();
  if (l == 0)
    return std::wstring({ self[0], L'\0' });
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

  if (indexStart == indexEnd) return L"";

  if (indexEnd < indexStart) {
    size_t tmp = indexStart;
    indexStart = indexEnd;
    indexEnd = tmp;
  }

  std::wstring res = L"";

  for (size_t i = indexStart; i < indexEnd; i++) {
    res += self[i];
  }

  return res;
}

std::string substring(const std::string& self, int indexStart, int indexEnd) {
  return toyo::charset::w2a(wsubstring(toyo::charset::a2w(self), indexStart, indexStart));
}

std::wstring wslice(const std::wstring& self, int start, int end) {
  size_t _length = self.length();
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

  return wsubstring(self, start, end + 1);
}

std::wstring wslice(const std::wstring& self, int start) {
  return wslice(self, start, self.length());
}

std::string slice(const std::string& self, int start, int end) {
  return toyo::charset::w2a(wslice(toyo::charset::a2w(self), start, end));
}

std::string slice(const std::string& self, int start) {
  return toyo::charset::w2a(wslice(toyo::charset::a2w(self), start));
}

std::wstring wto_lower_case(const std::wstring& self) {
  size_t bl = self.length();
  wchar_t* res = new wchar_t[bl + 1];
  memset(res, 0, (bl + 1) * sizeof(wchar_t));
  for (size_t i = 0; i < bl; i++) {
    if (self[i] >= 65 && self[i] <= 90) {
      res[i] = self[i] + 32;
    } else {
      res[i] = self[i];
    }
  }
  std::wstring ret(res);
  delete[] res;
  return ret;
}

std::string to_lower_case(const std::string& self) {
  return toyo::charset::w2a(wto_lower_case(toyo::charset::a2w(self)));
}

int wlast_index_of(const std::wstring& self, const std::wstring& searchValue, int fromIndex) {
  size_t thisLength = self.length();
  if (fromIndex < 0) {
    fromIndex = 0;
  } else if (fromIndex > thisLength) {
    fromIndex = thisLength;
  }

  size_t len = searchValue.length();
  for (int i = fromIndex - 1; i >= 0; i--) {
    if (searchValue == wsubstring(self, i, i + len)) {
      return i;
    }
  }
  return -1;
}

int wlast_index_of(const std::wstring& self, const std::wstring& searchValue) {
  return wlast_index_of(self, searchValue, self.length());
}

}

}
