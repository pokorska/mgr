%{ /* -*- C++ -*- */
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <string>
# include "mm2-driver.hh"
# include "mm2-parser.out.hh"
#include "../shared_files/bignum.h"

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

"->*"       return yy::mm2_parser::make_TRANSITION_INPUT(loc);
"->^"       return yy::mm2_parser::make_TRANSITION_OUTPUT(loc);
"->"        return yy::mm2_parser::make_TRANSITION(loc);
"START:"    return yy::mm2_parser::make_INIT_STATE(loc);
"_"         return yy::mm2_parser::make_ANY(loc);
"("         return yy::mm2_parser::make_LPAREN(loc);
")"         return yy::mm2_parser::make_RPAREN(loc);
"-1"        return yy::mm2_parser::make_DECREASE(loc);
"FLUSH"     return yy::mm2_parser::make_FLUSH(loc);
"LOAD"      return yy::mm2_parser::make_LOAD(loc);
"NOOP"      return yy::mm2_parser::make_NOOP(loc);
"Output:"   return yy::mm2_parser::make_OUTPUT(loc);
{id}        return yy::mm2_parser::make_STATE_NAME(yytext, loc);
{int}       {
              Bignum b(yytext);
              if (b < 1e9) return yy::mm2_parser::make_NUM_0_PLUS(b.getInt(), loc);
              else return yy::mm2_parser::make_BIGNUM(std::string(yytext), loc);
            }

<<EOF>>    return yy::mm2_parser::make_END(loc);
%%

void
mm2_driver::scan_begin ()
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
mm2_driver::scan_end ()
{
  fclose (yyin);
}
