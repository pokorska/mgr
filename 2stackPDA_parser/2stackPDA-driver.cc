#include "2stackPDA-driver.hh"
#include "2stackPDA-parser.out.hh"
#include "ast.h"
#include <iostream>
#include <vector>

_2stackPDA_driver::_2stackPDA_driver()
  : trace_scanning (false), trace_parsing (false), _minsky (false), debug(false) { }

_2stackPDA_driver::~_2stackPDA_driver() { }

int _2stackPDA_driver::parse (const std::string &f)
{
  file = f;
  ast = nullptr;
  scan_begin ();
  yy::_2stackPDA_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = -1;
  try {
  res = parser.parse ();
  } catch (const char* exc) {
    std::cerr << "ERROR: " << exc << "\n";
  }
  scan_end ();
  return res;
}

void _2stackPDA_driver::error (const yy::location& l, const std::string& m)
{
  std::cerr << l << ": " << m << std::endl;
}

void _2stackPDA_driver::error (const std::string& m)
{
  std::cerr << m << std::endl;
}

void _2stackPDA_driver::run() {
  if (ast == nullptr) {
    std::cerr << "No ast provided\n";
    return;
  }

  if (debug) ast->setDebug(true);

  if (_minsky) {
    std::cout << "unimplemented\n"; //ast->translate();
  } else {
    ast->evaluate();
    //ast->print_status();
  }
}

std::ostream& operator<<(std::ostream& out, const TransitionRaw& t) {
  std::string arrow = "->";
  if (t.type == TransitionRaw::Input) arrow = "->*";
  if (t.type == TransitionRaw::Output) arrow = "->^";
  out << t.curr_state << " " << t.left_pattern << " " << t.right_pattern << " "
      << arrow << " " << t.next_state << " " << "<output>";//t.output_symbol;
  return out;
}
/*
std::ostream& operator<<(std::ostream& out, const std::vector<int>& v) {
  for (int x : v)
    out << x << " ";
}
*/
