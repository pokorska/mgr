#include "2stackPDA-driver.hh"
#include "2stackPDA-parser.out.hh"
#include "ast.h"
#include <iostream>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

std::string extractDirFromPath(const std::string& path) {
  size_t sep = path.rfind("/");
  if (sep == std::string::npos) return path;
  return path.substr(0, sep);
}

bool createDir(const std::string& dir) {
  struct stat st;
  if (stat(dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return true;
  if (mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
    std::cerr << "ERROR: Could not create directory: " << dir << "\n";
    return false;
  }
  return true;
}

_2stackPDA_driver::_2stackPDA_driver()
  : trace_scanning (false), trace_parsing (false), _minsky (false),
  debug(false), translation_file("output.mm4"), direct_translation(false),
  direct_multifile_mode(false), multifile_base("output/base") { }

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
    try {
      if (direct_translation && direct_multifile_mode) {
        std::cout << "WARNING: Both direct single and multifile modes are set.\n";
      }

      if (direct_translation)
        ast->translateToFile(translation_file);
      if (direct_multifile_mode)
        if (createDir(extractDirFromPath(multifile_base)))
          ast->translateToManyFiles(multifile_base);
      if (!direct_translation && !direct_multifile_mode)
        std::cout << ast->translate();
    } catch (const char* msg) {
      std::cout << "ERROR: (translating) " << msg << "\n";
      return;
    }
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
