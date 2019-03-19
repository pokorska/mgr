#include <fstream>
#include <cstdio>
#include "ast.h"
#include "mm4-driver.hh"

using std::string;
using std::to_string;
using std::cout;
using std::ifstream;
using std::ofstream;
using std::hash;

const string TMP_FILE = "temporary.mm4";

inline bool file_exists (const string& name) {
  ifstream f(name.c_str());
  return f.good();
}

string getState(const string& transition) {
  return transition.substr(0, transition.find(' '));
}

void filterTransitions(const string& state, const string& filename) {
  ifstream file(filename);
  ofstream out(TMP_FILE, ofstream::out);
  string line;
  while (getline(file, line)) { //  mgr::STATEMENT_SEPARATOR should be used...
    if (getState(line) == state)
      out << line << mgr::STATEMENT_SEPARATOR;
  }
}

void TransitionMap::AddTransitions(const string& state) {
  int prev_size = transitions.size();
  hash<string> hash_fn;
  string filename =
      multifile_base + "_" + std::to_string(hash_fn(state) % mgr::DEFAULT_HASHTABLE_SIZE);
  if (!file_exists(filename)) return;
  filterTransitions(state, filename);
  mm4_driver driver;
  Statement* add = driver.parse_to_statement(TMP_FILE);
  std::remove(TMP_FILE.c_str());
  if (add != nullptr) Extend(add);
  //if (transitions.size() / 1000 > prev_size / 1000)
    //cout << "Current map size: " << transitions.size() << "\n";
}

int TransitionMap::FilesCount() const {
  return mgr::DEFAULT_HASHTABLE_SIZE;
}

void TransitionMap::AddTransitionsWholeFile(int file_no) {
  if (file_no < 0 || file_no >= FilesCount()) {
    std::cerr << "Invalid file number to read: " << file_no << "\n";
    return;
  }
  string filename = multifile_base + "_" + std::to_string(file_no);
  if (!file_exists(filename)) {
    std::cerr << "File " << filename << " does not exist. Nothing to add.\n";
    return;
  }
  mm4_driver driver;
  Statement* add = driver.parse_to_statement(filename);
  if (add != nullptr) Extend(add);
}
