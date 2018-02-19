#ifndef CM_DRIVER_HH
# define CM_DRIVER_HH
# include <string>
# include <map>
# include "cm-parser.out.hh"
// Tell Flex the lexer's prototype ...
# define YY_DECL \
  yy::cm_parser::symbol_type yylex (cm_driver& driver)
// ... and declare it for the parser's sake.
YY_DECL;
// Conducting the whole scanning and parsing of CM.
class cm_driver
{
public:
  cm_driver ();
  virtual ~cm_driver ();

  std::map<int, int> stacks;
  std::map<std::string, Statement*> labels;

  Statement* minsky_result;
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
#endif // ! CM_DRIVER_HH
