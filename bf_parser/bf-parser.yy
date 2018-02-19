%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%defines
%define parser_class_name {bf_parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert
%code requires
{
# include <string>
# include <iostream>
# include "ast.h"
class bf_driver;
}
// The parsing context.
%param { bf_driver& driver }

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
# include "bf-driver.hh"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  MOVE_RIGHT ">"
  MOVE_LEFT "<"
  INCREASE "+"
  DECREASE "-"
  WRITE "."
  READ ","
  LOOP_BEGIN "["
  LOOP_END "]"
;

//%token <std::string> IDENTIFIER "identifier"
//%token <std::string> LABEL "label"
//%token <int> NUMBER "number"
//%type  <int> exp
%type <Statement*> statements
%type <Statement*> statement
%type <Statement*> bf_unit

%printer { yyoutput << $$; } <*>;

%%
%start bf_unit;
bf_unit: statements { driver.ast = $1; $$ = $1; };

statements:
//  %empty { $$ = new Empty(); }
  statement { $$ = $1; }
| statement statements { $$ = new Sequence($1, $2); };

statement:
//  %empty { $$ = new Empty(); }
  ">" { $$ = new MoveRight(); }
| "<" { $$ = new MoveLeft(); }
| "+" { $$ = new Increase(); }
| "-" { $$ = new Decrease(); }
| "." { $$ = new Write(); }
| "," { $$ = new Read(); }
| "[" statements "]" { $$ = new Loop($2); };

%%

void
yy::bf_parser::error (const location_type& l,
                      const std::string& m)
{
  driver.error (l, m);
}
