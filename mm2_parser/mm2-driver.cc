#include "mm2-driver.hh"
#include "mm2-parser.out.hh"
#include "ast.h"
#include <iostream>
#include <fstream>

mm2_driver::mm2_driver()
  : trace_scanning (false), trace_parsing (false),
     ast (nullptr), tmp_ast(nullptr), enable_chunks(true),
    multifile_input(false), verbose(false) { }

mm2_driver::~mm2_driver() {
  if (ast != nullptr) delete ast;
  if (tmp_ast != nullptr) delete tmp_ast;
}

int mm2_driver::parse (const std::string& f) {
  if (multifile_input) {
    int res = parse_whole(f + "_init");
    ast->setMultifile(f);
    return res;
  }
  if (enable_chunks) return parse_in_chunks(f);
  return parse_whole(f);
}

Statement* mm2_driver::parse_to_statement(const std::string& f) {
  file = f;
  ast = nullptr;
  scan_begin ();
  yy::mm2_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  return tmp_ast;
}

int mm2_driver::parse_whole (const std::string &f)
{
  file = f;
  ast = nullptr;
  scan_begin ();
  yy::mm2_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  scan_end ();
  ast = new TransitionMap(tmp_ast);
  return res;
}

int mm2_driver::parse_helper() {
  scan_begin();
  yy::mm2_parser parser (*this);
  parser.set_debug_level (trace_parsing);
  int res = parser.parse ();
  if (ast == nullptr) ast = new TransitionMap(tmp_ast);
  else ast->Extend(tmp_ast);
  scan_end();
  return res;
}

int mm2_driver::parse_in_chunks (const std::string &f) {
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

void mm2_driver::error (const yy::location& l, const std::string& m)
{
  std::cerr << l << ": " << m << std::endl;
}

void mm2_driver::error (const std::string& m)
{
  std::cerr << m << std::endl;
}

void mm2_driver::run() {
  if (ast == nullptr) {
    std::cerr << "No ast provided\n";
    return;
  }

  ast->evaluate(verbose);
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
