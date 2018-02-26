#include "bf-driver.hh"
#include "bf-parser.out.hh"
#include "ast.h"

#include <iostream>

bf_driver::bf_driver()
  : trace_scanning (false), trace_parsing (false), turing (false) { }

bf_driver::~bf_driver() { }

int bf_driver::parse (const std::string &f)
{
  file = f;
  ast = nullptr;
  scan_begin ();
  yy::bf_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  return res;
}

void bf_driver::error (const yy::location& l, const std::string& m)
{
  std::cerr << l << ": " << m << std::endl;
}

void bf_driver::error (const std::string& m)
{
  std::cerr << m << std::endl;
}

void bf_driver::run() {
  if (ast == nullptr) {
    std::cerr << "No ast provided";
    return;
  }

  if (turing) {
    std::cout << ast->translate();
  } else {
    pos = 0;
    tape.clear();
    ast->evaluate(&tape, &pos);
  }
}

std::string strip_label(const std::string& label) {
  int pos = label.find(':');
  if (pos == std::string::npos) return label;
  return label.substr(0, pos);
}
