#ifndef TM_AST_H_
#define TM_AST_H_

#include <map>
#include <string>
#include <vector>
#include <iostream>

//#include "mm4-driver.hh"
#include "../shared_files/constants.h"
#include "translate.h"

struct TransitionRaw {
  enum Type { Regular, Input, Output };
  enum InputOperation { Load, Decrease, NothingIn };
  enum OutputOperation { Flush, Increase, NothingOut };
  Type type;
  bool patterns[4];
  int input_pattern;
  int changes[4];
  int output_change = 0;
  int match_mask = 15;
  InputOperation input_op;
  OutputOperation output_op;
  std::string curr_state, next_state;

  void init_mask(int p[4]) {
    //std::cout << "Init mask based on " << p[0] << " " << p[1] << " " << p[2] << " " << p[3] << "\n";
    match_mask = 0;
    for (int i = 0; i < 4; ++i)
      if (p[i] != -1)
        match_mask |= (1 << i);
  }

  TransitionRaw() : curr_state("Invalid"), patterns {-1, -1, -1, -1},
      input_pattern(-1), type(Regular), next_state("END"), input_op(NothingIn),
      output_op(NothingOut), changes {0, 0, 0, 0} { }

  // Input manipulating transition
  TransitionRaw(const std::string& curr_state, int p[4], int input_pattern,
      Type type, std::string next_state, int c[4], InputOperation input_op = NothingIn)
    : curr_state(curr_state), input_pattern(input_pattern), type(type),
      next_state(next_state), input_op(input_op), output_op(NothingOut),
      patterns { p[0] != 0, p[1] != 0, p[2] != 0, p[3] != 0 },
      changes { c[0], c[1], c[2], c[3] } {
    //std::cout << "pattern: " << p[0] << p[1] << p[2] << p[3] << "\n";
    //std::cout << "changes: " << c[0] << c[1] << c[2] << c[3] << "\n";
    init_mask(p);
    //std::cout << "Transition " << curr_state << " ->* " << next_state
    //          << " has mask: " << match_mask << "\n";
  }

  // Regular transition
  TransitionRaw(const std::string& curr_state, int p[4], Type type,
      std::string next_state, int c[4])
    : curr_state(curr_state), type(type), next_state(next_state),
      input_pattern(-1), input_op(NothingIn), output_op(NothingOut),
      patterns { p[0] != 0, p[1] != 0, p[2] != 0, p[3] != 0 },
      changes { c[0], c[1], c[2], c[3] } {
    //std::cout << "pattern: " << p[0] << p[1] << p[2] << p[3] << "\n";
    //std::cout << "changes: " << c[0] << c[1] << c[2] << c[3] << "\n";
    init_mask(p);
  }

  // Output manipulating transition
  TransitionRaw(const std::string& curr_state, int p[4], Type type,
      std::string next_state, int c[4], OutputOperation output_op,
      int output_change = 0)
    : curr_state(curr_state), type(type), next_state(next_state),
      input_pattern(-1), input_op(NothingIn), output_op(output_op),
      output_change(output_change),
      patterns { p[0] != 0, p[1] != 0, p[2] != 0, p[3] != 0 },
      changes { c[0], c[1], c[2], c[3] } {
    //std::cout << "pattern: " << p[0] << p[1] << p[2] << p[3] << "\n";
    //std::cout << "changes: " << c[0] << c[1] << c[2] << c[3] << "\n";
    init_mask(p);
  }
};

std::ostream& operator<<(std::ostream& out, const TransitionRaw& t);

class TransitionMap;

class Statement {
 public:
  virtual void convert_to_transition_map(TransitionMap* tmap) const = 0;
};

class TransitionMap {
 private:
  std::map<std::string, std::vector<TransitionRaw>> transitions;
  std::string init_state;
  bool multifile_mode;
  std::string multifile_base;
  int files_added = 0;

 public:
  TransitionMap (Statement* stm_sequence) : multifile_mode(false) {
    stm_sequence->convert_to_transition_map(this);
    if (init_state.empty()) {
      std::cout << "Warning: No init state defined.\n";
    }
  }

  // WARNING: for internal use ONLY.
  TransitionMap() : multifile_mode(true) {
    init_state = "undefined";
  }

  void Extend(Statement* tmp_sequence) {
    tmp_sequence->convert_to_transition_map(this);
  }

  // Forwarded to .cc
  void AddTransitions(const std::string& state);
  int FilesCount() const;
  void AddTransitionsWholeFile(int file_no);

  const std::map<std::string, std::vector<TransitionRaw>>& GetMap() const {
    return transitions;
  }

  void setMultifile(const std::string& base) {
    multifile_mode = true;
    multifile_base = base;
  }

  void ClearMap() {
    transitions.clear();
  }

  void AddTransition(TransitionRaw t) {
    transitions[t.curr_state].push_back(t);
  }
  void AddInitState(std::string name) {
    if (!init_state.empty())
      std::cout << "Warning: Overwriting init state " << init_state
                << " with new value: " << name << "\n";
    init_state = name;
  }

  bool MatchPattern(const bool counters[4], const bool expected[4], int match_mask) {
    int res = 0;
    for (int i = 0; i < 4; ++i)
      res |= ((counters[i] == expected[i]) << i);
    return match_mask == (match_mask & res);
  }

  TransitionRaw FindTransition(const std::string& state, long long counters[4], long long input_pattern) {
    if (transitions.count(state) == 0 && multifile_mode) {
      AddTransitions(state);
    }

    bool counters_state[] = { counters[0] > 0, counters[1] > 0, counters[2] > 0, counters[3] > 0 };
    bool input_counter_state = input_pattern > 0;
    for (const auto& t : transitions[state]) {
      if (t.input_pattern == -1 || input_counter_state == t.input_pattern)
        if (MatchPattern(counters_state, t.patterns, t.match_mask))
          return t;
    }
    // Default transition returned when no match is found.
    int default_pattern[] = { -1, -1, -1, -1 }, default_changes[] = { 0, 0, 0, 0 };
    return TransitionRaw(state, default_pattern, TransitionRaw::Regular, mgr::END_STATE, default_changes);
  }

  void evaluate(bool verbose) {
    // REMEMBER: match_mask specifies what counters should be taken into account.
    long long counters[4] = { 0LL, 0LL, 0LL, 0LL };
    long long input_counter = 0LL, output_counter = 0LL;
    std::string curr_state = init_state;
    while (curr_state != mgr::END_STATE) {
      if (verbose)
        std::cout << "-------------------------------\n"
                  << "Current state: " << curr_state << "\n";
      TransitionRaw transition = FindTransition(curr_state, counters, input_counter);
      if (verbose)
        std::cout << "Found transition " << transition.curr_state
                  << " -> " << transition.next_state << "\n";
      if (transition.input_op == TransitionRaw::Load) {
        char c; std::cin >> c;
        input_counter = c;
      }
      else if (transition.input_op == TransitionRaw::Decrease) input_counter--;
      if (transition.output_op == TransitionRaw::Flush) {
        // TODO: handle overflow - when output counter contains
        // too large number to become char then maybe it should
        // print multiple chars?
        std::cout << (char)output_counter;
        output_counter = 0LL;
      }
      else if (transition.output_op == TransitionRaw::Increase)
        output_counter += transition.output_change;
      for (int i = 0; i < 4; ++i)
        counters[i] += transition.changes[i];

      if (transition.next_state == mgr::END_STATE) {
        std::cout << "Very last state: " << curr_state << "\n";
      }
      curr_state = transition.next_state;

      // Sanity checks
      if (input_counter < 0)
        std::cout << "ERROR: input counter negative!\n";
      if (output_counter < 0)
        std::cout << "ERROR: output counter negative!\n";
      for (int i = 0; i < 4; ++i)
        if (counters[i] < 0)
          std::cout << "ERROR: counter " << i << " negative!\n";
      if (verbose) {
        std::cout << "- - - - - - - - - - - - - - - -\n"
                  << "Counters state:\n"
                  << input_counter << " (input)\n"
                  << output_counter << " (output)\n"
                  << counters[0] << " (1)\n"
                  << counters[1] << " (2)\n"
                  << counters[2] << " (3)\n"
                  << counters[3] << " (4)\n"
                  << "-------------------------------\n";
      }
    }
  //std::cout << "TOTAL FILES ADDED: " << files_added << "\n";
  }
/*
  std::string transitionTypeToString(TransitionRaw::Type t) {
    return t == TransitionRaw::Input ? "->*" : t == TransitionRaw::Output ? "->^" : "->";
  }
*/
  void translate(const std::string& output_base) {
    Translation translation;
    translation.translate(multifile_base, output_base);
  }

  void print_status() const {
    printf("init state: %s\nTransitions:\n", init_state.c_str());
    for (const auto& t : transitions) {
      printf("State %s has %lu transitions\n", t.first.c_str(), t.second.size());
    }
  }
};

class InitState : public Statement {
 private:
  std::string state_name;
 public:
  InitState(std::string state_name) : state_name(state_name) { }
  virtual void convert_to_transition_map(TransitionMap* tmap) const {
    tmap->AddInitState(state_name);
  }
};

class Transition : public Statement {
 private:
  TransitionRaw t;
 public:
  Transition(TransitionRaw t) : t(t) { }
  virtual void convert_to_transition_map(TransitionMap* tmap) const {
    tmap->AddTransition(t);
  }
};

class Sequence : public Statement {
 private:
  Statement *stm1, *stm2;
 public:
  Sequence(Statement* stm1, Statement* stm2) : stm1(stm1), stm2(stm2) { }
  virtual void convert_to_transition_map(TransitionMap* tmap) const {
    stm1->convert_to_transition_map(tmap);
    stm2->convert_to_transition_map(tmap);
  }
  ~Sequence() {
    delete stm1;
    delete stm2;
  }
};

#endif
