#ifndef BF_AST_H_
#define BF_AST_H_

#include <map>
#include <string>
#include <vector>

std::string strip_label(const std::string& label);

const int BLANK = 0;
const int MAX_TAPE_LENGTH = 1000;
const std::string STATE_NAME_BASE = "state";
const std::string STATEMENT_SEPARATOR = "\n";
const char ALL_CHARS = '*'; // TODO: change it to non-ASCII character.
const char NO_CHAR = '#'; // TODO: change to non-ASCII
const char NON_ZERO = '&'; // TODO: change to non-ASCII
const char NEXT_CHAR = '>'; // TODO: change to non-ASCII
const char PREV_CHAR = '<'; // TODO: change to non-ASCII
const char ZERO = '0'; // TODO: CHANGE!

class SequenceGenerator {
 private:
  int next_value;
  SequenceGenerator() : next_value(1) { }
 public:
  static SequenceGenerator* getInstance() {
    static SequenceGenerator* instance = nullptr;

    if (instance == nullptr) {
      instance = new SequenceGenerator();
    }

    return instance;
  }
  void reset() {
    next_value = 1;
  }
  int getNext() {
    return next_value++;
  }
};

enum HeadMove { Left, Right, None };
inline char to_char(HeadMove hm) {
  return hm == Left ? 'L' : hm == Right ? 'R' : '-';
}

class Statement {
 private:
  std::string state_name;

 public:
  void check_tape(std::vector<int>* tape, int* pos) const {
    if (tape == nullptr) {
      std::cout <<  "Tape does not exist!\n";
      throw "Tape does not exist!";
    }
    if (*pos < 0) {
      std::cout << "Pointer outside range\n";
      throw "Pointer outside range";
    }
    if (*pos >= MAX_TAPE_LENGTH) {
      std::cout << "Pointer too far on the tape: " << *pos << " MAX: " << MAX_TAPE_LENGTH << "\n";
      throw "Pointer too far on the tape";
    }
    while(*pos >= tape->size()) {
      tape->push_back(BLANK);
      //std::cout << "push. size: " << tape->size() << " pos: " << *pos << "\n";
    }
  }
  virtual void evaluate(std::vector<int>* tape, int* pos) const = 0;
  virtual std::string to_string() = 0;
  /* Translation to Turing Machine */
  virtual std::string getStateName() {
    if (state_name.empty())
      state_name = STATE_NAME_BASE + std::to_string(SequenceGenerator::getInstance()->getNext());
    return state_name;
  }
  virtual std::string translate() {
    SequenceGenerator::getInstance()->reset();
    return "START: " + getStateName() + STATEMENT_SEPARATOR + translate(nullptr);
  }
  virtual std::string translate(Statement* next) = 0;
};

inline std::string nullSafeGetName(Statement* s) {
  return s == nullptr ? "END" : s->getStateName();
}

class Empty : public Statement {
 public:
  virtual void evaluate(std::vector<int>* tape, int* pos) const { }
  virtual std::string to_string() { return "<empty>"; }
  virtual std::string translate(Statement* next) {
    return getStateName() + " " + ALL_CHARS + " -> " +
           nullSafeGetName(next) + " " + to_char(None) + " " + NO_CHAR +
           STATEMENT_SEPARATOR;
  }
};

class Sequence : public Statement {
 private:
  Statement *stm1, *stm2;
 public:
  Sequence(Statement* stm1, Statement* stm2) : stm1(stm1), stm2(stm2) { }
  virtual void evaluate(std::vector<int>* tape, int* pos) const {
    if (stm1 != nullptr) stm1->evaluate(tape, pos);
    if (stm2 != nullptr) stm2->evaluate(tape, pos);
  }
  virtual std::string to_string() {
    std::string stm1_str = stm1 == nullptr ? "<empty>" : stm1->to_string(),
        stm2_str = stm2 == nullptr ? "<empty>" : stm2->to_string();
    return "Sequence(" + stm1_str + ", " + stm2_str + ")";
  }

  virtual std::string getStateName() {
    return stm1->getStateName();
  }
  
  virtual std::string translate(Statement* next) {
    std::string result = stm1->translate(stm2);
    result += stm2->translate(next);
    return result;
  }
};

class MoveRight : public Statement {
 public:
  virtual void evaluate(std::vector<int>* tape, int* pos) const {
    //std::cout << "Move right\n";
    (*pos)++;
    check_tape(tape, pos);
    //std::cout << "> Now pos: " << pos << " " << *pos << "\n";
  }
  virtual std::string to_string() {
    return "MoveRight";
  }

  virtual std::string translate(Statement* next) {
    return getStateName() + " " + ALL_CHARS + " -> " +
           nullSafeGetName(next) + " " + to_char(Right) + " " + NO_CHAR +
           STATEMENT_SEPARATOR;
  }
};

class MoveLeft : public Statement {
 public:
  virtual void evaluate(std::vector<int>* tape, int* pos) const {
    //std::cout << "Move left\n";
    (*pos)--;
    check_tape(tape, pos);
    //std::cout << "< Now pos: " << pos << " " << *pos << "\n";
  }
  virtual std::string to_string() {
    return "MoveRight";
  }
  
  virtual std::string translate(Statement* next) {
    return getStateName() + " " + ALL_CHARS + " -> " +
           nullSafeGetName(next) + " " + to_char(Left) + " " + NO_CHAR +
           STATEMENT_SEPARATOR;
  }
};

class Increase : public Statement {
 public:
  virtual void evaluate(std::vector<int>* tape, int* pos) const {
    check_tape(tape, pos);
    (*tape)[*pos]++;
  }
  virtual std::string to_string() {
    return "Increase";
  }
  
  virtual std::string translate(Statement* next) {
    return getStateName() + " " + ALL_CHARS + " -> " +
           nullSafeGetName(next) + " " + to_char(None) + " " + NEXT_CHAR +
           STATEMENT_SEPARATOR;
  }
};

class Decrease : public Statement {
 public:
  virtual void evaluate(std::vector<int>* tape, int* pos) const {
    check_tape(tape, pos);
    (*tape)[*pos]--;
  }
  virtual std::string to_string() {
    return "Decrease";
  }
  
  virtual std::string translate(Statement* next) {
    return getStateName() + " " + ALL_CHARS + " -> " +
           nullSafeGetName(next) + " " + to_char(None) + " " + PREV_CHAR +
           STATEMENT_SEPARATOR;
  }
};

class Write : public Statement {
 public:
  virtual void evaluate(std::vector<int>* tape, int* pos) const {
    check_tape(tape, pos);
    std::cout << (char)(*tape)[*pos];
  }
  virtual std::string to_string() {
    return "Write";
  }

  virtual std::string translate(Statement* next) {
    return getStateName() + " " + ALL_CHARS + " ->^ " +
           nullSafeGetName(next) + " " + to_char(None) + " " + NO_CHAR +
           STATEMENT_SEPARATOR;
  }
};

class Read : public Statement {
 public:
  virtual void evaluate(std::vector<int>* tape, int* pos) const {
    check_tape(tape, pos);
    char x;
    std::cin >> x;
    (*tape)[*pos] = x;
  }
  virtual std::string to_string() {
    return "Read";
  }
  
  virtual std::string translate(Statement* next) {
    return getStateName() + " " + ALL_CHARS + " ->* " +
           nullSafeGetName(next) + " " + to_char(None) + " " + NO_CHAR +
           STATEMENT_SEPARATOR;
  }
};

class Loop : public Statement {
 private:
  Statement* inner;
 public:
  Loop(Statement* inner) : inner(inner) { }
  virtual void evaluate(std::vector<int>* tape, int* pos) const {
    check_tape(tape, pos);
    while ((*tape)[*pos] != 0) inner->evaluate(tape, pos);
  }
  virtual std::string to_string() {
    return "Loop { " + inner->to_string() + " }";
  }

  virtual std::string translate(Statement* next) {
    std::string result = "";
    result += getStateName() + " " + NON_ZERO + " -> " +
              nullSafeGetName(inner) + " " + to_char(None) + " " + NO_CHAR +
              STATEMENT_SEPARATOR;
    result += getStateName() + " " + ZERO + " -> " +
              nullSafeGetName(next) + " " + to_char(None) + " " + NO_CHAR +
              STATEMENT_SEPARATOR;
    return result + inner->translate(this);
  }
};

#endif
