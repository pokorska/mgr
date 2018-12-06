#ifndef TRANSLATION_H_
#define TRANSLATION_H_

#include <string>
#include <vector>

#include "ast.h"
#include "../shared_files/constants.h"
#include "names_manip.h"

// Creates transitions corresponding to pushing the given symbol to the top of the given stack.
void build_pushing_symbol(const std::string& base, int symbol, Stack stack,
    std::vector<std::string>* dest, int alph_size = mgr::DEFAULT_ALPHABET_SIZE);

// Creates transitions that print the output symbol (defined in PDA transition). Created MM4
// transitions are pushing the output symbol to the output counter and then flush it.
void build_output_items(const std::string& base, int left, int right,
    const TransitionRaw& t, std::vector<std::string>* dest, int alph_size);

// Creates single transition from recently generated name (given base string
// with number at the end) to the target - it is meant to be the very last
// MM4 transition ending in target state from PDA transition.
void build_closing_transition(const std::string& base, const std::string& target,
    std::vector<std::string>* dest);

// Creates transitions that push items to the top of the given stack.
void build_pushing_one_stack(const std::string& base, int left, int right, int input,
    Stack stack, const TransitionRaw& t, std::vector<std::string>* dest, int alph_size);

// Creates transitions that push items first to the top of left stack
// and then to the right stack.
void build_pushing_items(const std::string& start_state, int left, int right, int input,
    const TransitionRaw& t, std::vector<std::string>* dest,
    int alph_size = mgr::DEFAULT_ALPHABET_SIZE, bool ignore_right_stack = false);

// Checks whether it is possible to push input char without recognizing onto left stack.
// It requires the transition to have INPUT_CHAR as left stack items.
bool simple_track_possible(const TransitionRaw& t);

// Checks whether it is possible not to recognize right symbol on the stack. If it's
// removed from the stack and later pushed back (and it's not pushed anywhere else - either
// on left stack or on output).
bool simplify_right_possible(const TransitionRaw& t);

// Creates transitions that push input char onto the left stack.
void build_simple_track(const std::string& state, const TransitionRaw& t, int left, int right,
    std::vector<std::string>* dest, int alph_size = mgr::DEFAULT_ALPHABET_SIZE);

// Creates recognition of stack items for given state.
void build_items_recognition(const std::string& state,
    const std::vector<TransitionRaw>& transitions, std::vector<std::string>* dest);

#endif // TRANSLATION_H_
