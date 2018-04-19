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
  NEXT "NEXT"
  PREV "PREV"
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
%type <StackSymbol*> output_symbol
%type <std::vector<StackSymbol*> > stack_items
%type <std::vector<StackSymbol*> > stack_items_no_input
%type <std::vector<StackSymbol*> > general_items
%type <std::vector<StackSymbol*> > items_no_input
%type <std::vector<StackSymbol*> > item
%type <std::vector<StackSymbol*> > item_no_input
// TODO: vector<int> should be the type
%type <StackSymbol*> output_items
%type <Statement*> init_state
%type <Statement*> transition
%type <Statement*> transitions
%type <Statement*> statements
%type <TransitionMap*> _2stackPDA_unit

%printer { for (StackSymbol* x : $$) yyoutput << "<some symbol>" << " "; } stack_items stack_items_no_input general_items items_no_input item item_no_input
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
  "string" { $$ = TransitionRaw::explode_to_stack($1); }
| ORIG_LEFT { $$ = std::vector<StackSymbol*>(1, new SymbolOrigLeft()); }
| ORIG_RIGHT { $$ = std::vector<StackSymbol*>(1, new SymbolOrigRight()); }
| "B" { $$ = std::vector<StackSymbol*>(1, new SymbolRaw(mgr::BLANK)); }
| EMPTY_STACK { $$ = std::vector<StackSymbol*>(1, new SymbolRaw(mgr::EMPTY_STACK_CHAR)); }
| NEXT "(" item_no_input ")" { $$ = std::vector<StackSymbol*>(1, new SymbolNext($3[0])); } // TODO: fix
| PREV "(" item_no_input ")" { $$ = std::vector<StackSymbol*>(1, new SymbolPrev($3[0])); } // TODO: fix

item:
  item_no_input { $$ = $1; }
| INPUT_CHAR { $$ = std::vector<StackSymbol*>(1, new SymbolInput()); }
| NEXT "(" item ")" { $$ = std::vector<StackSymbol*>(1, new SymbolNext($3[0])); } // TODO: fix
| PREV "(" item ")" { $$ = std::vector<StackSymbol*>(1, new SymbolPrev($3[0])); } // TODO: fix

general_items:
  item { $$ = $1; }
| general_items "+" item { $$ = $1; $$.insert($$.end(), $3.begin(), $3.end()); }

items_no_input:
  item_no_input { $$ = $1; }
| items_no_input "+" item_no_input { $$ = $1; $$.insert($$.end(), $3.begin(), $3.end()); }

output_items:
  output_symbol { $$ = $1; }
| item { $$ = $1[0]; } // TODO: output_items should be a vector
| "(" general_items ")" { $$ = $2[0]; } // TODO: as above

output_symbol:
  "symbol" { $$ = new SymbolRaw($1); }
| NOTHING { $$ = new SymbolNothing(); }

stack_items:
  item { $$ = $1; }
| NOTHING { $$ = std::vector<StackSymbol*>(); }
| "(" general_items ")" { $$ = $2; }

stack_items_no_input:
  item_no_input { $$ = $1; }
| NOTHING { $$ = std::vector<StackSymbol*>(); }
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
