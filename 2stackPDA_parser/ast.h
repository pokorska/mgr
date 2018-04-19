#ifndef TM_AST_H_
#define TM_AST_H_

#include <map>
#include <string>
#include <vector>
#include <stack>

#include "../shared_files/constants.h"

class StackSymbol {
 public:
  virtual int evaluate(int orig_left, int orig_right, int input_symbol = mgr::NO_CHAR) = 0;
};

class SymbolRaw : public StackSymbol {
 private:
  int symbol;
 public:
  SymbolRaw(int symbol) : symbol(symbol) { }
  virtual int evaluate(int orig_left, int orig_right, int input_symbol = mgr::NO_CHAR) {
    return symbol;
  }
};

class SymbolInput : public StackSymbol {
 public:
  virtual int evaluate(int orig_left, int orig_right, int input_symbol = mgr::NO_CHAR) {
    if (input_symbol == mgr::NO_CHAR) throw "No input provided in stack symbol evaluation";
    return input_symbol;
  }
};

class SymbolOrigLeft : public StackSymbol {
 public:
  virtual int evaluate(int orig_left, int orig_right, int input_symbol = mgr::NO_CHAR) {
    return orig_left;
  }
};

class SymbolOrigRight : public StackSymbol {
 public:
  virtual int evaluate(int orig_left, int orig_right, int input_symbol = mgr::NO_CHAR) {
    return orig_right;
  }
};

class SymbolNothing : public StackSymbol {
 public:
  virtual int evaluate(int orig_left, int orig_right, int input_symbol = mgr::NO_CHAR) {
    throw "Trying to evaluate NOTHING symbol";
  }
};

class SymbolPrev : public StackSymbol {
 private:
  StackSymbol* inner;
 public:
  SymbolPrev(StackSymbol* inner) : inner(inner) { }
  ~SymbolPrev() { delete inner; }
  virtual int evaluate(int orig_left, int orig_right, int input_symbol = mgr::NO_CHAR) {
    int inner_symbol = inner->evaluate(orig_left, orig_right, input_symbol);
    return inner_symbol-1;
  }
};

class SymbolNext : public StackSymbol {
 private:
  StackSymbol* inner;
 public:
  SymbolNext(StackSymbol* inner) : inner(inner) { }
  ~SymbolNext() { delete inner; }
  virtual int evaluate(int orig_left, int orig_right, int input_symbol = mgr::NO_CHAR) {
    int inner_symbol = inner->evaluate(orig_left, orig_right, input_symbol);
    return inner_symbol+1;
  }
};

struct TransitionRaw {
  enum Type { Regular, Input, Output };
  Type type;
  char left_pattern, right_pattern;
  StackSymbol* output_symbol; // TODO: Change to sequence of symbols to write.
  std::string curr_state, next_state;
  std::vector<StackSymbol*> left_stack, right_stack;

  // Constructor with vector<int> as symbols to be pushed into stacks.
  TransitionRaw(const std::string& curr_state, char left_pattern, char right_pattern,
      Type type, std::string next_state, const std::vector<StackSymbol*>& left_stack,
      const std::vector<StackSymbol*>& right_stack, StackSymbol* output_symbol = new SymbolNothing())
    : curr_state(curr_state), left_pattern(left_pattern), right_pattern(right_pattern),
      type(type), next_state(next_state), left_stack(left_stack), right_stack(right_stack),
      output_symbol(output_symbol) { }

  // Constructor with strings as symbols to be pushed into stacks. It is exploded to separate characters
  // and stored within vector<int> internally.
  TransitionRaw(const std::string& curr_state, char left_pattern, char right_pattern,
      Type type, std::string next_state, const std::string& left_stack_str,
      const std::string& right_stack_str, StackSymbol* output_symbol = new SymbolNothing())
    : curr_state(curr_state), left_pattern(left_pattern), right_pattern(right_pattern),
      type(type), next_state(next_state), output_symbol(output_symbol) {
    std::vector<int> left_symbols, right_symbols;
    left_stack = explode_to_stack(left_stack_str);
    right_stack = explode_to_stack(right_stack_str);
  }

  static std::vector<StackSymbol*> explode_to_stack(const std::string& s) {
    std::vector<int> exploded_str = explode(s);
    std::vector<StackSymbol*> result;
    for (int symbol : exploded_str)
      result.emplace_back(new SymbolRaw(symbol));
    return result;
  }

  static std::vector<int> explode(const std::string& s) {
    std::vector<int> result;
    for (char c : s)
      result.push_back((int) c);
    return result;
  }
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
    if (pattern == mgr::ALL_CHARS) return true;
    if (pattern == mgr::NON_ZERO) return letter != '\0';
    if (pattern == mgr::ZERO) return letter == '\0';
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

  void AddTransition(const TransitionRaw& t) {
    transitions[t.curr_state].emplace_back(t);
  }
  void AddInitState(std::string name) {
    init_state = name;
  }

  TransitionRaw FindTransition(const std::string& state, char left_letter, char right_letter) {
    for (const auto& t : transitions[state]) {
      if (MatchPattern(left_letter, t.left_pattern) && MatchPattern(right_letter, t.right_pattern))
        return t;
    }
    // Default transition returned when no match is found.
    return TransitionRaw(state, left_letter, right_letter,
                         TransitionRaw::Regular, mgr::END_STATE, "", "");
  }

  void PushToStack(std::stack<int>* stack, const std::vector<StackSymbol*>& elems,
                   int orig_left, int orig_right, char input_char = mgr::NO_CHAR) {
    for (StackSymbol* elem : elems) {
      int to_push = elem->evaluate(orig_left, orig_right, input_char);
      if (stack->empty() && to_push != mgr::EMPTY_STACK_CHAR) {
        //std::cout << "Pushing empty stack char first.\n";
        stack->push(mgr::EMPTY_STACK_CHAR);
      }
      //std::cout << "Pushing to stack: " << (char)to_push << " value " << to_push << "\n";
      stack->push(to_push);
    }
  }

  void evaluate() {
    std::stack<int> left_stack, right_stack;
    left_stack.push(mgr::EMPTY_STACK_CHAR);
    right_stack.push(mgr::EMPTY_STACK_CHAR);
    std::string curr_state = init_state;
    while (curr_state != mgr::END_STATE) {
      int left_top = left_stack.top(), right_top = right_stack.top();
      left_stack.pop(); right_stack.pop();
      const TransitionRaw& transition = FindTransition(curr_state, left_top, right_top);
      char c = mgr::NO_CHAR;
      if (transition.type == TransitionRaw::Input) {
        do {
          std::cin >> c;
        } while (c == '\n' || c == ' ' || c == mgr::NO_CHAR);
      }
      else if (transition.type == TransitionRaw::Output)
        std::cout << (char)(transition.output_symbol->evaluate(left_top, right_top, c));
      //std::cout << "Left stack\n";
      PushToStack(&left_stack, transition.left_stack, left_top, right_top, c);
      //std::cout << "Right stack\n";
      PushToStack(&right_stack, transition.right_stack, left_top, right_top, c);
      //std::cout << "Stack pushes done.\n";
      curr_state = transition.next_state;
      //std::cout << "Transitioned to state: " << curr_state << "\n";
      if (left_stack.empty()) left_stack.push(mgr::EMPTY_STACK_CHAR);
      if (right_stack.empty()) right_stack.push(mgr::EMPTY_STACK_CHAR);
    }
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
