%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%defines
%define parser_class_name {calcxx_parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert
%code requires
{
# include <string>
# include <iostream>
# include "ast.h"
class calcxx_driver;
}
// The parsing context.
%param { calcxx_driver& driver }

%locations
%initial-action
{
  // Initialize the initial location.
  @$.begin.filename = @$.end.filename = &driver.file;
};

%define parse.trace
%define parse.error verbose

%code
{
# include "calc++-driver.hh"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  ASSIGN  ":="
  MINUS   "-"
  PLUS    "+"
  STAR    "*"
  SLASH   "/"
  LPAREN  "("
  RPAREN  ")"
  INCREASE "INC"
  DECREASE "DEC"
  JUMP_ZERO "JZ"
  PRINT "PRINT"
  READ "READ"
;

%token <std::string> IDENTIFIER "identifier"
%token <std::string> LABEL "label"
%token <int> NUMBER "number"
//%type  <int> exp
%type <Statement*> statements
%type <Statement*> statement
%type <Statement*> minsky_unit

%printer { yyoutput << $$; } <*>;

%%
%start minsky_unit;
/*
unit: assignments exp  { driver.result = $2; };

assignments:
  %empty                 {}
| assignments assignment {};

assignment:
  "identifier" ":=" exp { driver.variables[$1] = $3; };

%left "+" "-";
%left "*" "/";
exp:
  exp "+" exp   { $$ = $1 + $3; }
| exp "-" exp   { $$ = $1 - $3; }
| exp "*" exp   { $$ = $1 * $3; }
| exp "/" exp   { $$ = $1 / $3; }
| "(" exp ")"   { std::swap ($$, $2); }
| "identifier"  { $$ = driver.variables[$1]; }
| "number"      { std::swap ($$, $1); };
*/
minsky_unit: statements { driver.minsky_result = $1; $$ = $1; }

statements:
  %empty               { $$ = new Empty(); }
| "JZ" "number" "identifier" statements { $$ = new JumpZero($2, $3, $4); }
| "label" statements { driver.labels[strip_label($1)] = $2; $$ = $2; }
| statement statements { $$ = new Sequence($1, $2); };

statement:
  "INC" "number" { $$ = new Increase($2); }
| "DEC" "number" { $$ = new Decrease($2); }
| "PRINT" "number" { $$ = new Write($2); }
| "READ" "number" { $$ = new Read($2); }

%%

void
yy::calcxx_parser::error (const location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
