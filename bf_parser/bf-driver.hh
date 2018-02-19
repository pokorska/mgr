#ifndef BF_DRIVER_HH
# define BF_DRIVER_HH
# include <string>
# include <map>
# include "bf-parser.tab.hh"
// Tell Flex the lexer's prototype ...
# define YY_DECL \
  yy::bf_parser::symbol_type yylex (bf_driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;
// Conducting the whole scanning and parsing of Calc++.
class bf_driver
{
public:
  bf_driver ();
  virtual ~bf_driver ();

  std::vector<int> tape;
  int pos;

  Statement* ast;
// Handling the scanner.
  void scan_begin ();
  void scan_end ();
  bool trace_scanning;
  // Run the parser on file F.
  // Return 0 on success.
  int parse (const std::string& f);
  // The name of the file being parsed.
  // Used later to pass the file name to the location tracker.
  std::string file;
  // Whether parser traces should be generated.
  bool trace_parsing;
  // Error handling.
  void error (const yy::location& l, const std::string& m);
  void error (const std::string& m);

  void run();
};
#endif // ! BF_DRIVER_HH
