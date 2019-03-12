#include "ast.h"

#include <vector>
#include <fstream>
#include <iostream>
#include <cmath>

#define PUSH(state, outTrans) out->push_back(make_pair(state, outTrans));

using std::string;
using std::vector;
using std::pair;
using std::cout;
using std::to_string;

// Order of numbers assigned to 4 counters, the first counter will be
// represented as 2^a, second as 3^b etc. unless the order is changed.
// This is because of efficiency issues that may appear while running
// 2 counter machine code.
const int transform[] = { 2,3,5,7 };

//TODO: move to shared code
string gen_next_name(const string& base) {
  mgr::SequenceGenerator* seq_gen = mgr::SequenceGenerator::getInstance();
  return base + to_string(seq_gen->getNext());
}

//TODO: move to shared code
string get_curr_name(const string& base) {
  mgr::SequenceGenerator* seq_gen = mgr::SequenceGenerator::getInstance();
  return base + to_string(seq_gen->getCurrent());
}

//TODO: move to shared code
void reset_name_generation() {
  mgr::SequenceGenerator* seq_gen = mgr::SequenceGenerator::getInstance();
  seq_gen->reset();
}

bool matches_all(const TransitionRaw& t) {
  return t.match_mask == 0;
}

// TODO: Bignums.
string change_to_string(long long int counter) {
  if (counter < -1) {
    cout << "ERROR: invalid counter change: " << counter << "\n";
    return "0";
  }
  return std::to_string(counter);
}

string pattern_to_string(int counter) {
  if (counter == -1) return "_";
  if (counter == 0) return "0";
  if (counter > 0) return "1";
  cout << "ERROR: invalid counter pattern: " << counter << "\n";
  return "_";
}

string build_no_action_mm2(const string& state, const string& next_state) {
  return state + " (_ _) -> " + next_state + " (0 0)";
}

string build_mm2_transition(const string& state, const vector<int>& patterns,
    const string& next_state, const vector<long long int>& changes) {
  if (patterns.size() != 2 || changes.size() != 2) {
    cout << "ERROR: patterns/changes count is not equal 2\n";
    return "undefined";
  }
  return state + " (" + pattern_to_string(patterns[0]) + " " +
      pattern_to_string(patterns[1]) + ") -> " + next_state + " (" +
      change_to_string(changes[0]) + " " + change_to_string(changes[1]) + ")";
}

string build_input_mm2_transition(const string& state, int in_pattern,
    const string& next, TransitionRaw::InputOperation op) {
  string result = state + " (_ _) " + (in_pattern == -1 ? "_" : to_string(in_pattern))
      + " ->* " + next + " (0 0) ";
  if (op == TransitionRaw::Load) result += "LOAD";
  if (op == TransitionRaw::Decrease) result += "-1";
  if (op == TransitionRaw::NothingIn) result += "NOOP";
  return result;
}

string build_output_mm2_transition(const string& state, const string& next,
    TransitionRaw::OutputOperation op) {
  string result = state + " (_ _) ->^ " + next + " (0 0) Output: ";
  if (op == TransitionRaw::Flush) result += "FLUSH";
  if (op == TransitionRaw::Increase) result += "1";
  if (op == TransitionRaw::NothingOut) result += "0";
  return result;
}

bool no_modify(const TransitionRaw& t) {
  for (int i = 0; i < 4; ++i)
    if (t.changes[i] != 0) return false;
  return true;
}

string build_modify_counters(const string& state, const TransitionRaw& t,
  vector<pair<string,string>>* out) {
  if (no_modify(t)) {
    PUSH(state, build_no_action_mm2(state, t.next_state));
    return t.next_state;
  }
  string current_state = state + "_modify";
  string base_name = current_state;
  PUSH(state, build_no_action_mm2(state, current_state));
  reset_name_generation();
  for (int i = 0; i < 4; ++i) { //  for each change in output counters
    if (t.changes[i] == 0) continue;
    if (t.changes[i] > 0) {
      string next_state = gen_next_name(base_name);
      string rec_state = current_state + "_recovery";
      // TODO: Bignums.
      long long int mult = pow(transform[i], t.changes[i]);
      if (mult < -1) cout << "Calc pow(" << transform[i] << ", " << t.changes[i] << ")\n";
      // Transitions to multiply counter by p^c (where p is prime and c
      // is the original counter change in mm4)
      out->push_back(make_pair(current_state, build_mm2_transition(
          current_state, {1,-1}, current_state, {-1,mult})));
      out->push_back(make_pair(current_state, build_mm2_transition(
          current_state, {0,-1}, rec_state, {0,0})));
      // Transitions to recover counter to original "left" side.
      out->push_back(make_pair(rec_state, build_mm2_transition(
          rec_state, {-1,1}, rec_state, {1,-1})));
      out->push_back(make_pair(rec_state, build_mm2_transition(
          rec_state, {-1,0}, next_state, {0,0})));
      current_state = next_state;
    } else { // equals -1
      string next_state = gen_next_name(base_name); // Final state after -1
      string circle_base = current_state + "_rest";
      string recovery = current_state + "_recovery";
      PUSH(current_state, build_no_action_mm2(current_state, circle_base + "0"));
      // Transitions to divide counter by p (transform[i]).
      PUSH(circle_base + "0",
          build_mm2_transition(circle_base + "0", {1,-1}, circle_base + "1", {-1,1}));
      PUSH(circle_base + "0", build_mm2_transition(circle_base + "0", {0,-1}, recovery, {0,0}));
      for (int j = 1; j < transform[i]; ++j) {
        string from = circle_base + to_string(j);
        PUSH(from, build_mm2_transition(from, {1,-1}, circle_base + to_string((j+1)%transform[i]), {-1,0}));
      }
      // Transitions to recover couter to left side.
      PUSH(recovery, build_mm2_transition(recovery, {-1,1}, recovery, {1,-1}));
      PUSH(recovery, build_mm2_transition(recovery, {-1,0}, next_state, {0,0}));
      current_state = next_state;
    }
  }
  out->push_back(make_pair(current_state, build_mm2_transition(
      current_state, {-1,-1}, t.next_state, {0,0})));
  return t.next_state;
}

void build_simple_track(const string& state, const TransitionRaw& t,
  vector<pair<string,string>>* out) {
  string current_state = state;
  if (t.type == TransitionRaw::Input) {
    string next_state = current_state + "_input";
    PUSH(current_state, build_input_mm2_transition(
        current_state, t.input_pattern, next_state, t.input_op));
    current_state = next_state;
  }
  if (t.type == TransitionRaw::Output) {
    string next_state = current_state + "_output";
    PUSH(current_state, build_output_mm2_transition(
        current_state, next_state, t.output_op));
    current_state = next_state;
  }
  string last_state = build_modify_counters(current_state, t, out);
  //cout << "last state " << last_state << "\n";
}

int my_log2(int mask) {
  for (int i = 0; i < 4; ++i)
    if ((1 << i) & mask)
      return i;
  cout << "ERROR: transition matches everything (!)\n";
  return -1;
}

bool check_only_counter(int indx, int match_mask) {
  return match_mask == (1 << indx);
}

bool find_alternatives(const vector<TransitionRaw>& ts,
    TransitionRaw* t0, TransitionRaw* t1) {
  TransitionRaw first = ts[0];
  if (__builtin_popcount(first.match_mask) != 1) {
    cout << "ERROR: more than one counter to match, currently unimplemented.\n";
    return false;
  }
  int indx = my_log2(first.match_mask);
  for (const auto& t : ts) {
    if (!check_only_counter(indx, t.match_mask)) {
      cout << "ERROR: only counter " << indx << " failed\n";
      return false;
    }
  }
  int cs = 0; //checksum
  for (int i = ts.size()-1; i >= 0; --i) {
    if (ts[i].patterns[indx] == 0) {
      *(t0) = ts[i]; cs |= 1;
    }
    else if (ts[i].patterns[indx] == 1) {
      *(t1) = ts[i]; cs |= 2;
    }
    else cout << "Warning: transition with weird pattern " << ts[i].patterns[indx] << "\n";
  }
  return cs == 3;
}

// Inserts tuples (state, full transition to be written in output file).
void translate_one_state(const string& state,
    const vector<TransitionRaw>& transitions,
    vector<pair<string,string>>* out) {
  // Assumes all input/output transitions will fall into this case.
  if (matches_all(transitions[0]))
    build_simple_track(state, transitions[0], out);
  else {
    TransitionRaw t0, t1;
    if (!find_alternatives(transitions, &t0, &t1))
      return;

    if (t0.type != TransitionRaw::Regular || t1.type != TransitionRaw::Regular) {
      cout << "ERROR: Transition matches all but is of type input/output\n";
      return;
    }

    int indx = my_log2(t0.match_mask);
    if (my_log2(t1.match_mask) != indx) {
      cout << "ERROR: Match masks are different!\n";
      return;
    }
    const string check_base = state + "_check";
    PUSH(state,build_no_action_mm2(state, check_base + "0"));
    string recovery = check_base + "_recovery",
           recovered = check_base + "_recovered";
    PUSH(check_base + "0",
        build_mm2_transition(check_base + "0", {0,-1}, recovery, {0,0}));
    PUSH(recovery, build_mm2_transition(recovery, {-1,1}, recovery, {1,-1}));
    PUSH(recovery, build_mm2_transition(recovery, {-1,0}, recovered, {0,0}));
    build_modify_counters(recovered, t1, out); // finished path, counter == 1

    recovery = check_base + "_match_zero_recovery";
    recovered = check_base + "_match_zero_recovered";

    // Build circle of divisibility by transform[indx].
    for (int i = 0; i < transform[indx]; ++i) {
      string from = check_base + to_string(i);
      PUSH(from, build_mm2_transition(from, {1,-1},
          check_base + to_string((i+1)%transform[indx]), {-1,1}));
      if (i > 0)
        PUSH(from, build_mm2_transition(from, {0,-1}, recovery, {0,0}));
    }

    // Build general recovery states for matching counter state equal 0
    // (all rests from 1 to transform[i]-1 will end up in here).
    PUSH(recovery, build_mm2_transition(recovery, {-1,1}, recovery, {1,-1}));
    PUSH(recovery, build_mm2_transition(recovery, {-1,0}, recovered, {0,0}));
    build_modify_counters(recovered, t0, out); // finished path, counter == 0
  }
}

void Translation::translate(const string& input, const string& output) {
  TransitionMap map;
  map.setMultifile(input);
  output_base = output;
  std::hash<string> hash_fn;
  //cout << "Translation output base: " << output << "\n";
  /*
    Available functions within TransitionMap (field map):
      * FilesCount() - how many shard files there are
      * AddTransitionsWholeFile(file_no) - adds all transitions from given shard
      * ClearMap() - deletes all transitions from the map (init_state is untouched)
      * GetMap() - gets const reference to map of transitions
  */
  for (int i = 0; i < map.FilesCount(); ++i) {
    map.ClearMap();
    map.AddTransitionsWholeFile(i);
    const auto inner_map = map.GetMap();
    for (const auto& state : inner_map) {
      vector<pair<string,string>> outputs;
      translate_one_state(state.first, state.second, &outputs);
      for (const auto& s : outputs) {
        const string filename = output +
            std::to_string(hash_fn(s.first) % mgr::DEFAULT_HASHTABLE_SIZE);
        std::ofstream file (filename, std::ofstream::app);
        file << s.second << mgr::STATEMENT_SEPARATOR;
        file.close();
      }
    }
  }
}
