#ifndef BF_DRIVER_HH
# define BF_DRIVER_HH
# include <string>
# include <map>
# include "tm-parser.out.hh"
// Tell Flex the lexer's prototype ...
# define YY_DECL \
  yy::tm_parser::symbol_type yylex (tm_driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;
// Conducting the whole scanning and parsing of Calc++.
class tm_driver
{
public:
  tm_driver ();
  virtual ~tm_driver ();

  std::vector<int> tape;
  int pos;

  TransitionMap* ast;
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

  bool turing;
  void run();
};
#endif // ! BF_DRIVER_HH
