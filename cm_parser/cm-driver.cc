#include "cm-driver.hh"
#include "cm-parser.out.hh"
#include "ast.h"

cm_driver::cm_driver ()
  : trace_scanning (false), trace_parsing (false) { }

cm_driver::~cm_driver() { }

int cm_driver::parse (const std::string &f)
{
  file = f;
  minsky_result = nullptr;
  scan_begin ();
  labels.clear();
  yy::cm_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  return res;
}

void cm_driver::error (const yy::location& l, const std::string& m)
{
  std::cerr << l << ": " << m << std::endl;
}

void cm_driver::error (const std::string& m)
{
  std::cerr << m << std::endl;
}

void cm_driver::run() {
  stacks.clear();
  if (minsky_result != nullptr)
    minsky_result->evaluate(&stacks, labels);
}

std::string strip_label(const std::string& label) {
  int pos = label.find(':');
  if (pos == std::string::npos) return label;
  return label.substr(0, pos);
}
