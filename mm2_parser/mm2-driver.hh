#ifndef BF_DRIVER_HH
# define BF_DRIVER_HH
# include <string>
# include <map>
# include "mm2-parser.out.hh"
#include "../shared_files/constants.h"
// Tell Flex the lexer's prototype ...
# define YY_DECL \
  yy::mm2_parser::symbol_type yylex (mm2_driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;
// Conducting the whole scanning and parsing of Calc++.
class mm2_driver
{
public:
  mm2_driver ();
  virtual ~mm2_driver ();

  TransitionMap* ast;
  Statement* tmp_ast;
// Handling the scanner.
  void scan_begin ();
  void scan_end ();
  bool trace_scanning;
  // Run the parser on file F.
  // Return 0 on success.
  int parse (const std::string& f);
  Statement* parse_to_statement(const std::string& f);
  int parse_whole(const std::string& f);
  int parse_in_chunks(const std::string& f);
  int parse_helper();
  // The name of the file being parsed.
  // Used later to pass the file name to the location tracker.
  std::string file;
  // Whether parser traces should be generated.
  bool trace_parsing;
  // Error handling.
  void error (const yy::location& l, const std::string& m);
  void error (const std::string& m);

  void run();

  bool enable_chunks;
  bool multifile_input;
  bool verbose; // Prints all states and counters while evaluation.
  int chunk_size = mgr::DEFAULT_CHUNK_SIZE;
};
#endif // ! BF_DRIVER_HH
