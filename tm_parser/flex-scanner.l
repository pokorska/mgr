%{ /* -*- C++ -*- */
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <string>
# include "tm-driver.hh"
# include "tm-parser.out.hh"

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

"->*"       return yy::tm_parser::make_TRANSITION_INPUT(loc);
"->^"       return yy::tm_parser::make_TRANSITION_OUTPUT(loc);
"->"        return yy::tm_parser::make_TRANSITION(loc);
"START:"    return yy::tm_parser::make_INIT_STATE(loc);
"BLANK"     return yy::tm_parser::make_BLANK(loc);
"ALL"       return yy::tm_parser::make_ALL(loc);
"NOTHING"   return yy::tm_parser::make_NOTHING(loc);
"NON_ZERO"  return yy::tm_parser::make_NON_ZERO(loc);
"ZERO"      return yy::tm_parser::make_ZERO(loc);
"NEXT_CHAR" return yy::tm_parser::make_NEXT_CHAR(loc);
"PREV_CHAR" return yy::tm_parser::make_PREV_CHAR(loc);
"L"         return yy::tm_parser::make_HEAD_LEFT(loc);
"R"         return yy::tm_parser::make_HEAD_RIGHT(loc);
"-"         return yy::tm_parser::make_HEAD_STILL(loc);
{id}        return yy::tm_parser::make_STATE_NAME(yytext, loc);
{symbol}    return yy::tm_parser::make_SYMBOL(yytext[0], loc);

<<EOF>>    return yy::tm_parser::make_END(loc);
%%

void
tm_driver::scan_begin ()
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
tm_driver::scan_end ()
{
  fclose (yyin);
}
