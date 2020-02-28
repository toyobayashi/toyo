#include "win.h"
#include <vector>
#include <string>
#include <cstddef>

#ifndef _WIN32
#include <unistd.h>
#endif

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#include "path.hpp"
#include "process.hpp"

namespace toyo {

namespace path {

// Alphabet chars.
const unsigned short CHAR_UPPERCASE_A = 65; /* A */
const unsigned short CHAR_LOWERCASE_A = 97; /* a */
const unsigned short CHAR_UPPERCASE_Z = 90; /* Z */
const unsigned short CHAR_LOWERCASE_Z = 122; /* z */

// Non-alphabetic chars.
const unsigned short CHAR_DOT = 46; /* . */
const unsigned short CHAR_FORWARD_SLASH = 47; /* / */
const unsigned short CHAR_BACKWARD_SLASH = 92; /* \ */
const unsigned short CHAR_VERTICAL_LINE = 124; /* | */
const unsigned short CHAR_COLON = 58; /* : */
const unsigned short CHAR_QUESTION_MARK = 63; /* ? */
const unsigned short CHAR_UNDERSCORE = 95; /* _ */
const unsigned short CHAR_LINE_FEED = 10; /* \n */
const unsigned short CHAR_CARRIAGE_RETURN = 13; /* \r */
const unsigned short CHAR_TAB = 9; /* \t */
const unsigned short CHAR_FORM_FEED = 12; /* \f */
const unsigned short CHAR_EXCLAMATION_MARK = 33; /* ! */
const unsigned short CHAR_HASH = 35; /* # */
const unsigned short CHAR_SPACE = 32; /*   */
const unsigned short CHAR_NO_BREAK_SPACE = 160; /* \u00A0 */
const unsigned short CHAR_ZERO_WIDTH_NOBREAK_SPACE = 65279; /* \uFEFF */
const unsigned short CHAR_LEFT_SQUARE_BRACKET = 91; /* [ */
const unsigned short CHAR_RIGHT_SQUARE_BRACKET = 93; /* ] */
const unsigned short CHAR_LEFT_ANGLE_BRACKET = 60; /* < */
const unsigned short CHAR_RIGHT_ANGLE_BRACKET = 62; /* > */
const unsigned short CHAR_LEFT_CURLY_BRACKET = 123; /* { */
const unsigned short CHAR_RIGHT_CURLY_BRACKET = 125; /* } */
const unsigned short CHAR_HYPHEN_MINUS = 45; /* - */
const unsigned short CHAR_PLUS = 43; /* + */
const unsigned short CHAR_DOUBLE_QUOTE = 34; /* " */
const unsigned short CHAR_SINGLE_QUOTE = 39; /* ' */
const unsigned short CHAR_PERCENT = 37; /* % */
const unsigned short CHAR_SEMICOLON = 59; /* ; */
const unsigned short CHAR_CIRCUMFLEX_ACCENT = 94; /* ^ */
const unsigned short CHAR_GRAVE_ACCENT = 96; /* ` */
const unsigned short CHAR_AT = 64; /* @ */
const unsigned short CHAR_AMPERSAND = 38; /* & */
const unsigned short CHAR_EQUAL = 61; /* = */

// Digits
const unsigned short CHAR_0 = 48; /* 0 */
const unsigned short CHAR_9 = 57; /* 9 */

static bool _isPathSeparator(unsigned short code) {
  return code == CHAR_FORWARD_SLASH || code == CHAR_BACKWARD_SLASH;
}
static bool _isPosixPathSeparator(unsigned short code) {
  return code == CHAR_FORWARD_SLASH;
}
static bool _isWindowsDeviceRoot(unsigned short code) {
  return (code >= CHAR_UPPERCASE_A && code <= CHAR_UPPERCASE_Z) || (code >= CHAR_LOWERCASE_A && code <= CHAR_LOWERCASE_Z);
}

static toyo::string _normalizeString(const toyo::string& path, bool allowAboveRoot, const toyo::string& separator, bool (*isPathSeparator)(unsigned short)) {
  toyo::string res = L"";
  int lastSegmentLength = 0;
  int lastSlash = -1;
  int dots = 0;
  unsigned short code = 0;
  for (int i = 0; i <= (int)path.length(); ++i) {
    if (i < (int)path.length())
      code = path[i];
    else if (isPathSeparator(code))
      break;
    else
      code = CHAR_FORWARD_SLASH;

    if (isPathSeparator(code)) {
      if (lastSlash == i - 1 || dots == 1) {
        // NOOP
      } else if (lastSlash != i - 1 && dots == 2) {
        if (res.length() < 2 || lastSegmentLength != 2 ||
          res[res.length() - 1] != CHAR_DOT ||
          res[res.length() - 2] != CHAR_DOT) {
          if (res.length() > 2) {
            const int lastSlashIndex = res.last_index_of(separator);
            if (lastSlashIndex == -1) {
              res = L"";
              lastSegmentLength = 0;
            } else {
              res = res.slice(0, lastSlashIndex);
              lastSegmentLength = (int)res.length() - 1 - res.last_index_of(separator);;
            }
            lastSlash = i;
            dots = 0;
            continue;
          } else if (res.length() == 2 || res.length() == 1) {
            res = L"";
            lastSegmentLength = 0;
            lastSlash = i;
            dots = 0;
            continue;
          }
        }
        if (allowAboveRoot) {
          if (res.length() > 0)
            res += (separator + L"..");
          else
            res = L"..";
          lastSegmentLength = 2;
        }
      } else {
        if (res.length() > 0)
          res += separator + path.slice(lastSlash + 1, i);
        else
          res = path.slice(lastSlash + 1, i);
        lastSegmentLength = i - lastSlash - 1;
      }
      lastSlash = i;
      dots = 0;
    } else if (code == CHAR_DOT && dots != -1) {
      ++dots;
    } else {
      dots = -1;
    }
  }
  return res;
}

namespace win32 {
  const toyo::string EOL = "\r\n";

  toyo::string normalize(const toyo::string& p) {
    toyo::string path = p;
    const int len = (int)path.length();
    if (len == 0)
      return ".";
    int rootEnd = 0;
    toyo::string* device = nullptr;
    bool isAbsolute = false;
    const unsigned short code = path[0];

    if (len > 1) {
      if (_isPathSeparator(code)) {
        isAbsolute = true;

        if (_isPathSeparator(path[1])) {
          int j = 2;
          int last = j;
          for (; j < len; ++j) {
            if (_isPathSeparator(path[j]))
              break;
          }
          if (j < len && j != last) {
            toyo::string firstPart = path.slice(last, j);
            last = j;
            for (; j < len; ++j) {
              if (!_isPathSeparator(path[j]))
                break;
            }
            if (j < len && j != last) {
              last = j;
              for (; j < len; ++j) {
                if (_isPathSeparator(path[j]))
                  break;
              }
              if (j == len) {

                return toyo::string(L"\\\\") + firstPart + L'\\' + path.slice(last) + L'\\';
              } else if (j != last) {
                toyo::string tmp = toyo::string(L"\\\\") + firstPart + L'\\' + path.slice(last, j);
                device = new toyo::string(tmp);
                rootEnd = j;
              }
            }
          }
        } else {
          rootEnd = 1;
        }
      } else if (_isWindowsDeviceRoot(code)) {

        if (path[1] == CHAR_COLON) {
          toyo::string tmp = path.slice(0, 2);
          device = new toyo::string(tmp);
          rootEnd = 2;
          if (len > 2) {
            if (_isPathSeparator(path[2])) {
              isAbsolute = true;
              rootEnd = 3;
            }
          }
        }
      }
    } else if (_isPathSeparator(code)) {
      return "\\";
    }

    toyo::string tail;
    if (rootEnd < len) {
      tail = _normalizeString(path.slice(rootEnd), !isAbsolute, L"\\", _isPathSeparator);
    } else {
      tail = L"";
    }
    if (tail.length() == 0 && !isAbsolute)
      tail = L'.';
    if (tail.length() > 0 && _isPathSeparator(path[len - 1]))
      tail += L'\\';
    if (device == nullptr) {
      if (isAbsolute) {
        if (tail.length() > 0)
          return toyo::string(L"\\") + tail;
        else
          return "\\";
      } else if (tail.length() > 0) {
        return tail;
      } else {
        return "";
      }
    } else if (isAbsolute) {
      if (tail.length() > 0) {
        toyo::string _device = *device;
        delete device;
        return (_device + L'\\' + tail);
      } else {
        toyo::string _device = *device;
        delete device;
        return (_device + L'\\');
      }
    } else if (tail.length() > 0) {
      toyo::string _device = *device;
      delete device;
      return (_device + tail);
    } else {
      toyo::string _device = *device;
      delete device;
      return (_device);
    }
  }

  toyo::string resolve(const toyo::string& arg, const toyo::string& arg1) {
    std::vector<toyo::string> arguments = { (arg), (arg1) };

    toyo::string resolvedDevice = L"";
    toyo::string resolvedTail = L"";
    bool resolvedAbsolute = false;

    for (int i = (int)arguments.size() - 1; i >= -1; i--) {
      toyo::string path = L"";
      if (i >= 0) {
        path = arguments[i];
      } else if (resolvedDevice == L"") {
        path = (toyo::process::cwd());
      } else {
#ifdef _WIN32
        wchar_t env[MAX_PATH + 1];
        if (0 == GetEnvironmentVariableW((toyo::string(L"=") + resolvedDevice).c_strw(), env, MAX_PATH + 1)) {
          path = (process::cwd());
        } else {
          path = env;
        }
#else
        path = (process::cwd());
#endif
        if (path == L"" ||
          path.slice(0, 3).to_lower_case() != (resolvedDevice.to_lower_case() + L'\\')) {
          path = resolvedDevice + L'\\';
        }
      }

      if (path.length() == 0) {
        continue;
      }

      int len = (int)path.length();
      int rootEnd = 0;
      toyo::string device = L"";
      bool isAbsolute = false;
      const unsigned short code = path[0];

      if (len > 1) {
        if (_isPathSeparator(code)) {
          isAbsolute = true;

          if (_isPathSeparator(path[1])) {
            int j = 2;
            int last = j;
            for (; j < len; ++j) {
              if (_isPathSeparator(path[j]))
                break;
            }
            if (j < len && j != last) {
              const toyo::string firstPart = path.slice(last, j);
              last = j;
              for (; j < len; ++j) {
                if (!_isPathSeparator(path[j]))
                  break;
              }
              if (j < len && j != last) {
                last = j;
                for (; j < len; ++j) {
                  if (_isPathSeparator(path[j]))
                    break;
                }
                if (j == len) {
                  device = toyo::string(L"\\\\") + firstPart + L'\\' + path.slice(last);
                  rootEnd = j;
                } else if (j != last) {
                  device = toyo::string(L"\\\\") + firstPart + L'\\' + path.slice(last, j);
                  rootEnd = j;
                }
              }
            }
          } else {
            rootEnd = 1;
          }
        } else if (_isWindowsDeviceRoot(code)) {

          if (path[1] == CHAR_COLON) {
            device = path.slice(0, 2);
            rootEnd = 2;
            if (len > 2) {
              if (_isPathSeparator(path[2])) {
                isAbsolute = true;
                rootEnd = 3;
              }
            }
          }
        }
      } else if (_isPathSeparator(code)) {
        rootEnd = 1;
        isAbsolute = true;
      }

      if (device.length() > 0 &&
        resolvedDevice.length() > 0 && device.to_lower_case() != resolvedDevice.to_lower_case()) {
        continue;
      }

      if (resolvedDevice.length() == 0 && device.length() > 0) {
        resolvedDevice = device;
      }
      if (!resolvedAbsolute) {
        resolvedTail = path.slice(rootEnd) + L'\\' + resolvedTail;
        resolvedAbsolute = isAbsolute;
      }

      if (resolvedDevice.length() > 0 && resolvedAbsolute) {
        break;
      }
    }
    resolvedTail = _normalizeString(resolvedTail, !resolvedAbsolute, L"\\",
      _isPathSeparator);

    toyo::string res = resolvedDevice + (resolvedAbsolute ? toyo::string(L"\\") : toyo::string(L"")) + resolvedTail;

    return (res == L"" ? L"." : res);
  }

  toyo::string join(const toyo::string& arg1, const toyo::string& arg2) {
    const toyo::string warg1 = (arg1);
    const toyo::string warg2 = (arg2);

    toyo::string joined = L"";
    toyo::string firstPart = L"";

    if (warg1.length() > 0) {
      if (joined == L"")
        joined = firstPart = warg1;
      else
        joined += (toyo::string(L"\\") + warg1);
    }

    if (warg2.length() > 0) {
      if (joined == L"")
        joined = firstPart = warg2;
      else
        joined += (toyo::string(L"\\") + warg2);
    }

    if (joined == L"")
      return ".";

    bool needsReplace = true;
    int slashCount = 0;
    if (_isPathSeparator(firstPart[0])) {
      ++slashCount;
      const size_t firstLen = firstPart.length();
      if (firstLen > 1) {
        if (_isPathSeparator(firstPart[1])) {
          ++slashCount;
          if (firstLen > 2) {
            if (_isPathSeparator(firstPart[2]))
              ++slashCount;
            else {
              needsReplace = false;
            }
          }
        }
      }
    }
    if (needsReplace) {
      for (; slashCount < (int)joined.length(); ++slashCount) {
        if (!_isPathSeparator(joined[slashCount]))
          break;
      }

      if (slashCount >= 2)
        joined = (toyo::string(L"\\") + joined.slice(slashCount));
    }

    return win32::normalize(joined);
  }

  bool is_absolute(const toyo::string& p) {
    toyo::string path = (p);
    const size_t len = path.length();
    if (len == 0)
      return false;

    const unsigned short code = path[0];
    if (_isPathSeparator(code)) {
      return true;
    } else if (_isWindowsDeviceRoot(code)) {

      if (len > 2 && path[1] == CHAR_COLON) {
        if (_isPathSeparator(path[2]))
          return true;
      }
    }
    return false;
  }

  toyo::string dirname(const toyo::string& p) {
    toyo::string path = (p);
    const int len = (int)path.length();
    if (len == 0)
      return ".";
    int rootEnd = -1;
    int end = -1;
    bool matchedSlash = true;
    int offset = 0;
    unsigned short code = path[0];

    if (len > 1) {
      if (_isPathSeparator(code)) {

        rootEnd = offset = 1;

        if (_isPathSeparator(path[1])) {
          int j = 2;
          int last = j;
          for (; j < len; ++j) {
            if (_isPathSeparator(path[j]))
              break;
          }
          if (j < len && j != last) {
            last = j;
            for (; j < len; ++j) {
              if (!_isPathSeparator(path[j]))
                break;
            }
            if (j < len && j != last) {
              last = j;
              for (; j < len; ++j) {
                if (_isPathSeparator(path[j]))
                  break;
              }
              if (j == len) {
                return (path);
              }
              if (j != last) {
                rootEnd = offset = j + 1;
              }
            }
          }
        }
      } else if (_isWindowsDeviceRoot(code)) {
        if (path[1] == CHAR_COLON) {
          rootEnd = offset = 2;
          if (len > 2) {
            if (_isPathSeparator(path[2]))
              rootEnd = offset = 3;
          }
        }
      }
    } else if (_isPathSeparator(code)) {
      return (path);
    }

    for (int i = len - 1; i >= offset; --i) {
      if (_isPathSeparator(path[i])) {
        if (!matchedSlash) {
          end = i;
          break;
        }
      } else {
        matchedSlash = false;
      }
    }

    if (end == -1) {
      if (rootEnd == -1)
        return ".";
      else
        end = rootEnd;
    }
    return (path.slice(0, end));
  }

  toyo::string to_namespaced_path(const toyo::string& p) {
    if (p == "") {
      return "";
    }

    const toyo::string resolvedPath = (win32::resolve(p));

    if (resolvedPath.length() >= 3) {
      if (resolvedPath[0] == CHAR_BACKWARD_SLASH) {

        if (resolvedPath[1] == CHAR_BACKWARD_SLASH) {
          const int code = resolvedPath[2];
          if (code != CHAR_QUESTION_MARK && code != CHAR_DOT) {
            return (toyo::string(L"\\\\?\\UNC\\") + resolvedPath.slice(2));
          }
        }
      } else if (_isWindowsDeviceRoot(resolvedPath[0])) {

        if (resolvedPath[1] == CHAR_COLON &&
          resolvedPath[2] == CHAR_BACKWARD_SLASH) {
          return (toyo::string(L"\\\\?\\") + resolvedPath);
        }
      }
    }

    return p;
  }

  toyo::string basename(const toyo::string& p) {
    toyo::string path = (p);
    int start = 0;
    int end = -1;
    bool matchedSlash = true;
    int i;

    if (path.length() >= 2) {
      const unsigned short drive = path[0];
      if (_isWindowsDeviceRoot(drive)) {
        if (path[1] == CHAR_COLON)
          start = 2;
      }
    }

    for (i = (int)path.length() - 1; i >= start; --i) {
      if (_isPathSeparator(path[i])) {
        if (!matchedSlash) {
          start = i + 1;
          break;
        }
      } else if (end == -1) {
        matchedSlash = false;
        end = i + 1;
      }
    }

    if (end == -1)
      return "";
    return (path.slice(start, end));
  }

  toyo::string basename(const toyo::string& p, const toyo::string& e) {
    toyo::string path = (p);
    toyo::string ext = (e);

    int start = 0;
    int end = -1;
    bool matchedSlash = true;
    int i;

    if (path.length() >= 2) {
      const unsigned short drive = path[0];
      if (_isWindowsDeviceRoot(drive)) {
        if (path[1] == CHAR_COLON)
          start = 2;
      }
    }

    if (ext.length() > 0 && ext.length() <= path.length()) {
      if (ext.length() == path.length() && ext == path)
        return "";
      int extIdx = (int)ext.length() - 1;
      int firstNonSlashEnd = -1;
      for (i = (int)path.length() - 1; i >= start; --i) {
        const int code = path[i];
        if (_isPathSeparator(code)) {
          if (!matchedSlash) {
            start = i + 1;
            break;
          }
        } else {
          if (firstNonSlashEnd == -1) {
            matchedSlash = false;
            firstNonSlashEnd = i + 1;
          }
          if (extIdx >= 0) {
            if (code == ext[extIdx]) {
              if (--extIdx == -1) {
                end = i;
              }
            } else {
              extIdx = -1;
              end = firstNonSlashEnd;
            }
          }
        }
      }

      if (start == end)
        end = firstNonSlashEnd;
      else if (end == -1)
        end = (int)path.length();
      return (path.slice(start, end));
    }

    return win32::basename(p);
  }

  toyo::string extname(const toyo::string& p) {
    toyo::string path = (p);
    int start = 0;
    int startDot = -1;
    int startPart = 0;
    int end = -1;
    bool matchedSlash = true;
    int preDotState = 0;

    if (path.length() >= 2 &&
      path[1] == CHAR_COLON &&
      _isWindowsDeviceRoot(path[0])) {
      start = startPart = 2;
    }

    for (int i = (int)path.length() - 1; i >= start; --i) {
      const unsigned short code = path[i];
      if (_isPathSeparator(code)) {
        if (!matchedSlash) {
          startPart = i + 1;
          break;
        }
        continue;
      }
      if (end == -1) {
        matchedSlash = false;
        end = i + 1;
      }
      if (code == CHAR_DOT) {
        if (startDot == -1)
          startDot = i;
        else if (preDotState != 1)
          preDotState = 1;
      } else if (startDot != -1) {
        preDotState = -1;
      }
    }

    if (startDot == -1 ||
      end == -1 ||
      preDotState == 0 ||
      (preDotState == 1 &&
        startDot == end - 1 &&
        startDot == startPart + 1)) {
      return "";
    }
    return (path.slice(startDot, end));
  }

  toyo::string relative(const toyo::string& f, const toyo::string& t) {
    toyo::string from = (f);
    toyo::string to = (t);

    if (from == to)
      return "";

    toyo::string fromOrig = (win32::resolve(f));
    toyo::string toOrig = (win32::resolve(t));

    if (fromOrig == toOrig)
      return "";

    from = fromOrig.to_lower_case();
    to = toOrig.to_lower_case();

    if (from == to)
      return "";

    int fromStart = 0;
    for (; fromStart < (int)from.length(); ++fromStart) {
      if (from[fromStart] != CHAR_BACKWARD_SLASH)
        break;
    }
    int fromEnd = (int)from.length();
    for (; fromEnd - 1 > fromStart; --fromEnd) {
      if (from[fromEnd - 1] != CHAR_BACKWARD_SLASH)
        break;
    }
    int fromLen = (fromEnd - fromStart);

    int toStart = 0;
    for (; toStart < (int)to.length(); ++toStart) {
      if (to[toStart] != CHAR_BACKWARD_SLASH)
        break;
    }
    int toEnd = (int)to.length();
    for (; toEnd - 1 > toStart; --toEnd) {
      if (to[toEnd - 1] != CHAR_BACKWARD_SLASH)
        break;
    }
    int toLen = (toEnd - toStart);

    int length = (fromLen < toLen ? fromLen : toLen);
    int lastCommonSep = -1;
    int i = 0;
    for (; i <= length; ++i) {
      if (i == length) {
        if (toLen > length) {
          if (to[toStart + i] == CHAR_BACKWARD_SLASH) {
            return (toOrig.slice(toStart + i + 1));
          } else if (i == 2) {
            return (toOrig.slice(toStart + i));
          }
        }
        if (fromLen > length) {
          if (from[fromStart + i] == CHAR_BACKWARD_SLASH) {
            lastCommonSep = i;
          } else if (i == 2) {
            lastCommonSep = 3;
          }
        }
        break;
      }
      int fromCode = from[fromStart + i];
      int toCode = to[toStart + i];
      if (fromCode != toCode)
        break;
      else if (fromCode == CHAR_BACKWARD_SLASH)
        lastCommonSep = i;
    }

    if (i != length && lastCommonSep == -1) {
      return (toOrig);
    }

    toyo::string out = L"";
    if (lastCommonSep == -1)
      lastCommonSep = 0;

    for (i = fromStart + lastCommonSep + 1; i <= fromEnd; ++i) {
      if (i == fromEnd || from[i] == CHAR_BACKWARD_SLASH) {
        if (out.length() == 0)
          out += L"..";
        else
          out += L"\\..";
      }
    }

    if (out.length() > 0)
      return (out + toOrig.slice(toStart + lastCommonSep, toEnd));
    else {
      toStart += lastCommonSep;
      if (toOrig[toStart] == CHAR_BACKWARD_SLASH)
        ++toStart;
      return (toOrig.slice(toStart, toEnd));
    }
  }
} // win32

namespace posix {
  const toyo::string EOL = "n";

  toyo::string normalize(const toyo::string& p) {
    toyo::string path = (p);

    if (path.length() == 0)
      return ".";

    const bool isAbsolute = (path[0] == CHAR_FORWARD_SLASH);
    const bool trailingSeparator =
      (path[path.length() - 1] == CHAR_FORWARD_SLASH);

    toyo::string newpath = _normalizeString(path, !isAbsolute, L"/", _isPosixPathSeparator);

    if (newpath.length() == 0 && !isAbsolute)
      newpath = L'.';
    if (newpath.length() > 0 && trailingSeparator)
      newpath += L'/';

    if (isAbsolute)
      return (toyo::string(L"/") + newpath);
    return (newpath);
  }

  toyo::string resolve(const toyo::string& arg, const toyo::string& arg1) {
    std::vector<toyo::string> arguments = { (arg), (arg1) };

    toyo::string resolvedPath = L"";
    bool resolvedAbsolute = false;

    for (int i = (int)arguments.size() - 1; i >= -1 && !resolvedAbsolute; i--) {
      toyo::string path;
      if (i >= 0)
        path = arguments[i];
      else {
        path = (toyo::process::cwd());
      }

      if (path.length() == 0) {
        continue;
      }

      resolvedPath = path + L'/' + resolvedPath;
      resolvedAbsolute = (path[0] == CHAR_FORWARD_SLASH);
    }

    resolvedPath = _normalizeString(resolvedPath, !resolvedAbsolute, L"/",
      _isPosixPathSeparator);

    if (resolvedAbsolute) {
      if (resolvedPath.length() > 0)
        return (toyo::string(L"/") + resolvedPath);
      else
        return (toyo::string(L"/"));
    } else if (resolvedPath.length() > 0) {
      return (resolvedPath);
    } else {
      return ".";
    }
  }

  toyo::string join(const toyo::string& arg1, const toyo::string& arg2) {
    const toyo::string warg1 = (arg1);
    const toyo::string warg2 = (arg2);

    if (warg1 == L"" && warg2 == L"")
      return ".";
    toyo::string joined = L"";

    if (warg1.length() > 0) {
      if (joined == L"")
        joined = warg1;
      else
        joined += (toyo::string(L"/") + warg1);
    }

    if (warg2.length() > 0) {
      if (joined == L"")
        joined = warg2;
      else
        joined += (toyo::string(L"/") + warg2);
    }

    if (joined == L"")
      return ".";
    return normalize((joined));
  }

  bool is_absolute(const toyo::string& p) {
    toyo::string path = (p);
    return path.length() > 0 && path[0] == CHAR_FORWARD_SLASH;
  }

  toyo::string dirname(const toyo::string& p) {
    toyo::string path = (p);
    if (path.length() == 0)
      return ".";
    const bool hasRoot = (path[0] == CHAR_FORWARD_SLASH);
    int end = -1;
    bool matchedSlash = true;
    for (int i = (int)path.length() - 1; i >= 1; --i) {
      if (path[i] == CHAR_FORWARD_SLASH) {
        if (!matchedSlash) {
          end = i;
          break;
        }
      } else {
        matchedSlash = false;
      }
    }

    if (end == -1)
      return hasRoot ? "/" : ".";
    if (hasRoot && end == 1)
      return "//";
    return (path.slice(0, end));
  }

  toyo::string to_namespaced_path(const toyo::string& path) {
    return path;
  }

  toyo::string basename(const toyo::string& p) {
    toyo::string path = (p);
    int start = 0;
    int end = -1;
    bool matchedSlash = true;
    int i;

    for (i = (int)path.length() - 1; i >= 0; --i) {
      if (path[i] == CHAR_FORWARD_SLASH) {
        if (!matchedSlash) {
          start = i + 1;
          break;
        }
      } else if (end == -1) {
        matchedSlash = false;
        end = i + 1;
      }
    }

    if (end == -1)
      return "";
    return (path.slice(start, end));
  }

  toyo::string basename(const toyo::string& p, const toyo::string& e) {
    toyo::string path = (p);
    toyo::string ext = (e);

    int start = 0;
    int end = -1;
    bool matchedSlash = true;
    int i;

    if (ext.length() > 0 && ext.length() <= path.length()) {
      if (ext.length() == path.length() && ext == path)
        return "";
      int extIdx = (int)ext.length() - 1;
      int firstNonSlashEnd = -1;
      for (i = (int)path.length() - 1; i >= 0; --i) {
        const unsigned short code = path[i];
        if (code == CHAR_FORWARD_SLASH) {
          if (!matchedSlash) {
            start = i + 1;
            break;
          }
        } else {
          if (firstNonSlashEnd == -1) {
            matchedSlash = false;
            firstNonSlashEnd = i + 1;
          }
          if (extIdx >= 0) {
            if (code == ext[extIdx]) {
              if (--extIdx == -1) {
                end = i;
              }
            } else {
              extIdx = -1;
              end = firstNonSlashEnd;
            }
          }
        }
      }

      if (start == end)
        end = firstNonSlashEnd;
      else if (end == -1)
        end = (int)path.length();
      return (path.slice(start, end));
    }

    return posix::basename(p);
  }

  toyo::string extname(const toyo::string& p) {
    toyo::string path = (p);
    int startDot = -1;
    int startPart = 0;
    int end = -1;
    bool matchedSlash = true;
    int preDotState = 0;
    for (int i = (int)path.length() - 1; i >= 0; --i) {
      const unsigned short code = path[i];
      if (code == CHAR_FORWARD_SLASH) {
        if (!matchedSlash) {
          startPart = i + 1;
          break;
        }
        continue;
      }
      if (end == -1) {
        matchedSlash = false;
        end = i + 1;
      }
      if (code == CHAR_DOT) {
        if (startDot == -1)
          startDot = i;
        else if (preDotState != 1)
          preDotState = 1;
      } else if (startDot != -1) {
        preDotState = -1;
      }
    }

    if (startDot == -1 ||
      end == -1 ||
      preDotState == 0 ||
      (preDotState == 1 &&
        startDot == end - 1 &&
        startDot == startPart + 1)) {
      return "";
    }
    return (path.slice(startDot, end));
  }

  toyo::string relative(const toyo::string& f, const toyo::string& t) {
    toyo::string from = (f);
    toyo::string to = (t);

    if (from == to)
      return "";

    from = (posix::resolve(f));
    to = (posix::resolve(t));

    if (from == to)
      return "";

    int fromStart = 1;
    for (; fromStart < (int)from.length(); ++fromStart) {
      if (from[fromStart] != CHAR_FORWARD_SLASH)
        break;
    }
    int fromEnd = (int)from.length();
    int fromLen = (fromEnd - fromStart);

    int toStart = 1;
    for (; toStart < (int)to.length(); ++toStart) {
      if (to[toStart] != CHAR_FORWARD_SLASH)
        break;
    }
    int toEnd = (int)to.length();
    int toLen = (toEnd - toStart);

    // Compare paths to find the longest common path from root
    int length = (fromLen < toLen ? fromLen : toLen);
    int lastCommonSep = -1;
    int i = 0;
    for (; i <= length; ++i) {
      if (i == length) {
        if (toLen > length) {
          if (to[toStart + i] == CHAR_FORWARD_SLASH) {
            return (to.slice(toStart + i + 1));
          } else if (i == 0) {
            return (to.slice(toStart + i));
          }
        } else if (fromLen > length) {
          if (from[fromStart + i] == CHAR_FORWARD_SLASH) {
            lastCommonSep = i;
          } else if (i == 0) {
            lastCommonSep = 0;
          }
        }
        break;
      }
      int fromCode = from[fromStart + i];
      int toCode = to[toStart + i];
      if (fromCode != toCode)
        break;
      else if (fromCode == CHAR_FORWARD_SLASH)
        lastCommonSep = i;
    }

    toyo::string out = L"";
    for (i = fromStart + lastCommonSep + 1; i <= fromEnd; ++i) {
      if (i == fromEnd || from[i] == CHAR_FORWARD_SLASH) {
        if (out.length() == 0)
          out += L"..";
        else
          out += L"/..";
      }
    }

    if (out.length() > 0)
      return (out + to.slice(toStart + lastCommonSep));
    else {
      toStart += lastCommonSep;
      if (to[toStart] == CHAR_FORWARD_SLASH)
        ++toStart;
      return (to.slice(toStart));
    }
  }
} // posix

toyo::string normalize(const toyo::string& p) {
#ifdef _WIN32
  return win32::normalize(p);
#else
  return posix::normalize(p);
#endif
}

bool is_absolute(const toyo::string& p) {
#ifdef _WIN32
  return win32::is_absolute(p);
#else
  return posix::is_absolute(p);
#endif
}

toyo::string dirname(const toyo::string& p) {
#ifdef _WIN32
  return win32::dirname(p);
#else
  return posix::dirname(p);
#endif
}

toyo::string to_namespaced_path(const toyo::string& p) {
#ifdef _WIN32
  return win32::to_namespaced_path(p);
#else
  return posix::to_namespaced_path(p);
#endif
}

toyo::string basename(const toyo::string& p) {
#ifdef _WIN32
  return win32::basename(p);
#else
  return posix::basename(p);
#endif
}

toyo::string basename(const toyo::string& p, const toyo::string& ext) {
#ifdef _WIN32
  return win32::basename(p, ext);
#else
  return posix::basename(p, ext);
#endif
}

toyo::string extname(const toyo::string& p) {
#ifdef _WIN32
  return win32::extname(p);
#else
  return posix::extname(p);
#endif
}

toyo::string relative(const toyo::string& f, const toyo::string& t) {
#ifdef _WIN32
  return win32::relative(f, t);
#else
  return posix::relative(f, t);
#endif
}

toyo::string __filename() {
#if defined(_WIN32)
  wchar_t buf[MAX_PATH] = { 0 };
  DWORD code = GetModuleFileNameW(nullptr, buf, MAX_PATH);
  if (code == 0) return "";
  return (buf);
#elif defined(__APPLE__)
  char buf[1024] = { 0 };
  unsigned size = 1023;
  int code = _NSGetExecutablePath(buf, &size);
  if (code != 0) return "";
  return posix::normalize(buf);
#else
  char buf[1024] = { 0 };
  int code = readlink("/proc/self/exe", buf, 1023);
  if (code <= 0) return "";
  return buf;
#endif
}

toyo::string __dirname() {
  return dirname(__filename());
}

path::path(): dir_(""), root_(""), base_(""), name_(""), ext_("") {}

#ifdef _WIN32
path::path(const toyo::string& p): path(p, true) {}
path::path(const char* p): path(toyo::string(p), true) {}
#else
path::path(const toyo::string& p): path(p, false) {}
path::path(const char* p): path(toyo::string(p), false) {}
#endif

path::path(const toyo::string& p, bool is_win32): path() {
  if (is_win32) {
    toyo::string path = (p);

    if (path.length() == 0) {
      return;
    }

    int len = (int)path.length();
    int rootEnd = 0;
    unsigned short code = path[0];

    if (len > 1) {
      if (_isPathSeparator(code)) {

        rootEnd = 1;
        if (_isPathSeparator(path[1])) {
          int j = 2;
          int last = j;
          for (; j < len; ++j) {
            if (_isPathSeparator(path[j]))
              break;
          }
          if (j < len && j != last) {
            last = j;
            for (; j < len; ++j) {
              if (!_isPathSeparator(path[j]))
                break;
            }
            if (j < len && j != last) {
              last = j;
              for (; j < len; ++j) {
                if (_isPathSeparator(path[j]))
                  break;
              }
              if (j == len) {
                rootEnd = j;
              } else if (j != last) {
                rootEnd = j + 1;
              }
            }
          }
        }
      } else if (_isWindowsDeviceRoot(code)) {

        if (path[1] == CHAR_COLON) {
          rootEnd = 2;
          if (len > 2) {
            if (_isPathSeparator(path[2])) {
              if (len == 3) {
                toyo::string apath = (path);
                this->root_ = this->dir_ = apath;
                return;
              }
              rootEnd = 3;
            }
          } else {
            toyo::string apath = (path);
            this->root_ = this->dir_ = apath;
            return;
          }
        }
      }
    } else if (_isPathSeparator(code)) {
      toyo::string apath = (path);
      this->root_ = this->dir_ = apath;
      return;
    }

    if (rootEnd > 0) {
      this->root_ = (path.slice(0, rootEnd));
    }

    int startDot = -1;
    int startPart = rootEnd;
    int end = -1;
    bool matchedSlash = true;
    int i = (int)path.length() - 1;

    int preDotState = 0;

    for (; i >= rootEnd; --i) {
      code = path[i];
      if (_isPathSeparator(code)) {
        if (!matchedSlash) {
          startPart = i + 1;
          break;
        }
        continue;
      }
      if (end == -1) {
        matchedSlash = false;
        end = i + 1;
      }
      if (code == CHAR_DOT) {
        if (startDot == -1)
          startDot = i;
        else if (preDotState != 1)
          preDotState = 1;
      } else if (startDot != -1) {
        preDotState = -1;
      }
    }

    if (startDot == -1 ||
      end == -1 ||
      preDotState == 0 ||
      (preDotState == 1 &&
        startDot == end - 1 &&
        startDot == startPart + 1)) {
      if (end != -1) {
        toyo::string tmp = (path.slice(startPart, end));
        this->base_ = this->name_ = tmp;
      }
    } else {
      this->name_ = (path.slice(startPart, startDot));
      this->base_ = (path.slice(startPart, end));
      this->ext_ = (path.slice(startDot, end));
    }

    if (startPart > 0 && startPart != rootEnd)
      this->dir_ = (path.slice(0, startPart - 1));
    else
      this->dir_ = this->root_;
  } else {
    toyo::string path = (p);
    if (path.length() == 0) {
      return;
    }
    bool isAbsolute = (path[0] == CHAR_FORWARD_SLASH);
    int start;
    if (isAbsolute) {
      this->root_ = "/";
      start = 1;
    } else {
      start = 0;
    }
    int startDot = -1;
    int startPart = 0;
    int end = -1;
    bool matchedSlash = true;
    int i = (int)path.length() - 1;

    int preDotState = 0;

    for (; i >= start; --i) {
      const unsigned short code = path[i];
      if (code == CHAR_FORWARD_SLASH) {
        if (!matchedSlash) {
          startPart = i + 1;
          break;
        }
        continue;
      }
      if (end == -1) {
        matchedSlash = false;
        end = i + 1;
      }
      if (code == CHAR_DOT) {
        if (startDot == -1)
          startDot = i;
        else if (preDotState != 1)
          preDotState = 1;
      } else if (startDot != -1) {
        preDotState = -1;
      }
    }

    if (startDot == -1 ||
      end == -1 ||
      preDotState == 0 ||
      (preDotState == 1 &&
        startDot == end - 1 &&
        startDot == startPart + 1)) {
      if (end != -1) {
        if (startPart == 0 && isAbsolute) {
          toyo::string tmp = (path.slice(1, end));
          this->base_ = this->name_ = tmp;
        } else {
          toyo::string tmp = (path.slice(startPart, end));
          this->base_ = this->name_ = tmp;
        }
      }
    } else {
      if (startPart == 0 && isAbsolute) {
        this->name_ = (path.slice(1, startDot));
        this->base_ = (path.slice(1, end));
      } else {
        this->name_ = (path.slice(startPart, startDot));
        this->base_ = (path.slice(startPart, end));
      }
      this->ext_ = (path.slice(startDot, end));
    }

    if (startPart > 0)
      this->dir_ = (path.slice(0, startPart - 1));
    else if (isAbsolute)
      this->dir_ = "/";
  }
}

path path::parse_win32(const toyo::string& p) { return path(p, true); }

path path::parse_posix(const toyo::string& p) { return path(p, false); }

path path::parse(const toyo::string& p) { return path(p); }

toyo::string path::dir() const { return this->dir_; }
toyo::string path::root() const { return this->root_; }
toyo::string path::base() const { return this->base_; }
toyo::string path::name() const { return this->name_; }
toyo::string path::ext() const { return this->ext_; }

path& path::dir(const toyo::string& d) { this->dir_ = d; return *this; }
path& path::root(const toyo::string& r) { this->root_ = r; return *this; }
path& path::base(const toyo::string& b) { this->base_ = b; return *this; }
path& path::name(const toyo::string& n) { this->name_ = n; return *this; }
path& path::ext(const toyo::string& e) { this->ext_ = e; return *this; }

// path& path::operator=(const toyo::string& other) {
//   return this->operator=(path(other));
// }

path path::operator+(const path& p) const {
#ifdef _WIN32
  return path::parse_win32(win32::join(format_win32(), p.format_win32()));
#else
  return path::parse_posix(posix::join(format_posix(), p.format_posix()));
#endif
}

path& path::operator+=(const path& p) {
#ifdef _WIN32
  *this = path::parse_win32(win32::join(format_win32(), p.format_win32()));
#else
  *this = path::parse_posix(posix::join(format_posix(), p.format_posix()));
#endif
  return *this;
}

toyo::string path::_format(const toyo::string& sep) const {
  const toyo::string& dir = (this->dir_ == "" ?  this->root_ : this->dir_);
  const toyo::string& base = (this->base_ != "" ? this->base_ :
    (this->name_ + this->ext_));
  if (dir == "") {
    return base;
  }
  if (dir == this->root_) {
    return dir + base;
  }
  return dir + sep + base;
}

toyo::string path::format_win32() const {
  return this->_format("\\");
}

toyo::string path::format_posix() const {
  return this->_format("/");
}

toyo::string path::format() const {
#ifdef _WIN32
  return format_win32();
#else
  return format_posix();
#endif
}

bool path::operator==(const path& other) {
  if (&other == this) {
    return true;
  }
  return (other.dir_ == this->dir_
    && other.root_ == this->root_
    && other.base_ == this->base_
    && other.name_ == this->name_
    && other.ext_ == this->ext_);
}

} // path

} // toyo
