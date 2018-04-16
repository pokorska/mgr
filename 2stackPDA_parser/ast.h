#ifndef TM_AST_H_
#define TM_AST_H_

#include <map>
#include <string>
#include <vector>
#include <stack>

#include "../shared_files/constants.h"

struct TransitionRaw {
  enum Type { Regular, Input, Output };
  Type type;
  char left_pattern, right_pattern, output_symbol;
  std::string curr_state, next_state;
  std::vector<int> left_stack, right_stack;

  // Constructor with vector<int> as symbols to be pushed into stacks.
  TransitionRaw(const std::string& curr_state, char left_pattern, char right_pattern,
      Type type, std::string next_state, const std::vector<int>& left_stack,
      const std::vector<int>& right_stack, char output_symbol = mgr::NO_CHAR)
    : curr_state(curr_state), left_pattern(left_pattern), right_pattern(right_pattern),
      type(type), next_state(next_state), left_stack(left_stack), right_stack(right_stack),
      output_symbol(output_symbol) { }

  // Constructor with strings as symbols to be pushed into stacks. It is exploded to separate characters
  // and stored within vector<int> internally.
  TransitionRaw(const std::string& curr_state, char left_pattern, char right_pattern,
      Type type, std::string next_state, const std::string& left_stack_str,
      const std::string& right_stack_str, char output_symbol = mgr::NO_CHAR)
    : curr_state(curr_state), left_pattern(left_pattern), right_pattern(right_pattern),
      type(type), next_state(next_state), output_symbol(output_symbol) {
    left_stack = explode(left_stack_str);
    right_stack = explode(right_stack_str);
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

  void AddTransition(TransitionRaw t) {
    transitions[t.curr_state].push_back(t);
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
    return TransitionRaw(state, left_letter, right_letter, TransitionRaw::Regular, mgr::END_STATE, "", "");
  }

  char InterpretSymbol(const TransitionRaw& t, char curr_symbol) {
    char to_write = t.output_symbol;
    if (to_write == mgr::NEXT_CHAR)
      return curr_symbol + 1;
    else if (to_write == mgr::PREV_CHAR)
      return curr_symbol - 1;
    else
      return to_write;
  }

  void PushToStack(std::stack<int>* stack, const std::vector<int>& elems,
                   int orig_left, int orig_right, char input_char = mgr::NO_CHAR) {
    for (int elem : elems) {
      int to_push = InterpretSymbol(elem, orig_left, orig_right, input_char);
      if (stack->empty() && to_push != mgr::EMPTY_STACK_CHAR) {
        //std::cout << "Pushing empty stack char first.\n";
        stack->push(mgr::EMPTY_STACK_CHAR);
      }
      //std::cout << "Pushing to stack: " << (char)to_push << " value " << to_push << "\n";
      stack->push(InterpretSymbol(elem, orig_left, orig_right, input_char));
    }
  }

  int InterpretSymbol(int symbol, int orig_left, int orig_right, int input_char) {
    if (symbol == mgr::ORIG_LEFT) return orig_left;
    if (symbol == mgr::ORIG_RIGHT) return orig_right;
    if (symbol == mgr::INPUT_SYMBOL) return input_char;
    return symbol;
  }

  void evaluate() {
    std::stack<int> left_stack, right_stack;
    left_stack.push(mgr::EMPTY_STACK_CHAR);
    right_stack.push(mgr::EMPTY_STACK_CHAR);
    std::string curr_state = init_state;
    while (curr_state != mgr::END_STATE) {
      int left_top = left_stack.top(), right_top = right_stack.top();
      left_stack.pop(); right_stack.pop();
      TransitionRaw transition = FindTransition(curr_state, left_top, right_top);
      char c = mgr::NO_CHAR;
      if (transition.type == TransitionRaw::Input) {
        do {
          std::cin >> c;
        } while (c == '\n' || c == ' ' || c == mgr::NO_CHAR);
      }
      else if (transition.type == TransitionRaw::Output)
        std::cout << (char)InterpretSymbol(transition.output_symbol, left_top, right_top, c);
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
