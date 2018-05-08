#include "ast.h"

#include <unordered_set>
#include <unordered_map>

#include "names_manip.h"
#include "translation.h"

#include <iostream>

struct pairhash {
public:
  template <typename T, typename U>
  std::size_t operator()(const std::pair<T, U> &x) const
  {
    return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
  }
};

bool TransitionMap::MatchPattern(int letter, char pattern) const {
  if (pattern == mgr::ALL_CHARS) return true;
  if (pattern == mgr::NON_ZERO) return letter != '\0';
  if (pattern == mgr::ZERO) return letter == '\0';
  if (pattern == mgr::EMPTY_STACK_CHAR) return letter == mgr::EMPTY_STACK_VALUE;
  return letter == pattern;
}

TransitionMap::TransitionMap (Statement* stm_sequence) {
  stm_sequence->convert_to_transition_map(this);
  if (init_state.empty()) {
    printf("No init state defined");
    throw "No init state defined";
  }
}

void TransitionMap::setDebug(bool d) { debug = d; }

void TransitionMap::AddTransition(const TransitionRaw& t) {
  transitions[t.curr_state].emplace_back(t);
}
void TransitionMap::AddInitState(std::string name) {
  init_state = name;
}

TransitionRaw TransitionMap::FindTransition(const std::string& state, int left_letter, int right_letter) {
  for (const auto& t : transitions[state]) {
    if (MatchPattern(left_letter, t.left_pattern) && MatchPattern(right_letter, t.right_pattern))
      return t;
  }
  // Default transition returned when no match is found.
  return TransitionRaw(state, left_letter, right_letter,
                       TransitionRaw::Regular, mgr::END_STATE, "", "");
}

void TransitionMap::PushToStack(std::stack<int>* stack, const std::vector<StackSymbol*>& elems,
                 int orig_left, int orig_right, char input_char) {
  for (StackSymbol* elem : elems) {
    int to_push = elem->evaluate(orig_left, orig_right, input_char);
    if (stack->empty() && to_push != mgr::EMPTY_STACK_VALUE) {
      //std::cout << "Pushing empty stack char first.\n";
      stack->push(mgr::EMPTY_STACK_VALUE);
    }
    //std::cout << "Pushing to stack: " << (char)to_push << " value " << to_push << "\n";
    stack->push(to_push);
  }
}

void TransitionMap::print_stack(std::stack<int>* s) {
  std::stack<int> tmp;
  while (!s->empty()) {
    tmp.push(s->top());
    s->pop();
  }
  bool start = true;
  while (!tmp.empty()) {
    int val = tmp.top();
    tmp.pop();
    if (!start || val != -4)
      std::cout << val << " ";
    start = false;
    s->push(val);
  }
}

void TransitionMap::evaluate() {
  std::stack<int> left_stack, right_stack;
  left_stack.push(mgr::EMPTY_STACK_VALUE);
  right_stack.push(mgr::EMPTY_STACK_VALUE);
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
    PushToStack(&left_stack, transition.left_stack, left_top, right_top, c);
    PushToStack(&right_stack, transition.right_stack, left_top, right_top, c);
    curr_state = transition.next_state;
    if (left_stack.empty()) left_stack.push(mgr::EMPTY_STACK_VALUE);
    if (right_stack.empty()) right_stack.push(mgr::EMPTY_STACK_VALUE);

    if (debug) {
      std::cout << "  Left stack: ";
      print_stack(&left_stack);
      std::cout << "\n Right stack: ";
      print_stack(&right_stack);
      std::cout << "\nTransitioned to state: " << curr_state << "\n";
    }
  }
}

std::string TransitionMap::translate() {
  std::vector<std::string> results;
  for (const auto& item : transitions) {
    const std::string state = item.first;
    const std::vector<TransitionRaw>& transitions_v = item.second;
    build_items_recognition(state, transitions_v, &results);
  }
  std::string result = "START: " + init_state + mgr::STATEMENT_SEPARATOR;
  for (const std::string& s : results)
    result += s + mgr::STATEMENT_SEPARATOR;
  return result;
}

void TransitionMap::print_status() const {
  printf("init state: %s\nTransitions:\n", init_state.c_str());
  for (const auto& t : transitions) {
    printf("State %s has %lu transitions\n", t.first.c_str(), t.second.size());
  }
}
