%skeleton "lalr1.cc" /* -*- C++ -*- */
%require "3.0.4"
%defines
%define parser_class_name {mm2_parser}

%define api.token.constructor
%define api.value.type variant
%define parse.assert
%code requires
{
# include <string>
# include <iostream>
# include "ast.h"
# include "../shared_files/bignum.h"
class mm2_driver;
}
// The parsing context.
%param { mm2_driver& driver }

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
# include "mm2-driver.hh"
}

%define api.token.prefix {TOK_}
%token
  END  0  "end of file"
  TRANSITION "->"
  TRANSITION_INPUT "->*"
  TRANSITION_OUTPUT "->^"
  INIT_STATE "INIT"
  ANY "_"
  LPAREN "("
  RPAREN ")"
  DECREASE "-1"
  FLUSH
  LOAD
  NOOP
  OUTPUT "Output:"
;

%token <std::string> BIGNUM
%token <int> NUM_0_PLUS
%token <std::string> STATE_NAME "identifier"
%type <int> single_pattern
%type < std::pair<int,int> > pattern
%type <TransitionRaw::InputOperation> input_operation
%type <int> output_operation
%type <std::string> single_counter
%type < std::pair<std::string,std::string> > counters
%type <Statement*> init_state
%type <Statement*> transition
%type <Statement*> transitions
%type <Statement*> statements
%type <Statement*> mm2_unit

%printer { yyoutput << "not supported";/*$$;*/ } <*>;

%%
%start mm2_unit;
mm2_unit: statements { $$ = $1; driver.tmp_ast = $$; }

statements:
  transition { $$ = $1; }
| transition statements { $$ = new Sequence($1, $2); }
| init_state { $$ = $1; }
| init_state transitions { $$ = new Sequence($1, $2); }

transitions:
  transition { $$ = $1; }
| transitions transition { $$ = new Sequence($1, $2); }

init_state: "INIT" STATE_NAME { $$ = new InitState($2); }

single_pattern:
  NUM_0_PLUS { $$ = $1; }
| "_" { $$ = -1; }

pattern:
  "(" single_pattern single_pattern ")"
    {
      $$ = std::make_pair($2,$3);;
    }

single_counter:
  BIGNUM { $$ = $1; }
| NUM_0_PLUS { $$ = std::to_string($1); }
| "-1" { $$ = std::to_string(-1); }

counters:
  "(" single_counter single_counter ")"
    {
      $$ = std::make_pair($2,$3);
    }

input_operation:
  LOAD { $$ = TransitionRaw::Load; }
| "-1" { $$ = TransitionRaw::Decrease; }
| NOOP { $$ = TransitionRaw::NothingIn; }

output_operation:
  NUM_0_PLUS { $$ = $1; }
| FLUSH { $$ = -1; }

transition:
  "identifier" pattern "->" "identifier" counters
    { $$ = new Transition(TransitionRaw($1, $2, TransitionRaw::Regular, $4, $5)); }
| "identifier" pattern single_pattern "->*" "identifier" counters input_operation
    { $$ = new Transition(TransitionRaw($1, $2, $3, TransitionRaw::Input, $5, $6, $7)); }
| "identifier" pattern "->^" "identifier" counters "Output:" output_operation
    { TransitionRaw::OutputOperation output_op =
        ($7 == -1) ? TransitionRaw::Flush : TransitionRaw::Increase;
      $$ = new Transition(TransitionRaw($1, $2, TransitionRaw::Output, $4, $5, output_op, $7)); }

%%

void
yy::mm2_parser::error (const location_type& l,
                       const std::string& m)
{
  driver.error (l, m);
}
