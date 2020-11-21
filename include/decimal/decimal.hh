// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com

//
// Created by grakra on 2020/9/29.
//

#ifndef CPP_ETUDES_DECIMAL_HH
#define CPP_ETUDES_DECIMAL_HH

#include <cstddef>
#include <cstdint>
#include <cassert>
#include <string>
#include <random>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <climits>
#include <include/util/defer.hh>

typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

template<int64_t n>
struct Exp10 {
  static constexpr int64_t value = Exp10<n - 1>::value * 10;
};
template<>
struct Exp10<0> {
  static constexpr int64_t value = 1;
};

int64_t exp10s[] = {
    Exp10<0>::value,
    Exp10<1>::value,
    Exp10<2>::value,
    Exp10<3>::value,
    Exp10<4>::value,
    Exp10<5>::value,
    Exp10<6>::value,
    Exp10<7>::value,
    Exp10<8>::value,
    Exp10<9>::value,
    Exp10<10>::value,
    Exp10<11>::value,
    Exp10<12>::value,
    Exp10<13>::value,
    Exp10<14>::value,
    Exp10<15>::value,
    Exp10<16>::value,
    Exp10<17>::value,
    Exp10<18>::value,
};

typedef __int128 int128_t;
int128_t gen_decimal(int p, int s) {
  std::random_device rd;
  std::mt19937 gen(rd());
  assert(p - s <= 18 && s <= 18 && p >= 0 && s >= 0);
  const int64_t max_int_part = exp10s[p - s];
  const int64_t max_frac_part = exp10s[s];
  std::uniform_int_distribution<int64_t> ip_rand(-(max_int_part - 1), max_int_part - 1);
  std::uniform_int_distribution<int64_t> fp_rand(-(max_frac_part - 1), max_frac_part - 1);
  auto a = ip_rand(gen);
  auto b = fp_rand(gen);
  while (a == 0 && b == 0) {
    a = ip_rand(gen);
    b = fp_rand(gen);
  }
  if (b < 0) { b = -b; }
  auto positive = static_cast<int128_t>(a < 0 ? -1 : 1);
  if (a < 0) { a = -a; }
  return (static_cast<int128_t>(a) * max_frac_part + b) * positive;
}

std::string to_string(int128_t decimal, int p, int s) {
  const int64_t max_int_part = exp10s[p - s];
  const int64_t max_frac_part = exp10s[s];
  auto frac_part = abs(static_cast<int64_t>(decimal % max_frac_part));
  auto int_part = static_cast<int64_t>(decimal / max_frac_part);
  auto sign = int_part < 0 ? "-" : "";
  std::stringstream ss;
  ss << std::right << int_part;
  ss << ".";
  ss << std::noshowpos << std::setw(s) << std::setfill('0') << std::right << frac_part;
  return ss.str();
}

template<typename T, typename F>
void batch_compute(size_t n, T *lhs, T *rhs, T *result, F &&f) {
  for (auto i = 0; i < n; i++) {
    result[i] = f(lhs[i], rhs[i]);
  }
}

template<typename T, typename F>
void single_compute(size_t n, T *lhs, T *rhs, T *result, F &&f) {
  auto &a = lhs[0];
  auto &b = rhs[0];
  auto &c = result[0];
  for (int i=0;i<8192;++i) {
    c = f(a, b);
  }
}

union int128_wrapper {
  int128_t s128;
  uint128_t u128;
  struct {
#if __BYTE_ORDER == LITTLE_ENDIAN
    int64_t low;
    int64_t high;
#else
    int64_t high;
    int64_t low;
#endif
  } s;
  struct {
#if __BYTE_ORDER == LITTLE_ENDIAN
    uint64_t low;
    uint64_t high;
#else
    unt64_t high;
    uint64_t low;
#endif
  } u;
};

template<class T>
struct bit64_traits {};
template<>
struct bit64_traits<int64_t> { typedef int64_t type; };
template<>
struct bit64_traits<uint64_t> { typedef uint64_t type; };

struct DorisDecimalOp {
  enum DecimalError {
    E_DEC_OK = 0,
    E_DEC_TRUNCATED = 1,
    E_DEC_OVERFLOW = 2,
    E_DEC_DIV_ZERO = 4,
    E_DEC_BAD_NUM = 8,
    E_DEC_OOM = 16,

    E_DEC_ERROR = 31,
    E_DEC_FATAL_ERROR = 30
  };
  static constexpr int32_t PRECISION = 27;
  static constexpr int32_t SCALE = 9;
  static constexpr int64_t MAX_INT_VALUE = 999999999999999999ll;
  static constexpr int32_t MAX_FRAC_VALUE = 999999999l;
  static constexpr uint32_t ONE_BILLION = 1000000000;
  static constexpr int64_t MAX_INT64 = 9223372036854775807ll;

  static constexpr int128_t MAX_DECIMAL_VALUE =
      static_cast<int128_t>(MAX_INT_VALUE) * ONE_BILLION + MAX_FRAC_VALUE;
  static constexpr int128_t MIN_DECIMAL_VALUE = -MAX_DECIMAL_VALUE;

  template<
      class T,
      class U,
      class TT = typename bit64_traits<T>::type,
      class UU = typename bit64_traits<U>::type
  >
  static inline int asm_add(T x, U y, T &res) {
    int8_t overflow;
    // carry flag set 1 indicates overflow of unsigned add;
    // overflow flag(sign flag xor carry flag) set 1 for signed add overflow.
    if constexpr(std::is_unsigned<T>::value && std::is_unsigned<U>::value) {
      __asm__ __volatile__ (
      "mov %[x], %[res]\n\t"
      "add %[y], %[res]\n\t"
      "setc %b[overflow]\n\t"
      "sets %%r8b\n\t"
      "or %%r8b, %b[overflow]"
      : [res] "+r"(res), [overflow] "+r"(overflow)
      : [x] "r"(x), [y] "r"(y)
      : "cc", "r8"
      );
    } else {
      __asm__ __volatile__ (
      "mov %[x], %[res]\n\t"
      "add %[y], %[res]"
      : [res] "+r"(res), "=@cco"(overflow)
      : [x] "r"(x), [y] "r"(y)
      : "cc"
      );
    }
    return overflow;
  }

  template<
      class T,
      class U,
      class TT = typename bit64_traits<T>::type,
      class UU = typename bit64_traits<U>::type
  >
  static int asm_mul(T x, U y, int128_wrapper &res) {
    int8_t overflow;
    if constexpr(std::is_unsigned<T>::value && std::is_unsigned<U>::value) {
      __asm__ __volatile__ (
      "mov %[x], %%rax\n\t"
      "mul %[y]\n\t"
      "mov %%rdx, %[high]\n\t"
      "mov %%rax, %[low]"
      : [high] "=r"(res.u.high), [low] "=r"(res.u.low), "=@cco"(overflow)
      : [x] "r"(x), [y] "r"(y)
      : "cc", "rdx", "rax"
      );
    } else {
      __asm__ __volatile__(
      "mov %[x], %%rax\n\t"
      "imul %[y], %%rax\n\t"
      "mov %%rax, %[low]"
      : [low] "=r"(res.s.low), "=@cco"(overflow)
      : [x] "r"(x), [y] "r"(y)
      : "cc", "rdx", "rax"
      );
    }
    return overflow;
  }

  static inline int multi3(const int128_t &x, const int128_t &y, int128_t &res) {
    auto sx = x >> 127;
    auto sy = y >> 127;
    int128_wrapper wx = {.s128=(x ^ sx) - sx};
    int128_wrapper wy = {.s128=(y ^ sy) - sy};
    int128_wrapper wres;
    sx ^= sy;

    auto overflow = multi3(wx, wy, wres);

    res = (wres.s128 ^ sx) - sx;
    return overflow;
  }

  static inline int multi3(const int128_wrapper &x, const int128_wrapper &y, int128_wrapper &res) {
    auto no_zero = (x.u.low | x.u.high) || (y.u.low | y.u.high);
    if (__builtin_expect(!no_zero, 0)) {
      res.u128 = static_cast<int128_t>(0);
      return 0;
    }
    // x.u.high * y.u.high write into 128..256 bits of result
    int overflow = x.u.high != 0 && y.u.high != 0;

    asm_mul(x.u.low, y.u.low, res);
    int128_wrapper t0, t1;
    overflow |= asm_mul(x.u.low, y.u.high, t0);
    overflow |= asm_mul(y.u.low, x.u.high, t1);
    overflow |= asm_add(res.u.high, t0.u.low, res.u.high);
    overflow |= asm_add(res.u.high, t1.u.low, res.u.high);
    return overflow;
  }

  static inline void divmodti3(int128_t x, int128_t y, int128_t &q, uint128_t &r) {
    static constexpr int N = 127;
    int128_t s_x = x >> 127;
    int128_t s_y = y >> 127;
    x = (x ^ s_x) - s_x;
    y = (y ^ s_y) - s_y;
    q = udivmodti4(x, y, &r);
    s_y ^= s_x;
    q = (q ^ s_y) - s_y;
    r = (r ^ s_x) - s_x;
  }

  static inline uint64_t udiv128by64to64default(uint64_t u1, uint64_t u0, uint64_t v, uint64_t *r) {
    static constexpr unsigned n_udword_bits = sizeof(uint64_t) * CHAR_BIT;
    const uint64_t b = (1ULL << (n_udword_bits / 2)); // Number base (32 bits)
    uint64_t un1, un0;                                // Norm. dividend LSD's
    uint64_t vn1, vn0;                                // Norm. divisor digits
    uint64_t q1, q0;                                  // Quotient digits
    uint64_t un64, un21, un10;                        // Dividend digit pairs
    uint64_t rhat;                                    // A remainder
    int32_t s;                                       // Shift amount for normalization

    s = __builtin_clzll(v);
    if (s > 0) {
      // Normalize the divisor.
      v = v << s;
      un64 = (u1 << s) | (u0 >> (n_udword_bits - s));
      un10 = u0 << s; // Shift dividend left
    } else {
      // Avoid undefined behavior of (u0 >> 64).
      un64 = u1;
      un10 = u0;
    }

    // Break divisor up into two 32-bit digits.
    vn1 = v >> (n_udword_bits / 2);
    vn0 = v & 0xFFFFFFFF;

    // Break right half of dividend into two digits.
    un1 = un10 >> (n_udword_bits / 2);
    un0 = un10 & 0xFFFFFFFF;

    // Compute the first quotient digit, q1.
    q1 = un64 / vn1;
    rhat = un64 - q1 * vn1;

    // q1 has at most error 2. No more than 2 iterations.
    while (q1 >= b || q1 * vn0 > b * rhat + un1) {
      q1 = q1 - 1;
      rhat = rhat + vn1;
      if (rhat >= b)
        break;
    }

    un21 = un64 * b + un1 - q1 * v;

    // Compute the second quotient digit.
    q0 = un21 / vn1;
    rhat = un21 - q0 * vn1;

    // q0 has at most error 2. No more than 2 iterations.
    while (q0 >= b || q0 * vn0 > b * rhat + un0) {
      q0 = q0 - 1;
      rhat = rhat + vn1;
      if (rhat >= b)
        break;
    }

    *r = (un21 * b + un0 - q0 * v) >> s;
    return q1 * b + q0;
  }

  static inline uint64_t udiv128by64to64(uint64_t u1, uint64_t u0, uint64_t v,
                                         uint64_t *r) {
#if defined(__x86_64__)
    uint64_t result;
    __asm__("divq %[v]"
    : "=a"(result), "=d"(*r)
    : [ v ] "r"(v), "a"(u0), "d"(u1));
    return result;
#else
    return udiv128by64to64default(u1, u0, v, r);
#endif
  }

  // Effects: if rem != 0, *rem = a % b
  // Returns: a / b
  static inline uint128_t udivmodti4(uint128_t a, uint128_t b, uint128_t *rem) {
    static constexpr unsigned n_utword_bits = sizeof(uint128_t) * CHAR_BIT;
    int128_wrapper dividend;
    dividend.u128 = a;
    int128_wrapper divisor;
    divisor.u128 = b;
    int128_wrapper quotient;
    int128_wrapper remainder;
    if (divisor.u128 > dividend.u128) {
      if (rem)
        *rem = dividend.u128;
      return 0;
    }
    // When the divisor fits in 64 bits, we can use an optimized path.
    if (divisor.u.high == 0) {
      remainder.u.high = 0;
      if (dividend.u.high < divisor.u.low) {
        // The result fits in 64 bits.
        quotient.u.low = udiv128by64to64(dividend.u.high, dividend.u.low,
                                         divisor.u.low, &remainder.u.low);
        quotient.u.high = 0;
      } else {
        // First, divide with the high part to get the remainder in dividend.s.high.
        // After that dividend.s.high < divisor.s.low.
        quotient.u.high = dividend.u.high / divisor.u.low;
        dividend.u.high = dividend.u.high % divisor.u.low;
        quotient.u.low = udiv128by64to64(dividend.u.high, dividend.u.low,
                                         divisor.u.low, &remainder.u.low);
      }
      if (rem)
        *rem = remainder.u128;
      return quotient.u128;
    }
    // 0 <= shift <= 63.
    int32_t shift =
        __builtin_clzll(divisor.u.high) - __builtin_clzll(dividend.u.high);
    divisor.u128 <<= shift;
    quotient.u.high = 0;
    quotient.u.low = 0;
    for (; shift >= 0; --shift) {
      quotient.u.low <<= 1;
      // Branch free version of.
      // if (dividend.u128 >= divisor.u128)
      // {
      //    dividend.u128 -= divisor.u128;
      //    carry = 1;
      // }
      const int128_t s =
          (int128_t) (divisor.u128 - dividend.u128 - 1) >> (n_utword_bits - 1);
      quotient.u.low |= s & 1;
      dividend.u128 -= divisor.u128 & s;
      divisor.u128 >>= 1;
    }
    if (rem)
      *rem = dividend.u128;
    return quotient.u128;
  }

  int do_add2(int128_t x, int128_t y, int128_t &res) {
    res = x + y;
    auto s = res >> 127;
    res = (res ^ s) - s;

    if (__builtin_expect(res > MAX_DECIMAL_VALUE, 0)) {
      res = MAX_DECIMAL_VALUE;
    }
    res = (res ^ s) - s;
    return res;
  }

  int128_t add2(int128_t x, int128_t y) {
    int128_t res;
    do_add2(x, y, res);
    return res;
  }

  int128_t add(int128_t x, int128_t y) {
    int128_t result;
    if (x == 0) {
      result = y;
    } else if (y == 0) {
      result = x;
    } else if (x > 0) {
      if (y > 0) {
        do_add(x, y, &result);
      } else {
        do_sub(x, -y, &result);
      }
    } else { // x < 0
      if (y > 0) {
        do_sub(y, -x, &result);
      } else {
        do_add(-x, -y, &result);
        result = -result;
      }
    }
    return result;
  }

  int do_add(int128_t x, int128_t y, int128_t *result) {
    int error = E_DEC_OK;
    if (MAX_DECIMAL_VALUE - x >= y) {
      *result = x + y;
    } else {
      *result = MAX_DECIMAL_VALUE;
      error = E_DEC_OVERFLOW;
    }
    return error;
  }

  int do_sub(int128_t x, int128_t y, int128_t *result) {
    int error = E_DEC_OK;
    *result = x - y;
    return error;
  }

  int128_t abs(const int128_t &x) { return (x < 0) ? -x : x; }

  int unsignedMulOverflow(const int128_t &x, const int128_t &y, int128_t &result) {
    result = x * y;
    int error = E_DEC_OK;
    int128_t max128 = ~(static_cast<int128_t>(1ll) << 127);

    int leading_zero_bits = clz128(x) + clz128(y);
    int overflow = leading_zero_bits < sizeof(int128_t) || max128 / x < y;
    return overflow;
  }
  int mulOverflow(const int128_t &x, const int128_t &y, int128_t &result) {
    if (x == 0 || y == 0) return static_cast<int128_t>(0);
    bool is_positive = (x > 0 && y > 0) || (x < 0 && y < 0);
    auto overflow = unsignedMulOverflow(abs(x), abs(y), result);
    if (!is_positive) result = -result;
    return overflow;
  }
  int128_t mul(const int128_t &x, const int128_t &y) {
    int128_t result;
    if (x == 0 || y == 0) return static_cast<int128_t>(0);

    bool is_positive = (x > 0 && y > 0) || (x < 0 && y < 0);

    do_mul(abs(x), abs(y), &result);

    if (!is_positive) result = -result;

    return result;
  }

  int128_t mul2(const int128_t &x, const int128_t &y) {
    int128_t res;
    auto wx = (int128_wrapper *) (&x);
    auto wy = (int128_wrapper *) (&y);
    auto wres = (int128_wrapper *) (&res);
    multi3(*wx, *wy, *wres);
    uint128_t remainder;
    divmodti3(res, ONE_BILLION, res, remainder);
    if (remainder != 0) {
      int error = E_DEC_TRUNCATED;
      if (remainder >= (ONE_BILLION >> 1)) {
        res += 1;
      }
    }
    return res;
  }

  int do_mul(int128_t x, int128_t y, int128_t *result) {
    int error = E_DEC_OK;
    int128_t max128 = ~(static_cast<int128_t>(1ll) << 127);

    int leading_zero_bits = clz128(x) + clz128(y);
    if (leading_zero_bits < sizeof(int128_t) || max128 / x < y) {
      *result = MAX_DECIMAL_VALUE;
      error = E_DEC_OVERFLOW;
      return error;
    }

    int128_t product = x * y;
    *result = product / ONE_BILLION;

    // overflow
    if (*result > MAX_DECIMAL_VALUE) {
      *result = MAX_DECIMAL_VALUE;
      error = E_DEC_OVERFLOW;
      return error;
    }

    // truncate with round
    int128_t remainder = product % ONE_BILLION;
    if (remainder != 0) {
      error = E_DEC_TRUNCATED;
      if (remainder >= (ONE_BILLION >> 1)) {
        *result += 1;
      }
    }
    return error;
  }

  int clz128(unsigned __int128 v) {
    if (v == 0) return sizeof(__int128);
    unsigned __int128 shifted = v >> 64;
    if (shifted != 0) {
      return __builtin_clzll(shifted);
    } else {
      return __builtin_clzll(v) + 64;
    }
  }

  int128_t div(const int128_t &x, const int128_t &y) {
    int128_t result;
    //todo: return 0 for divide zero
    if (x == 0 || y == 0) return static_cast<int128_t>(0);
    bool is_positive = (x > 0 && y > 0) || (x < 0 && y < 0);
    do_div(abs(x), abs(y), &result);

    if (!is_positive) result = -result;

    return result;
  }

  int128_t div2(const int128_t &x, const int128_t &y) {
    int128_t result;
    // todo: return 0 for divide zero
    if (x == 0 || y == 0) return 0;
    //
    int128_t dividend = x * ONE_BILLION;

    uint128_t remainder;
    divmodti3(dividend, y, result, remainder);
    // round if remainder >= 0.5*y
    if (remainder != 0) {
      if (remainder >= (y >> 1)) {
        result += 1;
      }
    }
    return result;
  }

  int do_div(int128_t x, int128_t y, int128_t *result) {
    int error = E_DEC_OK;
    int128_t dividend = x * ONE_BILLION;
    *result = dividend / y;

    // overflow
    int128_t remainder = dividend % y;
    if (remainder != 0) {
      error = E_DEC_TRUNCATED;
      if (remainder >= (y >> 1)) {
        *result += 1;
      }
    }
    return error;
  }
};

template<bool adjust_scale, bool scale_left, bool check_overflow, bool can_overflow>
struct CKDecimalOp {
  struct Exception {
    std::string msg;
    int code;
    Exception(std::string const &msg, int code) : msg(msg), code(code) {}
  };
  static constexpr __int128 minInt128() { return static_cast<unsigned __int128>(1) << 127; }
  static constexpr __int128 maxInt128() { return (static_cast<unsigned __int128>(1) << 127) - 1; }

  inline bool addOverflow(__int128 x, __int128 y, __int128 &res) {
    static constexpr __int128 min_int128 = minInt128();
    static constexpr __int128 max_int128 = maxInt128();
    res = x + y;
    return (y > 0 && x > max_int128 - y) || (y < 0 && x < min_int128 - y);
  }

  inline bool mulOverflow(__int128 x, __int128 y, __int128 &res) {
    res = static_cast<unsigned __int128>(x) * static_cast<unsigned __int128>(y);    /// Avoid signed integer overflow.
    if (!x || !y)
      return false;

    unsigned __int128 a = (x > 0) ? x : -x;
    unsigned __int128 b = (y > 0) ? y : -y;
    return (a * b) / b != a;
  }

  inline bool mulOverflow_nodiv(__int128 x, __int128 y, __int128 &res) {
    res = static_cast<unsigned __int128>(x) * static_cast<unsigned __int128>(y);    /// Avoid signed integer overflow.
    if (!x || !y)
      return false;

    unsigned __int128 a = (x > 0) ? x : -x;
    unsigned __int128 b = (y > 0) ? y : -y;
    return (a * b) + b != a;
  }

  int128_t add(int128_t a, int128_t b, int128_t scale) {
    int128_t res;

    if constexpr (check_overflow) {
      bool overflow = false;
      if constexpr (adjust_scale) {
        if constexpr (scale_left)
          overflow |= mulOverflow(a, scale, a);
        else
          overflow |= mulOverflow(b, scale, b);
      }
      if constexpr (can_overflow)
        overflow |= addOverflow(a, b, res);
      else
        res = a + b;
      if (overflow)
        return 0;
    } else {
      if constexpr (adjust_scale) {
        if constexpr (scale_left)
          a *= scale;
        else
          b *= scale;
      }
      res = a + b;
    }
    return res;
  }

  int128_t mul(int128_t a, int128_t b) {
    if constexpr (can_overflow && check_overflow) {
      int128_t res;
      if (mulOverflow(a, b, res))
        return 0;
      else
        return res;
    } else
      return a * b;
  }

  template<bool is_decimal_a>
  int128_t div(int128_t a, int128_t b, int128_t scale) {
    if constexpr (check_overflow) {
      bool overflow = false;
      if constexpr (!is_decimal_a)
        overflow |= mulOverflow(scale, scale, scale);
      overflow |= mulOverflow(a, scale, a);
      if (overflow)
        return static_cast<int128_t>(-1);
    } else {
      if constexpr (!is_decimal_a)
        scale *= scale;
      a *= scale;
    }
    return a / b;
  }
};

struct PrepareData {
  size_t batch_size = 8192;
  int128_t zero = static_cast<int128_t>(0);
  std::vector<int128_t> lhs;
  std::vector<int128_t> rhs;
  std::vector<int128_t> result;

  std::vector<int64_t> lhs64;
  std::vector<int64_t> rhs64;
  std::vector<int64_t> result64;

  std::vector<int32_t> lhs32;
  std::vector<int32_t> rhs32;
  std::vector<int32_t> result32;

  PrepareData() {
    auto batch_size_env_value = getenv("batch_size");
    if (batch_size_env_value != nullptr) {
      batch_size = strtoul(batch_size_env_value, nullptr, 10);
      if (batch_size <= 0) {
        batch_size = 8192;
      }
    } else {
      batch_size = 8192;
    }

    auto fill_zero_env_value = getenv("fill_zero");
    auto fill_zero = false;
    if (fill_zero_env_value != nullptr && strncasecmp(fill_zero_env_value, "true", 4) == 0) {
      fill_zero = true;
    }

    auto fill_max_env_value = getenv("fill_max");
    auto fill_max = false;
    if (fill_max_env_value != nullptr && strncasecmp(fill_max_env_value, "true", 4) == 0) {
      fill_max = true;
    }

    std::cout << std::boolalpha << "prepare data: batch_size=" << batch_size
              << ", fill_zero=" << fill_zero
              << ", fill_max=" << fill_max
              << std::endl;

    DEFER([]() {
      std::cout << "prepare data: Done" << std::endl;
    })

    lhs.resize(batch_size, zero);
    rhs.resize(batch_size, zero);
    result.resize(batch_size, zero);

    lhs64.resize(batch_size, 0);
    rhs64.resize(batch_size, 0);
    result64.resize(batch_size, 0);

    lhs32.resize(batch_size, 0);
    rhs32.resize(batch_size, 0);
    result32.resize(batch_size, 0);

    if (fill_zero) {
      return;
    }

    auto max_decimal = DorisDecimalOp::MAX_DECIMAL_VALUE;
    if (fill_max) {
      lhs.resize(0);
      rhs.resize(0);
      lhs.resize(batch_size, max_decimal);
      rhs.resize(batch_size, max_decimal);

      lhs64.resize(0);
      rhs64.resize(0);
      lhs64.resize(batch_size, INT64_MAX);
      rhs64.resize(batch_size, INT64_MAX);

      lhs32.resize(0);
      rhs32.resize(0);
      lhs32.resize(batch_size, INT32_MAX);
      rhs32.resize(batch_size, INT32_MAX);
      return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    /*
    std::uniform_int_distribution<int64_t> rand(INT64_MIN, INT64_MAX);


    for (int i = 0; i < batch_size; ++i) {
      int64_t hi = rand(gen);
      int64_t lo = rand(gen);
      lhs[i] = (static_cast<int128_t>(hi) << 64) + lo;
      do {
        hi = rand(gen);
        lo = rand(gen);
        rhs[i] = (static_cast<int128_t>(hi) << 64) + lo;
      } while (rhs[i] == zero);
    }
    */

    std::uniform_int_distribution<int64_t> ip_rand(-99999, 99999);
    std::uniform_int_distribution<int64_t> fp_rand(0, 99);
    for (auto i = 0; i < batch_size; ++i) {
      auto gen_int128 = [&]() -> int128_t {
        auto a = ip_rand(gen);
        auto b = fp_rand(gen);
        while (a == 0 && b == 0) {
          a = ip_rand(gen);
          b = fp_rand(gen);
        }
        if (b < 0) { b = -b; }
        auto positive = static_cast<int128_t>(a < 0 ? -1 : 1);
        return (static_cast<int128_t>(a) * 100 + b) * positive;
      };
      lhs[i] = gen_int128();
      rhs[i] = gen_int128();
      lhs64[i] = static_cast<int64_t>(lhs[i] >> 64) | static_cast<int64_t>(lhs[i]);
      rhs64[i] = static_cast<int64_t>(rhs[i] >> 64) | static_cast<int64_t>(rhs[i]);
      lhs32[i] = static_cast<int32_t>(lhs64[i] >> 32) | static_cast<int32_t>(lhs64[i]);
      rhs32[i] = static_cast<int32_t>(rhs64[i] >> 32) | static_cast<int32_t>(rhs64[i]);
    }
  }
};
static inline std::string ToHexString(int128_t x) {
  int128_wrapper wx = {.s128 =x};
  std::stringstream ss;
  ss
      << std::hex << std::showbase
      << "{high=" << wx.s.high
      << ", low=" << wx.s.low << "}";
  return ss.str();
}

static inline int128_t gen_int128(int n) {
  std::random_device rd;
  std::mt19937 gen(rd());
  assert(0 <= n && n <= 128);

  int low_bits = 64;
  int high_bits = 64;
  if (n > 64) {
    low_bits = 64;
    high_bits = n - 64;
  } else {
    low_bits = n;
    high_bits = 0;
  }

  int64_t min_low_part = 0;
  int64_t max_low_part = 0;
  if (low_bits < 64) {
    max_low_part = (1l < low_bits) - 1;
    min_low_part = -max_low_part;
  } else {
    max_low_part = -1l ^ (1l << 63);
    min_low_part = 1l << 63;
  }

  int64_t min_high_part = 0;
  int64_t max_high_part = 0;
  if (high_bits < 64) {
    max_high_part = (1l < high_bits) - 1;
    min_high_part = -max_low_part;
  } else {
    max_high_part = -1l ^ (1l << 63);
    min_high_part = 1l << 63;
  }

  std::uniform_int_distribution<int64_t> low_rand(min_low_part, max_low_part);
  std::uniform_int_distribution<int64_t> high_rand(min_high_part, max_high_part);
  auto a = high_rand(gen);
  auto b = static_cast<uint64_t>(low_rand(gen));
  return (static_cast<int128_t>(a) << 64) ^ static_cast<int128_t>(b);
}
#endif //CPP_ETUDES_DECIMAL_HH
