#ifndef TRANSLATION_H_
#define TRANSLATION_H_

#include <string>
#include <vector>

#include "ast.h"
#include "../shared_files/constants.h"
#include "names_manip.h"

void build_pushing_symbol(const std::string& base, int symbol, Stack stack,
    std::vector<std::string>* dest, int alph_size = mgr::DEFAULT_ALPHABET_SIZE);

void build_output_items(const std::string& base, int left, int right,
    const TransitionRaw& t, std::vector<std::string>* dest, int alph_size);

void build_closing_transition(const std::string& base, const std::string& target,
    std::vector<std::string>* dest);

void build_pushing_items(const std::string& start_state, int left, int right, int input,
    const TransitionRaw& t, std::vector<std::string>* dest,
    int alph_size = mgr::DEFAULT_ALPHABET_SIZE);

bool simple_track_possible(const TransitionRaw& t);

void build_simple_track(const std::string& state, const TransitionRaw& t, int left, int right,
    std::vector<std::string>* dest, int alph_size = mgr::DEFAULT_ALPHABET_SIZE);

// Creates recognition of stack items for given state.
void build_items_recognition(const std::string& state,
    const std::vector<TransitionRaw>& transitions, std::vector<std::string>* dest);

#endif // TRANSLATION_H_
