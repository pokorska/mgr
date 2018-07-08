#ifndef CONSTANTS_H_
#define CONSTANTS_H_

namespace mgr {

const int BLANK = 0;
const int MAX_TAPE_LENGTH = 1000;
const std::string STATEMENT_SEPARATOR = "\n";
const char ALL_CHARS = '*'; // TODO: change it to non-ASCII character.
const char NO_CHAR = '#'; // TODO: change to non-ASCII
const char NON_ZERO = '&'; // TODO: change to non-ASCII
const char NEXT_CHAR = '>'; // TODO: change to non-ASCII
const char PREV_CHAR = '<'; // TODO: change to non-ASCII
const char ZERO = '0'; // TODO: CHANGE!
const char EMPTY_STACK_CHAR = '$';
const std::string END_STATE = "END";

const int INPUT_SYMBOL = -1;
const int ORIG_LEFT = -2;
const int ORIG_RIGHT = -3;
const int EMPTY_STACK_VALUE = -4;

const int DEFAULT_ALPHABET_SIZE = 140;
const int DEFAULT_CHUNK_SIZE = 10000;

class SequenceGenerator {
 private:
  int curr_value;
  SequenceGenerator() : curr_value(0) { }
 public:
  static SequenceGenerator* getInstance() {
    static SequenceGenerator* instance = nullptr;

    if (instance == nullptr) {
      instance = new SequenceGenerator();
    }

    return instance;
  }
  void reset() {
    curr_value = 0;
  }
  int getNext() {
    return ++curr_value;
  }
  int getCurrent() {
    return curr_value;
  }
};

}

#endif // CONSTANTS_H_
