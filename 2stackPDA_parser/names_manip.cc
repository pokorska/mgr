#include "names_manip.h"

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

using std::string;
using std::to_string;
using std::cout;
using std::unordered_set;
using std::vector;

string c_to_string(int counter) {
  if (counter == -1) return "_";
  if (counter == 0) return "0";
  if (counter == 1) return "1";
  cout << "ERROR: Unknown counter state expected: " << counter << "\n";
  return "_";
}

string build_name(const string& base, int left, int right) {
  return base + "_L" + to_string(left) + (right == -1 ? "" : "_R" + to_string(right));
}

string gen_next_name(const string& base) {
  mgr::SequenceGenerator* seq_gen = mgr::SequenceGenerator::getInstance();
  return base + to_string(seq_gen->getNext());
}

string get_curr_name(const string& base) {
  mgr::SequenceGenerator* seq_gen = mgr::SequenceGenerator::getInstance();
  return base + to_string(seq_gen->getCurrent());
}

string create_no_action_MM4(const string& state, const string& target) {
  return state + " (_ _ _ _) -> " + target + " (0 0 0 0)";
}

string create_MM4(const string& state, int c1_in, int c2_in, Stack stack,
    const string& target, int c1_out, int c2_out) {
  //cout << "> STATE: " << state << " TARGET: " << target << "\n";
  const string counters_in = c_to_string(c1_in) + " " + c_to_string(c2_in);
  const string counters_out = to_string(c1_out) + " " + to_string(c2_out);
  return state + " (" + (stack == Right ? "_ _ " + counters_in : counters_in + " _ _") + ") -> "
      + target + " (" + (stack == Right ? "0 0 " + counters_out : counters_out + " 0 0") + ")";
}

string create_output_MM4(const string& state, int symbol, const string& target) {
  return state + " (_ _ _ _) ->^ " + target + " (0 0 0 0) Output: "
      + (symbol == -1 ? "FLUSH" : to_string(symbol));
}

string create_input_MM4(const string& state, int in,
    const string& target, int out) {
  string input_operation = "-1";
  if (out == -2) input_operation = "LOAD";
  if (out == 0) input_operation = "NOOP";
  return state + " (_ _ _ _) " + (in == -1 ? "_" : to_string(in)) + " ->* " + target
      + " (0 0 0 0) " + input_operation;
}

string create_input_MM4(const string& state, int c[4], int in,
    const string& target, int c_out[4], int in_oper) {
  string input_operation = "-1";
  if (in_oper == -2) input_operation = "LOAD";
  if (in_oper == 0) input_operation = "NOOP";
  char in_str = in == -1 ? '_' : in == 0 ? '0' : '1';
  char c_str[4];
  string c_out_str[4];
  for (int i = 0; i < 4; ++i) {
    c_str[i] = c[i] == -1 ? '_' : c[i] == 0 ? '0' : '1';
    c_out_str[i] = to_string(c_out[i]);
  }
  return state + " (" + c_str[0] + " " + c_str[1] + " " + c_str[2] + " " + c_str[3] + ")"
      + " " + in_str + " ->* " + target + " (" + c_out_str[0] + " " + c_out_str[1]
      + " " + c_out_str[2] + " " + c_out_str[3] + ") " + input_operation;
}

void insert_interpreted_chars(char pattern, unordered_set<int>* chars, int alph_size) {
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

vector<int> get_all_chars(char pattern, int alph_size) {
  vector<int> result;
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
