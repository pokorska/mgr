#ifndef NAMES_MANIP_H_
#define NAMES_MANIP_H_

#include <string>
#include <vector>
#include <unordered_set>

#include "../shared_files/constants.h"

enum Stack { Left, Right };

std::string c_to_string(int counter);
std::string build_name(const std::string& base, int left, int right = -1);
std::string gen_next_name(const std::string& base);
std::string get_curr_name(const std::string& base);

std::string create_no_action_MM4(const std::string& state, const std::string& target);
std::string create_MM4(const std::string& state, int c1_in, int c2_in, Stack stack,
    const std::string& target, int c1_out, int c2_out);
std::string create_output_MM4(const std::string& state, int symbol,
    const std::string& target);
std::string create_input_MM4(const std::string& state, int in,
    const std::string& target, int out);
std::string create_input_MM4(const std::string& state, int c[4], int in,
    const std::string& target, int c_out[4], int in_oper);

void insert_interpreted_chars(char pattern, std::unordered_set<int>* chars,
    int alph_size = mgr::DEFAULT_ALPHABET_SIZE);
std::vector<int> get_all_chars(char pattern, int alph_size = mgr::DEFAULT_ALPHABET_SIZE);

#endif // NAMES_MANIP_H_
