#include "bignum.h"

std::ostream& operator<<(std::ostream& out, const Bignum& b) {
  if (b.neg_1) {
    out << "-1";
    return out;
  }
  if (b.nums.size() == 0) {
    out << "0";
    return out;
  }
  out << b.nums[b.nums.size()-1];
  for (int i = b.nums.size()-2; i >= 0; --i)
    out << std::setfill('0') << std::setw(9) << b.nums[i];
  return out;
}

