#include "ast.h"

#include <iostream>

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

std::string c_to_string(int counter) {
  if (counter == -1) return "_";
  if (counter == 0) return "0";
  if (counter == 1) return "1";
  std::cout << "ERROR: Unknown counter state expected: " << counter << "\n";
  return "_";
}

std::string create_no_action_MM4(const std::string& state, const std::string& target) {
  return state + " (_ _ _ _) -> " + target + " (0 0 0 0)";
}

std::string create_MM4(const std::string& state, int c1_in, int c2_in, int stack,
    const std::string& target, int c1_out, int c2_out) {
  //std::cout << "> STATE: " << state << " TARGET: " << target << "\n";
  const std::string counters_in = c_to_string(c1_in) + " " + c_to_string(c2_in);
  const std::string counters_out = std::to_string(c1_out) + " " + std::to_string(c2_out);
  return state + " (" + (stack > 0 ? "_ _ " + counters_in : counters_in + " _ _") + ") -> "
      + target + " (" + (stack > 0 ? "0 0 " + counters_out : counters_out + " 0 0") + ")";
}

void build_char_recognition(const std::string& state,
    std::map<int,std::string> out_map, std::string default_out,
    std::vector<std::string>* dest, int stack) {
  const int alph_size = mgr::DEFAULT_ALPHABET_SIZE; // TODO: It should be given as parameter.
  const std::string inner_base = state + "_" + (stack > 0 ? "R" : "L");
  dest->push_back(create_MM4(state, -1, -1, stack, inner_base + "0", 0, 0));
  for (int i = 0; i < alph_size; ++i) {
    dest->push_back(create_MM4(inner_base + std::to_string(i), 1, -1, stack,
        inner_base + std::to_string((i+1) % alph_size), -1, i == alph_size-1 ? 1 : 0));
    if (out_map.count(i) > 0) {
      dest->push_back(create_MM4(inner_base + std::to_string(i), 0, -1, stack,
          out_map[i] + "_tmp", 0, 0));
      // Moving counter back to its position.
      //std::cout << ">> STATE: " << out_map[i] << "\n";
      dest->push_back(create_MM4(out_map[i] + "_tmp", -1, 1, stack,
          out_map[i] + "_tmp", 1, -1));
      dest->push_back(create_MM4(out_map[i] + "_tmp", -1, 0, stack,
          out_map[i], 0, 0));
    } else if (default_out != mgr::END_STATE) {
      dest->push_back(create_MM4(inner_base + std::to_string(i), 0, -1, stack,
          default_out + "_tmp", 0, 0));
      // Moving counter back to its position.
      //std::cout << ">> DEFAULT: " << default_out << "\n";
      dest->push_back(create_MM4(default_out + "_tmp", -1, 1, stack,
          default_out + "_tmp", 1, -1));
      dest->push_back(create_MM4(default_out + "_tmp", -1, 0, stack,
          default_out, 0, 0));
    }
  }
}

bool add_to_recognition_outmap(char pattern, const std::string& base,
    std::map<int, std::string>* outmap) {
  if (pattern == mgr::NON_ZERO) return false;
  if (pattern == mgr::ALL_CHARS) return false;
  (*outmap)[(int)pattern+1] = base + "_" + pattern;
  return true;
}

std::string TransitionMap::translate() {
  std::vector<std::string> results;
  for (const auto& item : transitions) {
    const std::string state = item.first;
    const std::vector<TransitionRaw>& transitions_v = item.second;
    //std::cout << state << " " << transitions.size() << "\n";
    std::map<char, std::vector<TransitionRaw>> left_out;
    std::map<int,std::string> recognition_outmap;
    std::string default_out = mgr::END_STATE;
    for (const TransitionRaw& t : transitions_v)
      left_out[t.left_pattern].push_back(t);
    for (const auto& group : left_out) {
      if (!add_to_recognition_outmap(group.first, state, &recognition_outmap))
        default_out = state + "_" + group.first;
    }
    //std::cout << "> DEFAULT (1): " << default_out << "\n";
    build_char_recognition(state, recognition_outmap, default_out, &results, 0);
    // For each group of recognized patterns in left stack we need to build
    // structure for recognizing elements on the right stack.
    for (const auto& group : left_out) {
      const std::string beg_state = state + "_" + group.first;
      std::map<int, std::string> recognition_outmap;
      std::string default_out = mgr::END_STATE;
      for (const TransitionRaw& t : group.second) {
        if (!add_to_recognition_outmap(t.right_pattern, beg_state, &recognition_outmap))
          default_out = beg_state + "_" + t.right_pattern;
      }
      //std::cout << "> DEFAULT: " << default_out << "\n";
      build_char_recognition(beg_state, recognition_outmap, default_out, &results, 1);

      // Creating final return to target state (after recognizing both stack elements).
      for (const TransitionRaw& t : group.second) 
        results.push_back(create_no_action_MM4(beg_state + "_" + t.right_pattern, t.next_state));
    }
  }
  std::string result = "";
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
