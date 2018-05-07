#include "ast.h"

#include <unordered_set>
#include <unordered_map>

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

enum Stack { Left, Right };

std::string create_MM4(const std::string& state, int c1_in, int c2_in, Stack stack,
    const std::string& target, int c1_out, int c2_out) {
  //std::cout << "> STATE: " << state << " TARGET: " << target << "\n";
  const std::string counters_in = c_to_string(c1_in) + " " + c_to_string(c2_in);
  const std::string counters_out = std::to_string(c1_out) + " " + std::to_string(c2_out);
  return state + " (" + (stack == Right ? "_ _ " + counters_in : counters_in + " _ _") + ") -> "
      + target + " (" + (stack == Right ? "0 0 " + counters_out : counters_out + " 0 0") + ")";
}

void insert_interpreted_chars(char pattern, std::unordered_set<int>* chars,
    int alph_size = mgr::DEFAULT_ALPHABET_SIZE) {
  if (pattern == mgr::ALL_CHARS)
    for (int i = 0; i < alph_size; ++i)
      chars->insert(i);
  else if (pattern == mgr::NON_ZERO)
    for (int i = 2; i < alph_size; ++i)
      chars->insert(i);
  else if (pattern == mgr::ZERO)
    chars->insert(1);
  else
    chars->insert((int)pattern + 1);
}

std::string build_name(const std::string& base, int left, int right = -1) {
  return base + "_L" + std::to_string(left) + (right == -1 ? "" : "_R" + std::to_string(right));
}

std::vector<int> get_all_chars(char pattern, int alph_size = mgr::DEFAULT_ALPHABET_SIZE) {
  std::vector<int> result;
  if (pattern == mgr::ALL_CHARS)
    for (int i = 0; i < alph_size; ++i)
      result.push_back(i);
  else if (pattern == mgr::NON_ZERO)
    for (int i = 2; i < alph_size; ++i)
      result.push_back(i);
  else if (pattern == mgr::ZERO)
    result.push_back(1);
  else
    result.push_back((int)pattern + 1);
  return result;
}

std::string gen_next_name(const std::string& base) {
  mgr::SequenceGenerator* seq_gen = mgr::SequenceGenerator::getInstance();
  return base + std::to_string(seq_gen->getNext());
}

std::string get_curr_name(const std::string& base) {
  mgr::SequenceGenerator* seq_gen = mgr::SequenceGenerator::getInstance();
  return base + std::to_string(seq_gen->getCurrent());
}

void build_pushing_symbol(const std::string& base, int symbol, Stack stack,
    std::vector<std::string>* dest, int alph_size = mgr::DEFAULT_ALPHABET_SIZE) {
  const std::string start = get_curr_name(base);
  // Multiply by N.
  dest->push_back(create_MM4(start, 1, -1, stack, start, -1, alph_size));
  // Change state.
  dest->push_back(create_MM4(start, 0, -1, stack, gen_next_name(base), 0, 0));
  // Copy back to original counter.
  std::string state1 = get_curr_name(base);
  std::string state2 = gen_next_name(base);
  dest->push_back(create_MM4(state1, -1, 1, stack, state1, 1, -1));
  dest->push_back(create_MM4(state1, -1, 0, stack, state2, 0, 0));
  // Add given symbol.
  state1 = get_curr_name(base);
  state2 = gen_next_name(base);
  dest->push_back(create_MM4(state1, -1, -1, stack, state2, symbol, 0));
}

std::string create_output_MM4(const std::string& state, int symbol, const std::string& target) {
  return state + " (_ _ _ _) ->^ " + target + " (0 0 0 0) Output: "
      + (symbol == -1 ? "FLUSH" : std::to_string(symbol));
}

std::string create_input_MM4(const std::string& state, int in,
    const std::string& target, int out) {
  std::string input_operation = "-1";
  if (out == -2) input_operation = "LOAD";
  if (out == 0) input_operation = "NOOP";
  return state + " (_ _ _ _) " + (in == -1 ? "_" : std::to_string(in)) + " ->* " + target
      + " (0 0 0 0) " + input_operation;
}

std::string create_input_MM4(const std::string& state, int c[4], int in,
    const std::string& target, int c_out[4], int in_oper) {
  std::string input_operation = "-1";
  if (in_oper == -2) input_operation = "LOAD";
  if (in_oper == 0) input_operation = "NOOP";
  char in_str = in == -1 ? '_' : in == 0 ? '0' : '1';
  char c_str[4];
  std::string c_out_str[4];
  for (int i = 0; i < 4; ++i) {
    c_str[i] = c[i] == -1 ? '_' : c[i] == 0 ? '0' : '1';
    c_out_str[i] = std::to_string(c_out[i]);
  }
  return state + " (" + c_str[0] + " " + c_str[1] + " " + c_str[2] + " " + c_str[3] + ")"
      + " " + in_str + " ->* " + target + " (" + c_out_str[0] + " " + c_out_str[1]
      + " " + c_out_str[2] + " " + c_out_str[3] + ") " + input_operation;
}

void build_output_items(const std::string& base, int left, int right,
    const TransitionRaw& t, std::vector<std::string>* dest, int alph_size) {
  if (t.type != TransitionRaw::Output) return;
  // Push to output counter.
  const std::string state1 = get_curr_name(base);
  const std::string state2 = gen_next_name(base);
  const std::string state3 = gen_next_name(base);
  dest->push_back(create_output_MM4(state1, t.output_symbol->evaluate(left-1, right-1),
      state2));
  // Flush output counter.
  dest->push_back(create_output_MM4(state2, -1, state3));
}

void build_closing_transition(const std::string& base, const std::string& target,
    std::vector<std::string>* dest) {
  dest->push_back(create_no_action_MM4(get_curr_name(base), target));
}

void build_pushing_items(const std::string& start_state, int left, int right, int input,
    const TransitionRaw& t, std::vector<std::string>* dest,
    int alph_size = mgr::DEFAULT_ALPHABET_SIZE) {
  mgr::SequenceGenerator* seq_gen = mgr::SequenceGenerator::getInstance();
  seq_gen->reset();
  dest->push_back(create_no_action_MM4(start_state, gen_next_name(start_state)));
  for (StackSymbol* symbol : t.left_stack) {
    int symbol_to_push = 1 + symbol->evaluate(left-1, right-1, input);
    build_pushing_symbol(start_state, symbol_to_push, Left, dest, alph_size);
  }
  for (StackSymbol* symbol : t.right_stack) {
    int symbol_to_push = 1 + symbol->evaluate(left-1, right-1, input);
    build_pushing_symbol(start_state, symbol_to_push, Right, dest, alph_size);
  }
  build_output_items(start_state, left, right, t, dest, alph_size);
  build_closing_transition(start_state, t.next_state, dest);
}

bool simple_track_possible(const TransitionRaw& t) {
  if (t.left_stack.size() == 1 && t.left_stack[0]->isInputChar()) return true;
  //TODO: if (t.right_stack.size() == 1 && t.right_stack[0]->isInputChar()) return true;
  return false;
}

void build_simple_track(const std::string& state, const TransitionRaw& t, int left, int right,
    std::vector<std::string>* dest, int alph_size = mgr::DEFAULT_ALPHABET_SIZE) {
  // Assume left stack grabs input char (and nothing else), right stack may be anything
  // but cannot contain INPUT_CHAR.

  // Multiply by N.
  dest->push_back(create_MM4(state, 1, -1, Left, state, -1, alph_size));
  // Change state.
  dest->push_back(create_MM4(state, 0, -1, Left, gen_next_name(state), 0, 0));
  // Copy back to original counter.
  std::string state1 = get_curr_name(state);
  std::string state2 = gen_next_name(state);
  dest->push_back(create_MM4(state1, -1, 1, Left, state1, 1, -1));
  dest->push_back(create_MM4(state1, -1, 0, Left, state2, 0, 0));

  // Load and copy input char to the top of the left stack.
  state1 = get_curr_name(state);
  state2 = gen_next_name(state);
  std::string state3 = gen_next_name(state);
  int counters[4] = { -1,-1,-1,-1 };
  int counters_changes[4] = { 0,0,0,0 };
  dest->push_back(create_input_MM4(state1, counters, -1, state2, counters_changes, -2));
  counters_changes[0] = 1;
  dest->push_back(create_input_MM4(state2, counters, 1, state2, counters_changes, -1));
  // Notice we increase first counter once more to hande +1 shift of all ASCII chars within stack.
  dest->push_back(create_input_MM4(state2, counters, 0, state3, counters_changes, 0));

  for (StackSymbol* symbol : t.right_stack) {
    int symbol_to_push = 1 + symbol->evaluate(left-1, right-1);
    build_pushing_symbol(state, symbol_to_push, Right, dest, alph_size);
  }
  build_closing_transition(state, t.next_state, dest);
}

// Creates recognition of stack items for given state.
void build_items_recognition(const std::string& state,
    const std::vector<TransitionRaw>& transitions, std::vector<std::string>* dest) {
  const int alph_size = mgr::DEFAULT_ALPHABET_SIZE; // TODO: It should be given as parameter.
  std::unordered_map<std::pair<int,int>, const TransitionRaw*, pairhash> bindings;
  std::unordered_set<int> left_items;
  for (const TransitionRaw& t : transitions) {
    if (t.curr_state != state) continue;

    // Generate bindings.
    std::vector<int> all_left = get_all_chars(t.left_pattern);
    std::vector<int> all_right = get_all_chars(t.right_pattern);
    for (int left_char : all_left)
      for (int right_char : all_right) {
        const std::pair<int,int> chars = std::make_pair(left_char, right_char);
        // Making sure it's not yet in the map - first transition on the list should be matched.
        if (bindings.count(chars) == 0)
          bindings[std::make_pair(left_char, right_char)] = &t;
      }

    // Gather existing characters as left stack items to recognize.
    insert_interpreted_chars(t.left_pattern, &left_items, alph_size);
  }
  dest->push_back(create_no_action_MM4(state, build_name(state,0)));
  for (int i = 0; i < alph_size; ++i) {
    dest->push_back(create_MM4(build_name(state,i), 1, -1, Left,
        build_name(state, (i+1) % alph_size), -1, i == alph_size - 1 ? 1 : 0));
    if (left_items.count(i) > 0) {
      // Moving counter back to its position.
      const std::string tmp_state = build_name(state,i) + "_tmp";
      dest->push_back(create_MM4(build_name(state,i), 0, -1, Left, tmp_state, 0, 0));
      dest->push_back(create_MM4(tmp_state, -1, 1, Left, tmp_state, 1, -1));
      dest->push_back(create_MM4(tmp_state, -1, 0, Left, build_name(state,i,0), 0, 0));
      // Build inner circle for recognizing item on the right stack.
      for (int j = 0; j < alph_size; ++j) {
        dest->push_back(create_MM4(build_name(state,i,j), 1, -1, Right,
            build_name(state,i,(j+1) % alph_size), -1, j == alph_size - 1 ? 1 : 0));
        const std::pair<int,int> recognized = std::make_pair(i,j);
        if (bindings.count(recognized) > 0) { // If transition matching these chars exists
          const TransitionRaw* t = bindings[recognized];
          // Moving counter back to its position.
          const std::string tmp_state = build_name(state,i,j) + "_tmp";
          const std::string final_state = build_name(state, i, j) + "_RECOGNIZED";
          dest->push_back(create_MM4(build_name(state,i,j), 0, -1, Right, tmp_state, 0, 0));
          dest->push_back(create_MM4(tmp_state, -1, 1, Right, tmp_state, 1, -1));
          dest->push_back(create_MM4(tmp_state, -1, 0, Right, final_state, 0, 0));
          if (t->type == TransitionRaw::Input) {
            if (simple_track_possible(*t)) {
              build_simple_track(final_state, *t, i, j, dest);
            } else {
              dest->push_back(create_input_MM4(final_state, -1, final_state + "_input0", -2));
              for (int k = 0; k < alph_size; ++k) {
                const std::string state = final_state + "_input" + std::to_string(k);
                const std::string next_state = final_state + "_input" + std::to_string(k+1);
                const std::string read_input_state = state + "_in_done";
                dest->push_back(create_input_MM4(state, 1, next_state, -1));
                dest->push_back(create_input_MM4(state, 0, read_input_state, 0));
                if (k == mgr::NO_CHAR) continue; // Cannot recognize NO_CHAR.
                build_pushing_items(read_input_state, i, j, k, *t, dest, alph_size);
              }
            }
          } else {
            build_pushing_items(final_state, i, j, mgr::NO_CHAR, *t, dest, alph_size);
          }
        }
      }
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
