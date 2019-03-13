#ifndef BIGNUM_H_
#define BIGNUM_H_

#include <vector>
#include <iostream>
#include <iomanip>

const int BASE = 1e9;

class Bignum {
 private:
  std::vector<int> nums;
  bool neg_1;

 public:
  Bignum() : neg_1(false) { nums.push_back(0); }
  Bignum(long long int x) {
    neg_1 = (x < 0);
    while (x > 0) {
      nums.push_back(x % BASE);
      x /= BASE;
    }
  }

  Bignum& operator+=(const Bignum& rhs) {
    if (neg_1 || rhs.neg_1) {
      std::cout << "ERROR: adding negative bignum(s)\n";
      return *this;
    }
    int carry = 0;
    int size = std::min(nums.size(), rhs.nums.size());
    for (int i = 0; i < size; ++i) {
      nums[i] += rhs.nums[i] + carry;
      carry = nums[i] / BASE;
      nums[i] %= BASE;
    }
    int itr = size;
    while (carry > 0) {
      if (itr >= nums.size()) {
        nums.push_back(carry);
        carry = 0;
      }
      else {
        nums[itr] += carry;
        carry = nums[itr] / BASE;
        nums[itr] %= BASE;
        itr++;
      }
    }
    return *this;
  }

  Bignum& operator*=(int mult) {
    if (neg_1) {
      std::cout << "ERROR: multiplying negative bignum\n";
      return *this;
    }
    int carry = 0;
    for (int i = 0; i < nums.size(); ++i) {
      long long int tmp = nums[i];
      tmp *= mult;
      tmp += carry;
      nums[i] = tmp % BASE;
      carry = tmp / BASE;
    }
    if (carry > 0) nums.push_back(carry);
    return *this;
  }

  bool operator<(long long int x) const {
    if (neg_1)
      return (-1 < x);
    if (nums.size() > 1) return false;
    if (nums.size() == 0) return (0 < x);
    return (nums[0] < x);
  }

  static Bignum pow(int a, int b) {
    Bignum result(1);
    for (int i = 0; i < b; ++i)
      result *= a;
    return result;
  }

  friend std::ostream& operator<<(std::ostream& out, const Bignum& b);
};

std::ostream& operator<<(std::ostream& out, const Bignum& b) {
  if (b.nums.size() == 0) {
    out << "0";
    return out;
  }
  if (b.neg_1) {
    out << "-1";
    return out;
  }
  out << b.nums[b.nums.size()-1];
  for (int i = b.nums.size()-2; i >= 0; --i)
    out << std::setfill('0') << std::setw(9) << b.nums[i];
  return out;
}

#endif // BIGNUM_H_
