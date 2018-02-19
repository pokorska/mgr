#ifndef BF_AST_H_
#define BF_AST_H_

#include <map>
#include <string>
#include <vector>

std::string strip_label(const std::string& label);

const int BLANK = 0;
const int MAX_TAPE_LENGTH = 1000;

class Statement {
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
};

class Empty : public Statement {
 public:
  virtual void evaluate(std::vector<int>* tape, int* pos) const { }
  virtual std::string to_string() { return "<empty>"; }
};

class Sequence : public Statement {
 private:
  Statement *stm1, *stm2;
 public:
  Sequence(Statement* stm1, Statement* stm2) : stm1(stm1), stm2(stm2) { }
  virtual void evaluate(std::vector<int>* tape, int* pos) const {
    //std::cout << "Seq eval... " << pos << "\n";
    if (stm1 != nullptr) stm1->evaluate(tape, pos);
    if (stm2 != nullptr) stm2->evaluate(tape, pos);
  }
  virtual std::string to_string() {
    std::string stm1_str = stm1 == nullptr ? "<empty>" : stm1->to_string(),
        stm2_str = stm2 == nullptr ? "<empty>" : stm2->to_string();
    return "Sequence(" + stm1_str + ", " + stm2_str + ")";
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
};

#endif
