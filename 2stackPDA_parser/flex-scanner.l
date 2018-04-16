%{ /* -*- C++ -*- */
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <string>
# include "2stackPDA-driver.hh"
# include "2stackPDA-parser.out.hh"

// Work around an incompatibility in flex (at least versions
// 2.5.31 through 2.5.33): it generates code that does
// not conform to C89.  See Debian bug 333231
// <http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=333231>.
# undef yywrap
# define yywrap() 1

// The location of the current token.
static yy::location loc;
%}

%option noyywrap nounput batch debug noinput

symbol .
id     [a-zA-Z][a-zA-Z_0-9]+
int    [0-9]+
blank  [ \t]

%{
  // Code run each time a pattern is matched.
  # define YY_USER_ACTION  loc.columns (yyleng);
%}
%%
%{
  // Code run each time yylex is called.
  loc.step ();
%}
{blank}+   loc.step ();
[\n]+      loc.lines (yyleng); loc.step ();

"->*"        return yy::_2stackPDA_parser::make_TRANSITION_INPUT(loc);
"->^"        return yy::_2stackPDA_parser::make_TRANSITION_OUTPUT(loc);
"->"         return yy::_2stackPDA_parser::make_TRANSITION(loc);
"START:"     return yy::_2stackPDA_parser::make_INIT_STATE(loc);
"ALL"        return yy::_2stackPDA_parser::make_ALL(loc);
"NOTHING"    return yy::_2stackPDA_parser::make_NOTHING(loc);
"NON_ZERO"   return yy::_2stackPDA_parser::make_NON_ZERO(loc);
"ZERO"       return yy::_2stackPDA_parser::make_ZERO(loc);
"NEXT_CHAR"  return yy::_2stackPDA_parser::make_NEXT_CHAR(loc);
"PREV_CHAR"  return yy::_2stackPDA_parser::make_PREV_CHAR(loc);
"ORIG_LEFT"  return yy::_2stackPDA_parser::make_ORIG_LEFT(loc);
"ORIG_RIGHT" return yy::_2stackPDA_parser::make_ORIG_RIGHT(loc);
"INPUT_CHAR" return yy::_2stackPDA_parser::make_INPUT_CHAR(loc);
"BLANK"      return yy::_2stackPDA_parser::make_BLANK(loc);
"Output:"    return yy::_2stackPDA_parser::make_OUTPUT(loc);
"("          return yy::_2stackPDA_parser::make_LPAREN(loc);
")"          return yy::_2stackPDA_parser::make_RPAREN(loc);
"+"          return yy::_2stackPDA_parser::make_PLUS(loc);
"$"          return yy::_2stackPDA_parser::make_EMPTY_STACK(loc);
\"[^\"]+\"   {
               std::string inner_string = &yytext[1]; // Removing " symbol from the beginning.
               inner_string.pop_back(); // Removing " symbol from the end.
               return yy::_2stackPDA_parser::make_STRING(inner_string, loc);
             }
{id}         return yy::_2stackPDA_parser::make_STATE_NAME(yytext, loc);
{symbol}     return yy::_2stackPDA_parser::make_SYMBOL(yytext[0], loc);

<<EOF>>    return yy::_2stackPDA_parser::make_END(loc);
%%

void
_2stackPDA_driver::scan_begin ()
{
  yy_flex_debug = trace_scanning;
  if (file.empty () || file == "-")
    yyin = stdin;
  else if (!(yyin = fopen (file.c_str (), "r")))
    {
      error ("cannot open " + file + ": " + strerror(errno));
      exit (EXIT_FAILURE);
    }
}
void
_2stackPDA_driver::scan_end ()
{
  fclose (yyin);
}
