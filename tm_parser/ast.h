#ifndef TM_AST_H_
#define TM_AST_H_

#include <map>
#include <string>
#include <vector>

const int BLANK = 0;
const int MAX_TAPE_LENGTH = 1000;
const std::string STATEMENT_SEPARATOR = "\n";
const char ALL_CHARS = '*'; // TODO: change it to non-ASCII character.
const char NO_CHAR = '#'; // TODO: change to non-ASCII
const char NON_ZERO = '&'; // TODO: change to non-ASCII
const char NEXT_CHAR = '>'; // TODO: change to non-ASCII
const char PREV_CHAR = '<'; // TODO: change to non-ASCII
const char ZERO = '0'; // TODO: CHANGE!
const char EMPTY_STACK_CHAR = '$';
const std::string END_STATE = "END";

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

  TransitionRaw(const std::string& curr_state, char pattern, Type type,
      std::string next_state, HeadMove head_move, char symbol_to_write)
    : curr_state(curr_state), pattern(pattern), type(type),
      next_state(next_state), head_move(head_move),
      symbol_to_write(symbol_to_write) { }
};

std::ostream& operator<<(std::ostream& out, const TransitionRaw& t);

class TransitionMap;

class Statement {
 public:
  virtual void convert_to_transition_map(TransitionMap* tmap) const = 0;
};

class TransitionMap {
 private:
  std::map<std::string, std::vector<TransitionRaw>> transitions;
  std::string init_state;

  bool MatchPattern(char letter, char pattern) const {
    if (pattern == ALL_CHARS) return true;
    if (pattern == NON_ZERO) return letter != '\0';
    if (pattern == ZERO) return letter == '\0';
    return letter == pattern;
  }

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

  TransitionRaw FindTransition(const std::string& state, char letter) {
    for (const auto& t : transitions[state]) {
      if (MatchPattern(letter, t.pattern))
        return t;
    }
    // Default transition returned when no match is found.
    return TransitionRaw(state, letter, TransitionRaw::Regular, END_STATE, None, NO_CHAR);
  }

  char GetSymbolToWrite(const TransitionRaw& t, char curr_symbol) {
    char to_write = t.symbol_to_write;
    if (to_write == NO_CHAR)
      return curr_symbol;
    if (to_write == NEXT_CHAR)
      return curr_symbol + 1;
    else if (to_write == PREV_CHAR)
      return curr_symbol - 1;
    else
      return to_write;
  }

  int NewHeadPos(int curr_pos, HeadMove move) {
    return move == Right ? curr_pos + 1 : move == Left ? curr_pos - 1 : curr_pos;
  }

  void ExtendTape(std::vector<int>* tape, int curr_pos) {
    while (curr_pos >= tape->size())
      tape->push_back(BLANK);
  }

  void evaluate() {
    std::vector<int> tape;
    int head = 0;
    tape.push_back(BLANK);
    std::string curr_state = init_state;
    while (curr_state != END_STATE) {
      TransitionRaw transition = FindTransition(curr_state, tape[head]);
      if (transition.type == TransitionRaw::Input) {
        char c = ' ';
        do {
          std::cin >> c;
        } while (c == '\n' || c == ' ');
        tape[head] = (int) c;
      }
      else if (transition.type == TransitionRaw::Output)
        std::cout << (char)tape[head];
      tape[head] = GetSymbolToWrite(transition, tape[head]);
      curr_state = transition.next_state;
      head = NewHeadPos(head, transition.head_move);
      if (head < 0) {
        curr_state = END_STATE;
        break;
      }
      ExtendTape(&tape, head);
    }
  }

  std::string transitionTypeToString(TransitionRaw::Type t) {
    return t == TransitionRaw::Input ? "->*" : t == TransitionRaw::Output ? "->^" : "->";
  }

  std::string buildTapeExtension(const TransitionRaw& t) {
    std::string result = "";
    std::string input_handling = (t.type == TransitionRaw::Input) ? "INPUT_CHAR" : "ORIG_LEFT";
    std::string output_char_section = (t.type == TransitionRaw::Output) ? " Output: ORIG_LEFT" : "";
    result += t.curr_state + " " + t.pattern + " " + EMPTY_STACK_CHAR + " " +
              transitionTypeToString(t.type) + " " + t.next_state + " (" + input_handling + " + BLANK) " +
              EMPTY_STACK_CHAR + output_char_section;
    return result;
  }

  std::string translate() {
    std::string result = "START: " + init_state + STATEMENT_SEPARATOR;
    for (const auto& alt : transitions) {
      for (const TransitionRaw& t : alt.second) {
        // Additional transition for handling tape extensions (inserting BLANKs).
        if (t.head_move == Right)
          result += buildTapeExtension(t) + STATEMENT_SEPARATOR;

        std::string input_handling = "ORIG_LEFT";
        if (t.symbol_to_write != NO_CHAR)
          input_handling = std::string("\"") + t.symbol_to_write + "\"";
        // Warning: Defining read transition OVERWRITES symbol on the tape with symbol read from stdin.
        if (t.type == TransitionRaw::Input)
          input_handling = "INPUT_CHAR";
        std::string output_char_section = (t.type == TransitionRaw::Output) ? " Output: ORIG_LEFT" : "";
        std::string left_stack = "", right_stack = "";
        if (t.head_move == Right) {
          left_stack = "(" + input_handling + " + ORIG_RIGHT)";
          right_stack = "NOTHING";
        }
        else if (t.head_move == Left) {
          left_stack = "NOTHING";
          right_stack = "(ORIG_RIGHT + " + input_handling + ")";
        } else {
          left_stack = input_handling;
          right_stack = "ORIG_RIGHT";
        }
        result += t.curr_state + " " + t.pattern + " " + ALL_CHARS + " " +
                  transitionTypeToString(t.type) + " " + t.next_state + " " +
                  left_stack + " " + right_stack +
                  output_char_section + STATEMENT_SEPARATOR;
      }
    }
    return result;
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
