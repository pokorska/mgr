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
    for (int i = 1; i < alph_size; ++i)
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
    for (int i = 1; i < alph_size; ++i)
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
          dest->push_back(create_MM4(build_name(state,i,j), 0, -1, Right, tmp_state, 0, 0));
          dest->push_back(create_MM4(tmp_state, -1, 1, Right, tmp_state, 1, -1));
          dest->push_back(create_MM4(tmp_state, -1, 0, Right,
              build_name(state,i,j) + "_RECOGNIZED_" + t->next_state, 0, 0));
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
