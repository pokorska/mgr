%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%defines
%define parser_class_name {cm_parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert
%code requires
{
# include <string>
# include <iostream>
# include "ast.h"
class cm_driver;
}
// The parsing context.
%param { cm_driver& driver }

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
# include "cm-driver.hh"
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
yy::cm_parser::error (const location_type& l,
                          const std::string& m)
{
  driver.error (l, m);
}
