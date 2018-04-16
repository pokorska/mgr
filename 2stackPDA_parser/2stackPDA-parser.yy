%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%defines
%define parser_class_name {_2stackPDA_parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert
%code requires
{
# include <string>
# include <iostream>
# include "ast.h"
# include "../shared_files/constants.h"
class _2stackPDA_driver;
}
// The parsing context.
%param { _2stackPDA_driver& driver }

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
# include "2stackPDA-driver.hh"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  TRANSITION "->"
  TRANSITION_INPUT "->*"
  TRANSITION_OUTPUT "->^"
  INIT_STATE "INIT"
  ALL "ALL"
  NOTHING "NOTHING"
  NON_ZERO "NON_ZERO"
  ZERO "ZERO"
  NEXT_CHAR "NEXT_CHAR"
  PREV_CHAR "PREV_CHAR"
  ORIG_LEFT "ORIG_LEFT"
  ORIG_RIGHT "ORIG_RIGHT"
  INPUT_CHAR "INPUT_CHAR"
  EMPTY_STACK "EMPTY_STACK"
  BLANK "B"
  OUTPUT "OUTPUT"
  LPAREN "("
  RPAREN ")"
  PLUS "+"
;

%token <std::string> STATE_NAME "identifier"
%token <std::string> STRING "string"
%token <char> SYMBOL "symbol"
%type <char> pattern
%type <TransitionRaw::Type> transition_regular
%type <TransitionRaw::Type> transition_in
%type <TransitionRaw::Type> transition_out
%type <char> output_symbol
%type <std::vector<int> > stack_items
%type <std::vector<int> > stack_items_no_input
%type <std::vector<int> > general_items
%type <std::vector<int> > items_no_input
%type <std::vector<int> > item
%type <std::vector<int> > item_no_input
// TODO: vector<int> should be the type
%type <char> output_items
%type <Statement*> init_state
%type <Statement*> transition
%type <Statement*> transitions
%type <Statement*> statements
%type <TransitionMap*> _2stackPDA_unit

%printer { for (int x : $$) yyoutput << x << " "; } stack_items stack_items_no_input general_items items_no_input item item_no_input
%printer { yyoutput << $$; } <*>;

%%
%start _2stackPDA_unit;
_2stackPDA_unit: statements { driver.ast = new TransitionMap($1); $$ = driver.ast; }

//_2stackPDA_unit: LPAREN "string" RPAREN { printf("Parsed string constant: %s\n", $2.c_str()); }

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
| EMPTY_STACK { $$ = mgr::EMPTY_STACK_CHAR; }

transition_regular:
  "->" { $$ = TransitionRaw::Regular; }

transition_in:
  "->*" { $$ = TransitionRaw::Input; }

transition_out:
  "->^" { $$ = TransitionRaw::Output; }

// TODO: extend.
item_no_input:
  "string" { $$ = TransitionRaw::explode($1); }
| ORIG_LEFT { $$ = std::vector<int>(1, mgr::ORIG_LEFT); }
| ORIG_RIGHT { $$ = std::vector<int>(1, mgr::ORIG_RIGHT); }
| "B" { $$ = std::vector<int>(1, mgr::BLANK); }
| EMPTY_STACK { $$ = std::vector<int>(1, mgr::EMPTY_STACK_CHAR); }

item:
  item_no_input { $$ = $1; }
| INPUT_CHAR { $$ = std::vector<int>(1, mgr::INPUT_SYMBOL); }

general_items:
  item { $$ = $1; }
| general_items "+" item { $$ = $1; $$.insert($$.end(), $3.begin(), $3.end()); }

items_no_input:
  item_no_input { $$ = $1; }
| items_no_input "+" item_no_input { $$ = $1; $$.insert($$.end(), $3.begin(), $3.end()); }

output_items:
  output_symbol { $$ = $1; }
| item { $$ = (char)$1[0]; }
| "(" general_items ")" { $$ = (char)$2[0]; }

output_symbol:
  "symbol" { $$ = $1; }
| NEXT_CHAR { $$ = '>'; }
| PREV_CHAR { $$ = '<'; }
| NOTHING { $$ = '#'; }

stack_items:
  item { $$ = $1; }
| NOTHING { $$ = std::vector<int>(); }
| "(" general_items ")" { $$ = $2; }

stack_items_no_input:
  item_no_input { $$ = $1; }
| NOTHING { $$ = std::vector<int>(); }
| "(" items_no_input ")" { $$ = $2; }

transition:
  "identifier" pattern pattern transition_regular "identifier" stack_items_no_input stack_items_no_input
    { $$ = new Transition(TransitionRaw($1, $2, $3, $4, $5, $6, $7)); }
| "identifier" pattern pattern transition_in "identifier" stack_items stack_items
    { $$ = new Transition(TransitionRaw($1, $2, $3, $4, $5, $6, $7)); }
| "identifier" pattern pattern transition_out "identifier" stack_items_no_input stack_items_no_input OUTPUT output_items
    { $$ = new Transition(TransitionRaw($1, $2, $3, $4, $5, $6, $7, $9)); }

%%

void
yy::_2stackPDA_parser::error (const location_type& l, const std::string& m)
{
  driver.error (l, m);
}
