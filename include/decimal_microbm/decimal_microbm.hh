//
// Created by grakra on 2020/9/29.
//

#ifndef CPP_ETUDES_DECIMAL_MICROBM_HH
#define CPP_ETUDES_DECIMAL_MICROBM_HH

#include <cstddef>
#include <cstdint>
#include <string>
typedef __int128 int128_t;
template<typename T, typename F>
void batch_compute(size_t n, T *lhs, T *rhs, T *result, F &&f) {
  for (auto i = 0; i < n; i++) {
    result[i] = f(lhs[i], rhs[i]);
  }
}

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
  static const int32_t PRECISION = 27;
  static const int32_t SCALE = 9;
  static const uint32_t ONE_BILLION = 1000000000;
  static const int64_t MAX_INT_VALUE = 999999999999999999ll;
  static const int32_t MAX_FRAC_VALUE = 999999999l;
  static const int64_t MAX_INT64 = 9223372036854775807ll;

  static const int128_t MAX_DECIMAL_VALUE =
      static_cast<int128_t>(MAX_INT64) * ONE_BILLION + MAX_FRAC_VALUE;

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

  int128_t mul(const int128_t &x, const int128_t &y) {
    int128_t result;
    if (x == 0 || y == 0) return static_cast<int128_t>(0);

    bool is_positive = (x > 0 && y > 0) || (x < 0 && y < 0);

    do_mul(abs(x), abs(y), &result);

    if (!is_positive) result = -result;

    return result;
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
#endif //CPP_ETUDES_DECIMAL_MICROBM_HH
