#ifndef BF_AST_H_
#define BF_AST_H_

#include <map>
#include <string>
#include <vector>

std::string strip_label(const std::string& label);

const int BLANK = 0;
const int MAX_TAPE_LENGTH = 1000;
const std::string STATE_NAME_BASE = "state";
const std::string STATEMENT_SEPARATOR = "\n";
const char ALL_CHARS = '*'; // TODO: change it to non-ASCII character.
const char NO_CHAR = '#'; // TODO: change to non-ASCII
const char NON_ZERO = '&'; // TODO: change to non-ASCII
const char NEXT_CHAR = '>'; // TODO: change to non-ASCII
const char PREV_CHAR = '<'; // TODO: change to non-ASCII
const char ZERO = '0'; // TODO: CHANGE!

enum HeadMove { Left, Right, None };
inline char to_char(HeadMove hm) {
  return hm == Left ? 'L' : hm == Right ? 'R' : '-';
}
inline HeadMove from_char(char c) {
  return c == 'L' ? Left : c == 'R' ? Right : c == '-' ? None
      : throw "Unknown head move";
}

struct TransitionRaw {
  enum Type { Regular, Input, Output };
  Type type;
  char pattern, symbol_to_write;
  std::string curr_state, next_state;
  HeadMove head_move;

  TransitionRaw(std::string curr_state, char pattern, Type type,
      std::string next_state, HeadMove head_move, char symbol_to_write)
    : curr_state(curr_state), pattern(pattern), type(type),
      next_state(next_state), head_move(head_move),
      symbol_to_write(symbol_to_write) { }
};

class TransitionMap;

class Statement {
 public:
  virtual void convert_to_transition_map(TransitionMap* tmap) const = 0;
};

class TransitionMap {
 private:
  std::map<std::string, std::vector<TransitionRaw>> transitions;
  std::string init_state;

 public:
  TransitionMap (Statement* stm_sequence) {
    stm_sequence->convert_to_transition_map(this);
    if (init_state.empty()) {
      printf("No init state defined");
      throw "No init state defined";
    }
  }

  void AddTransition(TransitionRaw t) {
    transitions[t.curr_state].push_back(t);
  }
  void AddInitState(std::string name) {
    init_state = name;
  }

  void print_status() const {
    printf("init state: %s\nTransitions:\n", init_state.c_str());
    for (const auto& t : transitions) {
      printf("State %s has %lu transitions\n", t.first.c_str(), t.second.size());
    }
  }
};

class InitState : public Statement {
 private:
  std::string state_name;
 public:
  InitState(std::string state_name) : state_name(state_name) { }
  virtual void convert_to_transition_map(TransitionMap* tmap) const {
    tmap->AddInitState(state_name);
  }
};

class Transition : public Statement {
 private:
  TransitionRaw t;
 public:
  Transition(TransitionRaw t) : t(t) { }
  virtual void convert_to_transition_map(TransitionMap* tmap) const {
    tmap->AddTransition(t);
  }
};

class Sequence : public Statement {
 private:
  Statement *stm1, *stm2;
 public:
  Sequence(Statement* stm1, Statement* stm2) : stm1(stm1), stm2(stm2) { }
  virtual void convert_to_transition_map(TransitionMap* tmap) const {
    stm1->convert_to_transition_map(tmap);
    stm2->convert_to_transition_map(tmap);
  }
};

#endif
