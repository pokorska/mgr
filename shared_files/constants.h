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

}

#endif // CONSTANTS_H_
