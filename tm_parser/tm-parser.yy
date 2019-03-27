%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%defines
%define parser_class_name {tm_parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert
%code requires
{
# include <string>
# include <iostream>
# include "ast.h"
class tm_driver;
}
// The parsing context.
%param { tm_driver& driver }

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
# include "tm-driver.hh"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  TRANSITION "->"
  TRANSITION_INPUT "->*"
  TRANSITION_OUTPUT "->^"
  TRANSITION_OUTPUT_SHIFTED "->~"
  INIT_STATE "INIT"
  BLANK "BLANK"
  ALL "ALL"
  NOTHING "NOTHING"
  NON_ZERO "NON_ZERO"
  ZERO "ZERO"
  NEXT_CHAR "NEXT_CHAR"
  PREV_CHAR "PREV_CHAR"
  HEAD_LEFT "L"
  HEAD_RIGHT "R"
  HEAD_STILL "-"
;

%token <std::string> STATE_NAME "identifier"
%token <char> SYMBOL "symbol"
%type <char> pattern
%type <HeadMove> head_move
%type <TransitionRaw::Type> transition_symbol
%type <char> symbol_to_write
%type <Statement*> init_state
%type <Statement*> transition
%type <Statement*> transitions
%type <Statement*> statements
%type <TransitionMap*> tm_unit

%printer { yyoutput << $$; } <*>;

%%
%start tm_unit;
tm_unit: statements { driver.ast = new TransitionMap($1); $$ = driver.ast; }

statements:
  transition { $$ = $1; }
| transition statements { $$ = new Sequence($1, $2); }
| init_state { $$ = $1; }
| init_state transitions { $$ = new Sequence($1, $2); }

transitions:
  transition { $$ = $1; }
| transition transitions { $$ = new Sequence($1, $2); }

init_state: "INIT" STATE_NAME { $$ = new InitState($2); }

pattern:
  "symbol" { $$ = $1; }
| ALL { $$ = '*'; }
| NON_ZERO { $$ = '&'; }
| ZERO { $$ = '0'; }

transition_symbol:
  "->" { $$ = TransitionRaw::Regular; }
| "->*" { $$ = TransitionRaw::Input; }
| "->^" { $$ = TransitionRaw::Output; }
| "->~" { $$ = TransitionRaw::OutputShifted; }

head_move:
  "L" { $$ = Left; }
| "R" { $$ = Right; }
| "-" { $$ = None; }

symbol_to_write:
  "symbol" { $$ = $1; }
| NEXT_CHAR { $$ = '>'; }
| PREV_CHAR { $$ = '<'; }
| NOTHING { $$ = '#'; }

transition:
  "identifier" pattern transition_symbol "identifier" head_move symbol_to_write
    { $$ = new Transition(TransitionRaw($1, $2, $3, $4, $5, $6)); }

%%

void
yy::tm_parser::error (const location_type& l,
                      const std::string& m)
{
  driver.error (l, m);
}
