#ifndef BIGNUM_H_
#define BIGNUM_H_

#include <vector>
#include <iostream>
#include <iomanip>
#include <cstring>

const int BASE = 1e9;

class Bignum {
 private:
  std::vector<int> nums;
  bool neg_1;

  void subtract_1() {
    if ((*this) == 0) {
      neg_1 = true;
      return;
    }
    int carry = -1, itr = 0;
    while (carry != 0 && itr < nums.size()) {
      nums[itr] += carry;
      if (nums[itr] < 0) {
        nums[itr] += BASE;
        carry = -1;
      }
      else carry = 0;
      itr++;
    }
    while (nums.back() == 0 && nums.size() > 1) nums.pop_back();
  }

 public:
  Bignum() : neg_1(false) { nums.push_back(0); }
  Bignum(long long int x) : neg_1(false) {
    neg_1 = (x < 0);
    while (x > 0) {
      nums.push_back(x % BASE);
      x /= BASE;
    }
    if (nums.size() == 0) nums.push_back(0);
  }
  Bignum(const std::string& x) : neg_1(false) {
    if (x[0] == '-') {
      neg_1 = true;
    } else {
      nums.push_back(0);
      for (int i = 0; i < x.size(); ++i) {
        (*this) *= 10;
        (*this) += x[i] - '0';
      }
    }
  }

  Bignum& operator+=(const Bignum& rhs) {
    if (neg_1 && rhs.neg_1) {
      std::cout << "ERROR: adding negative bignum(s)\n";
      return *this;
    }
    if (rhs.neg_1) {
      subtract_1();
      return *this;
    }
    if (neg_1) {
      *this = std::move(rhs);
      subtract_1();
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
    while (carry > 0 || itr < rhs.nums.size()) {
      if (itr < rhs.nums.size()) carry += rhs.nums[itr];
      if (itr >= nums.size()) {
        nums.push_back(carry);
      }
      else {
        nums[itr] += carry;
      }
      carry = nums[itr] / BASE;
      nums[itr] %= BASE;
      itr++;
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

  bool operator==(long long int x) const {
    if (neg_1)
      return (x == -1);
    if (nums.size() > 1) return false;
    if (nums.size() == 0) return (x == 0);
    return nums[0] == x;
  }

  bool operator>(long long int x) const {
    if ((*this) < x) return false;
    if ((*this) == x) return false;
    return true;
  }

  int getInt() const {
    if (nums.size() < 1) return 0;
    if (nums.size() > 1)
      std::cout << "Warning: cutting bignum to integer\n";
    return nums[0];
  }

  static Bignum pow(int a, int b) {
    Bignum result(1);
    for (int i = 0; i < b; ++i)
      result *= a;
    return result;
  }

  friend std::ostream& operator<<(std::ostream& out, const Bignum& b);
};


#endif // BIGNUM_H_
