#include "regex.hpp"
#include "toyo/path.hpp"
#include "cmocha/cmocha.h"

/* function match(
  glob: string,
  strUnix: string,
  strWin?: string | object,
  opts: GlobrexOptions = {}
): boolean {
  if (typeof strWin === "object") {
    opts = strWin;
    strWin = "";
  }
  const { regex } = globrex(glob, opts);
  const match = (isWin && strWin ? strWin : strUnix).match(regex);
  if (match && !regex.flags.includes("g")) {
    assertEquals(match.length, 1);
  }
  return !!match;
} */

static int match(
  const std::string& glob,
  const std::string& strUnix,
  const std::string& strWin,
  const toyo::path::globrex::globrex_options& opts
) {
  toyo::path::globrex g(glob, opts);
  auto regex = g.regex;
  std::vector<std::smatch> match;
  std::smatch sm;
#ifdef _WIN32
  std::string tmpstr = strWin != "" ? strWin : strUnix;
#else
  std::string tmpstr = strUnix;
#endif
  while(tmpstr != "" && std::regex_search(tmpstr, sm, regex)) {
    match.push_back(sm);
    tmpstr = sm.suffix();
  }
  if (match.size() > 0) {
    expect(match.size() == 1);
  }
  return match.size() > 0 ? 0 : 1;
}

static int match(
  const std::string& glob,
  const std::string& strUnix,
  const toyo::path::globrex::globrex_options& opts
) {
  return match(glob, strUnix, "", opts);
}

static int match(
  const std::string& glob,
  const std::string& strUnix
) {
  toyo::path::globrex::globrex_options opts;
  return match(glob, strUnix, opts);
}

static int match(
  const std::string& glob,
  const std::string& strUnix,
  const std::string& strWin
) {
  toyo::path::globrex::globrex_options opts;
  return match(glob, strUnix, strWin, opts);
}

int test_globrex() {
  using namespace toyo;
  path::globrex res("*.js");
  expect(res.regex_str == "^.*\\.js$");

  expect(match("*", "foo") == 0);
  expect(match("f*", "foo") == 0);
  expect(match("*o", "foo") == 0);
  expect(match("u*orn", "unicorn") == 0);
  expect(match("ico", "unicorn") == 1);
  expect(match("u*nicorn", "unicorn") == 0);

  toyo::path::globrex::globrex_options opts1;
  opts1.globstar = false;
  expect(match("*.min.js", "http://example.com/jquery.min.js", opts1) == 0);
  expect(match("*.min.*", "http://example.com/jquery.min.js", opts1) == 0);
  expect(match("*/js/*.js", "http://example.com/js/jquery.min.js", opts1) == 0);

  std::string str = "\\/$^+?.()=!|{},[].*";
  expect(match(str, str) == 0);

  expect(match(".min.", "http://example.com/jquery.min.js") == 1);
  expect(match("*.min.*", "http://example.com/jquery.min.js") == 0);
  expect(match("http:", "http://example.com/jquery.min.js") == 1);
  expect(match("http:*", "http://example.com/jquery.min.js") == 0);
  expect(match("min.js", "http://example.com/jquery.min.js") == 1);
  expect(match("*min.js", "http://example.com/jquery.min.js") == 0);

  return 0;
}
