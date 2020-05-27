#include "regex.hpp"
#include "toyo/path.hpp"
#include "cmocha/cmocha.h"

int test_globrex() {
  using namespace toyo;
  path::globrex res("*.js");
  expect(res.regex_str == "^.*\\.js$");
  return 0;
}
