#include <iostream>
#include "charset.hpp"
#include "process.hpp"
#include "path.hpp"
#include "fs.hpp"

#include "mocha.h"

using namespace toyo;

static int test_join() {
  expect(path::posix::join("/foo", "bar", "baz/asdf", "quux", "..", "a", "bbb") == "/foo/bar/baz/asdf/a/bbb")
  expect(path::posix::join("中文", "文件夹/1/2", "..") == "中文/文件夹/1")
  return 0;
}

static int test_normalize() {
  expect(path::posix::normalize("/foo/bar//baz/asdf/quux/..") == "/foo/bar/baz/asdf")
  expect(path::win32::normalize("C:////temp\\\\/\\/\\/foo/bar") == "C:\\temp\\foo\\bar")
  return 0;
}

static int test_absolute() {
  expect(path::posix::is_absolute("/foo/bar") == true)
  expect(path::posix::is_absolute("/baz/..") == true)
  expect(path::posix::is_absolute("qux/") == false)
  expect(path::posix::is_absolute(".") == false)

  expect(path::win32::is_absolute("//server") == true)
  expect(path::win32::is_absolute("\\\\server") == true)
  expect(path::win32::is_absolute("C:/foo/..") == true)
  expect(path::win32::is_absolute("C:\\foo\\..") == true)
  expect(path::win32::is_absolute("bar\\baz") == false)
  expect(path::win32::is_absolute("bar/baz") == false)
  expect(path::win32::is_absolute(".") == false)
  return 0;
}

static int test_dirname() {
  expect(path::dirname("/foo/bar/baz/asdf/quux") == "/foo/bar/baz/asdf")
  expect(path::dirname(".") == ".")
  expect(path::dirname("..") == ".")
  return 0;
}

static int test_basename() {
  expect(path::basename("/foo/bar/baz/asdf/quux.html") == "quux.html")
  expect(path::basename("/foo/bar/baz/asdf/quux.html", ".html") == "quux")
  expect(path::basename(".") == ".")
  expect(path::basename("..") == "..")
  return 0;
}

static int test_extname() {
  expect(path::extname("index.html") == ".html")
  expect(path::extname("index.coffee.md") == ".md")
  expect(path::extname("index.") == ".")
  expect(path::extname("index") == "")
  expect(path::extname(".index") == "")
  expect(path::extname(".index.md") == ".md")
  return 0;
}

static int test_resolve() {
#ifdef _WIN32
  expect(path::resolve("/foo/bar", "/tmp/file/") == "C:\\tmp\\file")
  expect(path::resolve("/foo/bar", "./baz") == "C:\\foo\\bar\\baz")
#else
  expect(path::resolve("/foo/bar", "/tmp/file/") == "/tmp/file")
  expect(path::resolve("/foo/bar", "./baz") == "/foo/bar/baz")
#endif

  expect(path::resolve("wwwroot", "static_files/png/", "../gif/image.gif") == path::join(toyo::process::cwd(), "wwwroot/static_files/gif/image.gif"))
  return 0;
}

static int test_relative() {
  expect(path::posix::relative("/data/orandea/test/aaa", "/data/orandea/impl/bbb") == "../../impl/bbb")
  expect(path::win32::relative("C:\\orandea\\test\\aaa", "C:\\orandea\\impl\\bbb") == "..\\..\\impl\\bbb")
  return 0;
}

static int test_class() {
  path::path p = path::path::parse_posix("/home/user/dir/file.txt");
  expect(p.root() == "/")
  expect(p.dir() == "/home/user/dir")
  expect(p.base() == "file.txt")
  expect(p.ext() == ".txt")
  expect(p.name() == "file")

  p = path::path::parse_win32("C:\\path\\dir\\file.txt");
  expect(p.root() == "C:\\")
  expect(p.dir() == "C:\\path\\dir")
  expect(p.base() == "file.txt")
  expect(p.ext() == ".txt")
  expect(p.name() == "file")

  path::path obj1;
  obj1
    .root("/ignored")
    .dir("/home/user/dir")
    .base("file.txt");
  expect(obj1.format_posix() == "/home/user/dir/file.txt")

  path::path obj2;
  obj2
    .root("/")
    .base("file.txt")
    .ext("ignored");
  expect(obj2.format_posix() == "/file.txt")

  path::path obj3;
  obj3
    .root("/")
    .name("file")
    .ext(".txt");
  expect(obj3.format_posix() == "/file.txt")

  path::path obj4;
  obj4
    .dir("C:\\path\\dir")
    .name("file.txt");
  expect(obj4.format_win32() == "C:\\path\\dir\\file.txt")

  path::path obj5 = toyo::process::cwd();

  obj5 += std::string("123");

  expect(obj5.format() == path::join(process::cwd(), "123"))

  return 0;
}

static int test_readdir() {
  auto ls = toyo::fs::readdir(path::__dirname());
  expect(ls.size() > 0)
  std::cout << "[";
  for (int i = 0; i < ls.size(); i++) {
    std::cout << "\"" << ls[i] << "\"";
    if (i != ls.size() - 1) {
      std::cout << ", ";
    }
  }
  std::cout << "]" << std::endl;

  try {
    auto ls2 = toyo::fs::readdir("notexists");
    return -1;
  } catch (const std::exception& e) {
    std::string message = e.what();
    expect(message.find("No such file or directory") != std::string::npos)
    std::cout << e.what() << std::endl;
  }

  return 0;
}

static int test_delimiter_and_sep() {
#ifdef _WIN32
  expect(path::delimiter == ";")
  expect(path::sep == "\\")
#else
  expect(path::delimiter == ":")
  expect(path::sep == "/")
#endif
  return 0;
}

static int test_exists() {
  expect(fs::exists(path::__filename()) == true)
  expect(fs::exists("notexists") == false)
  return 0;
}

int main() {
  int code = 0;
  int fail = 0;
  code = describe("path",
    test_join,
    test_normalize,
    test_absolute,
    test_dirname,
    test_basename,
    test_extname,
    test_resolve,
    test_relative,
    test_delimiter_and_sep,
    test_class);
  
  if (code != 0) {
    fail++;
    code = 0;
  }

  code = describe("fs",
    test_exists,
    test_readdir);

  if (code != 0) {
    fail++;
    code = 0;
  }

  int exit_code = fail > 0 ? -1 : 0;
  std::cout << "cwd: " << toyo::process::cwd() << std::endl;
  std::cout << "__filename: " << toyo::path::__filename() << std::endl;
  std::cout << "__dirname: " << toyo::path::__dirname() << std::endl << std::endl;
  std::cout << "exit: " << exit_code << ", pid: " << toyo::process::pid() << std::endl;
  return exit_code;
}
