#ifndef TRANSLATION_4CM_H_
#define TRANSLATION_4CM_H_

#include <string>

class Translation {
 private:
  std::string output_base = "output/base";
  //TransitionMap map;

 public:
  Translation() { }
  void translate(const std::string& input, const std::string& output);
};

#endif // TRANSLATION_4CM_H_
