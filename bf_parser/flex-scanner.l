%{ /* -*- C++ -*- */
# include <cerrno>
# include <climits>
# include <cstdlib>
# include <string>
# include "bf-driver.hh"
# include "bf-parser.out.hh"

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

id    [a-zA-Z][a-zA-Z_0-9]*
int   [0-9]+
blank [ \t]

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

">"      return yy::bf_parser::make_MOVE_RIGHT(loc);
"<"      return yy::bf_parser::make_MOVE_LEFT(loc);
"+"      return yy::bf_parser::make_INCREASE(loc);
"-"      return yy::bf_parser::make_DECREASE(loc);
"."      return yy::bf_parser::make_WRITE(loc);
","      return yy::bf_parser::make_READ(loc);
"["      return yy::bf_parser::make_LOOP_BEGIN(loc);
"]"      return yy::bf_parser::make_LOOP_END(loc);

.          loc.step(); // comment
<<EOF>>    return yy::bf_parser::make_END(loc);
%%

void
bf_driver::scan_begin ()
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
bf_driver::scan_end ()
{
  fclose (yyin);
}
