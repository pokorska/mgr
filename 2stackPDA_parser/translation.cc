#include "translation.h"

#include <unordered_set>
#include <unordered_map>

#include "names_manip.h"

struct pairhash {
public:
  template <typename T, typename U>
  std::size_t operator()(const std::pair<T, U> &x) const
  {
    return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
  }
};


void build_pushing_symbol(const std::string& base, int symbol, Stack stack,
    std::vector<std::string>* dest, int alph_size) {
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
    const TransitionRaw& t, std::vector<std::string>* dest, int alph_size) {
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
    std::vector<std::string>* dest, int alph_size) {
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
