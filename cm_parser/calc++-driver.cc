#include "calc++-driver.hh"
#include "calc++-parser.tab.hh"
#include "ast.h"

calcxx_driver::calcxx_driver ()
  : trace_scanning (false), trace_parsing (false)
{
  variables["one"] = 1;
  variables["two"] = 2;
}

calcxx_driver::~calcxx_driver ()
{
}

int
calcxx_driver::parse (const std::string &f)
{
  file = f;
  minsky_result = nullptr;
  scan_begin ();
  yy::calcxx_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  if (minsky_result != nullptr)
    minsky_result->evaluate(&stacks, labels);
  return res;
}

void
calcxx_driver::error (const yy::location& l, const std::string& m)
{
  std::cerr << l << ": " << m << std::endl;
}

void
calcxx_driver::error (const std::string& m)
{
  std::cerr << m << std::endl;
}

std::string strip_label(const std::string& label) {
  int pos = label.find(':');
  if (pos == std::string::npos) return label;
  return label.substr(0, pos);
}
