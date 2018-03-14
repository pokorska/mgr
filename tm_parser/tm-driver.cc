#include "tm-driver.hh"
#include "tm-parser.out.hh"
#include "ast.h"

#include <iostream>

tm_driver::tm_driver()
  : trace_scanning (false), trace_parsing (false), turing (false) { }

tm_driver::~tm_driver() { }

int tm_driver::parse (const std::string &f)
{
  file = f;
  ast = nullptr;
  scan_begin ();
  yy::tm_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  return res;
}

void tm_driver::error (const yy::location& l, const std::string& m)
{
  std::cerr << l << ": " << m << std::endl;
}

void tm_driver::error (const std::string& m)
{
  std::cerr << m << std::endl;
}

void tm_driver::run() {
  if (ast == nullptr) {
    std::cerr << "No ast provided\n";
    return;
  }

  if (turing) {
    std::cout << "No translation implemented";//ast->translate();
  } else {
    pos = 0;
    tape.clear();
    //ast->evaluate(&tape, &pos);
    ast->print_status();
  }
}

std::string strip_label(const std::string& label) {
  int pos = label.find(':');
  if (pos == std::string::npos) return label;
  return label.substr(0, pos);
}
