#include "mm4-driver.hh"
#include "mm4-parser.out.hh"
#include "ast.h"
#include <iostream>

mm4_driver::mm4_driver()
  : trace_scanning (false), trace_parsing (false), _minsky (false) { }

mm4_driver::~mm4_driver() { }

int mm4_driver::parse (const std::string &f)
{
  file = f;
  ast = nullptr;
  scan_begin ();
  yy::mm4_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  return res;
}

void mm4_driver::error (const yy::location& l, const std::string& m)
{
  std::cerr << l << ": " << m << std::endl;
}

void mm4_driver::error (const std::string& m)
{
  std::cerr << m << std::endl;
}

void mm4_driver::run() {
  if (ast == nullptr) {
    std::cerr << "No ast provided\n";
    return;
  }

  if (_minsky) {
    std::cout << "Not implemented\n"; //std::cout << ast->translate();
  } else {
    ast->evaluate();
    //ast->print_status();
  }
}

std::ostream& operator<<(std::ostream& out, const TransitionRaw& t) {
  std::string arrow = "->";
  if (t.type == TransitionRaw::Input) arrow = "->*";
  if (t.type == TransitionRaw::Output) arrow = "->^";
  out << t.curr_state << " ( ";
  for (int i = 0; i < 4; ++i)
    out << t.patterns[i] << " ";
  out << ") " << arrow << " " << t.next_state << " ( ";
  for (int i = 0; i < 4; ++i) {
    out << t.changes[i] << " ";
  }
  out << " )";
  return out;
}
