#include "tm-driver.hh"
#include "tm-parser.out.hh"
#include "ast.h"
#include <iostream>

tm_driver::tm_driver()
  : trace_scanning (false), trace_parsing (false), _2stackPDA (false) { }

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

  if (_2stackPDA) {
    std::cout << "No translation implemented";//ast->translate();
  } else {
    ast->evaluate();
    //ast->print_status();
  }
}

std::ostream& operator<<(std::ostream& out, const TransitionRaw& t) {
  std::string arrow = "->";
  if (t.type == TransitionRaw::Input) arrow = "->*";
  if (t.type == TransitionRaw::Output) arrow = "->^";
  out << t.curr_state << " " << t.pattern << " " << arrow << " "
      << t.next_state << " " << t.symbol_to_write;
  return out;
}
