#include "mm4-driver.hh"
#include "mm4-parser.out.hh"
#include "ast.h"
#include <iostream>
#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

// TODO: move to shared code.
std::string extractDirFromPath(const std::string& path) {
  size_t sep = path.rfind("/");
  if (sep == std::string::npos) return path;
  return path.substr(0, sep);
}

// TODO: move to shared file.
bool createDir(const std::string& dir) {
  struct stat st;
  if (stat(dir.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) return true;
  if (mkdir(dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
    std::cerr << "ERROR: Could not create directory: " << dir << "\n";
    return false;
  }
  return true;
}

mm4_driver::mm4_driver()
  : trace_scanning (false), trace_parsing (false),
    _minsky (false), ast (nullptr), tmp_ast(nullptr), enable_chunks(true),
    multifile_input(false), translation_out("output/base") { }

mm4_driver::~mm4_driver() {
  if (ast != nullptr) delete ast;
  if (tmp_ast != nullptr) delete tmp_ast;
}

int mm4_driver::parse (const std::string& f) {
  if (multifile_input) {
    int res = parse_whole(f + "_init");
    ast->setMultifile(f);
    return res;
  }
  if (enable_chunks) return parse_in_chunks(f);
  return parse_whole(f);
}

Statement* mm4_driver::parse_to_statement(const std::string& f) {
  file = f;
  ast = nullptr;
  scan_begin ();
  yy::mm4_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  return tmp_ast;
}

int mm4_driver::parse_whole (const std::string &f)
{
  file = f;
  ast = nullptr;
  scan_begin ();
  yy::mm4_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  ast = new TransitionMap(tmp_ast);
  return res;
}

int mm4_driver::parse_helper() {
  scan_begin();
  yy::mm4_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  if (ast == nullptr) ast = new TransitionMap(tmp_ast);
  else ast->Extend(tmp_ast);
  scan_end();
  return res;
}

int mm4_driver::parse_in_chunks (const std::string &f) {
  std::cout << "CHUNKS! Current chunk size: " << chunk_size << "\n";
  const std::string chunk_filename = "chunk.tmp";
  std::ifstream input (f, std::ifstream::in);
  std::string line;
  ast = nullptr;
  int lines_stored = 0;
  std::ofstream chunk (chunk_filename, std::ofstream::out);

  while (std::getline(input, line)) {
    chunk << line << "\n";
    lines_stored++;
    if (lines_stored >= chunk_size) {
      chunk.close();
      file = chunk_filename;
      int res = parse_helper();
      if (res != 0) {
        chunk.close(); input.close();
        return res;
      }
      chunk = std::ofstream(chunk_filename, std::ofstream::out);
      lines_stored = 0;
    }
  }
  chunk.close();
  input.close();
  int res = 0;
  if (lines_stored > 0) {
    file = chunk_filename;
    res = parse_helper();
  }
  remove(chunk_filename.c_str());
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
    //std::cout << "Not implemented\n";
    if (createDir(extractDirFromPath(translation_out)))
      ast->translate(translation_out);
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
