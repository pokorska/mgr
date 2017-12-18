#ifndef AST_H_
#define AST_H_

#include <map>
#include <string>

std::string strip_label(const std::string& label);

class Statement {
 public:
  virtual void evaluate(
      std::map<int, int>* stacks,
      const std::map<std::string, Statement*>& labels) const = 0;
  virtual std::string to_string() = 0;
};

class Empty : public Statement {
 public:
  Empty() { }
  virtual void evaluate(
      std::map<int, int>* stacks,
      const std::map<std::string, Statement*>& labels) const { }
  virtual std::string to_string() { return "<empty>"; }
};

class Sequence : public Statement {
 private:
  Statement *stm1, *stm2;
 public:
  Sequence(Statement* stm1, Statement* stm2) : stm1(stm1), stm2(stm2) { }
  virtual void evaluate(
      std::map<int, int>* stacks,
      const std::map<std::string, Statement*>& labels) const {
    if (stm1 != nullptr) stm1->evaluate(stacks, labels);
    if (stm2 != nullptr) stm2->evaluate(stacks, labels);
  }
  virtual std::string to_string() {
    std::string stm1_str = stm1 == nullptr ? "<empty>" : stm1->to_string(),
        stm2_str = stm2 == nullptr ? "<empty>" : stm2->to_string();
    return "Sequence(" + stm1_str + ", " + stm2_str + ")";
  }
};

class Increase : public Statement {
 private:
  int stack_num;
 public:
  Increase(int stack_num) : stack_num(stack_num) { }
  virtual void evaluate(
      std::map<int, int>* stacks,
      const std::map<std::string, Statement*>& labels) const {
    (*stacks)[stack_num]++;
  }
  virtual std::string to_string() {
    return "Increase(" + std::to_string(stack_num) + ")";
  }
};

class Decrease : public Statement {
 private:
  int stack_num;
 public:
  Decrease(int stack_num) : stack_num(stack_num) { }
  virtual void evaluate(
      std::map<int, int>* stacks,
      const std::map<std::string, Statement*>& labels) const {
    (*stacks)[stack_num]--;
  }
  virtual std::string to_string() {
    return "Decrease(" + std::to_string(stack_num) + ")";
  }
};

// It should be redesigned - it's not allowed to print whole stack.
class Write : public Statement {
 private:
  int stack_num;
 public:
  Write(int stack_num) : stack_num(stack_num) { }
  virtual void evaluate(
      std::map<int, int>* stacks,
      const std::map<std::string, Statement*>& labels) const {
    std::cout << (*stacks)[stack_num] << "\n";
  }
  virtual std::string to_string() {
    return "Write(" + std::to_string(stack_num) + ")";
  }
};

class Read : public Statement {
 private:
  int stack_num;
 public:
  Read(int stack_num) : stack_num(stack_num) { }
  virtual void evaluate(
      std::map<int, int>* stacks,
      const std::map<std::string, Statement*>& labels) const {
    int input;
    std::cin >> input;
    (*stacks)[stack_num] = input;
  }
  virtual std::string to_string() {
    return "Read(" + std::to_string(stack_num) + ")";
  }
};

class JumpZero : public Statement {
 private:
  int stack_num;
  std::string label;
  Statement* normal_flow;
 public:
  JumpZero(int stack_num, std::string label, Statement* normal_flow)
      : stack_num(stack_num), label(label), normal_flow(normal_flow) { }
  virtual void evaluate(
      std::map<int, int>* stacks,
      const std::map<std::string, Statement*>& labels) const {
    if (labels.find(label) == labels.end()) std::cout << "FAIL: " << label << "\n";
    if ((*stacks)[stack_num] > 0) normal_flow->evaluate(stacks, labels);
    else labels.at(label)->evaluate(stacks, labels);
  }
  
  virtual std::string to_string() {
    return "JumpZero(" + std::to_string(stack_num) + ", " + label
           + ", " + normal_flow->to_string() + ")";
  }
};

#endif
