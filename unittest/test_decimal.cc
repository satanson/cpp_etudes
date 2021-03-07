// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/10/14.
//

#include <gtest/gtest.h>
#include <include/decimal/decimal.hh>
#include <sstream>
namespace test {
class TestDecimal : public testing::Test {
public:
  static inline std::string ToHexString(int128_t x) {
    int128_wrapper wx = {.s128 = x};
    std::stringstream ss;
    ss << std::hex << std::showbase << "{high=" << wx.s.high
       << ", low=" << wx.s.low << "}";
    return ss.str();
  }

  static inline void compareAdd(int128_t a, int128_t b) {
    DorisDecimalOp op;
    auto r0 = op.add(a, b);
    auto r1 = op.add2(a, b);
    if (r0 != r1) {
      std::cout << "a=" << ToHexString(a) << std::endl;
      std::cout << "b=" << ToHexString(b) << std::endl;
      std::cout << "r0=" << ToHexString(r0) << std::endl;
      std::cout << "r1=" << ToHexString(r1) << std::endl;
      ASSERT_FALSE(true);
    }
  }
  template <typename F> void testCompare(F &&f) {
    for (int p1 = 1; p1 <= 36; ++p1) {
      int s1 = std::max(0, p1 - 18);
      for (int p2 = 1; p2 <= 36; ++p2) {
        int s2 = std::max(0, p2 - 18);
        // std::cout<<"BEGIN TEST: p1="<<p1<<", s1="<<s1<<"; p2="<<p2<<",
        // s2="<<s2<<std::endl;
        for (int i = 0; i < 1; ++i) {
          auto a = gen_decimal(p1, s1);
          auto b = gen_decimal(p2, s2);
          f(a, b);
        }
        // std::cout<<"END TEST: p1="<<p1<<", s1="<<s1<<"; p2="<<p2<<",
        // s2="<<s2<<std::endl;
      }
    }
  }

  void compareMul(int128_t a, int128_t b) {
    DorisDecimalOp op;
    int128_t c1;
    auto overflow1 = op.mulOverflow(a, b, c1);
    int128_t c2 = 0;
    auto overflow2 = op.multi3(a, b, c2);
    ASSERT_EQ(c1, c2);
    if (overflow1 != overflow2) {
      int128_wrapper wa = {.s128 = a};
      int128_wrapper wb = {.s128 = b};
      int128_wrapper wc = {.s128 = c1};
      std::cout << std::showbase << std::hex << "a=" << to_string(a, 36, 9)
                << std::endl
                << "b=" << to_string(b, 36, 9) << std::endl
                << "a.high=" << wa.s.high << ", a.low=" << wa.s.low << "\n"
                << "b.high=" << wb.s.high << ", b.low=" << wb.s.low << "\n"
                << "result.high=" << wc.s.high << ", result.low=" << wb.s.low
                << "\n"
                << "overflow1=" << overflow1 << ", overflow2=" << overflow2
                << std::endl;
      ASSERT_FALSE(true);
    }
  }
};
TEST_F(TestDecimal, testDorisMulAndMul2) {
  PrepareData data;
  auto &lhs = data.lhs;
  auto &rhs = data.rhs;
  auto batch_size = data.batch_size;
  DorisDecimalOp op;
  for (auto i = 0; i < batch_size; ++i) {
    compareMul(lhs[i], rhs[i]);
  }
}

TEST_F(TestDecimal, testMaxMinDecimalValue) {
  std::cout << "max=" << ToHexString(DorisDecimalOp::MAX_DECIMAL_VALUE)
            << std::endl;
  std::cout << "min=" << ToHexString(DorisDecimalOp::MIN_DECIMAL_VALUE)
            << std::endl;
}

TEST_F(TestDecimal, testCompareNewAndOldMul) {
  for (int p1 = 1; p1 <= 36; ++p1) {
    int s1 = std::max(0, p1 - 18);
    for (int p2 = 1; p2 <= 36; ++p2) {
      int s2 = std::max(0, p2 - 18);
      // std::cout<<"BEGIN TEST: p1="<<p1<<", s1="<<s1<<"; p2="<<p2<<",
      // s2="<<s2<<std::endl;
      for (int i = 0; i < 1024; ++i) {
        auto a = gen_decimal(p1, s1);
        auto b = gen_decimal(p2, s2);
        compareMul(a, b);
      }
      // std::cout<<"END TEST: p1="<<p1<<", s1="<<s1<<"; p2="<<p2<<",
      // s2="<<s2<<std::endl;
    }
  }
}

TEST_F(TestDecimal, testCompareAdd) {
  for (int i = 0; i < 8192; ++i) {
    auto a = gen_decimal(27, 9);
    auto b = gen_decimal(27, 9);
    compareAdd(a, b);
  }
}

TEST_F(TestDecimal, testDivMod) {
  testCompare([](int128_t x, int128_t y) {
    auto q0 = x / y;
    auto r0 = x % y;
    int128_t q1;
    uint128_t r1;
    DorisDecimalOp::divmodti3(x, y, q1, r1);
    ASSERT_EQ(q0, q1);
    ASSERT_EQ(r0, r1);
  });
}

TEST_F(TestDecimal, testOverflow) {
  std::vector<std::tuple<int128_t, int128_t, int128_t, int>> data = {
      {0, 0, 0, 0},
      {-1, -1, 1, 0},
      {-1, 1, -1, 0},
      {1, -1, -1, 0},
      {0xffff'ffff'ffff'ffffl, 0xffff'ffff'ffff'ffffl,
       (static_cast<int128_t>(0xfffffffffffffffe) << 64) + 0000000000000001, 1},
      {0x8000'0000'0000'0000l, 0x8000'0000'0000'0000l,
       (static_cast<int128_t>(0x4000'0000'0000'0000l) << 64), 0},
      {0x8000'0000'0000'0000l, (static_cast<int128_t>(1) << 64),
       (static_cast<int128_t>(0x8000'0000'0000'0000l) << 64), 1},
      {0xffff'ffff'ffff'ffffl, (static_cast<int128_t>(1) << 64),
       (static_cast<int128_t>(0xffff'ffff'ffff'ffffl) << 64), 1},
      {(static_cast<int128_t>(0xffff'ffff'ffff'ffffl) << 64),
       (static_cast<int128_t>(1) << 63),
       (static_cast<int128_t>(0x8000'0000'0000'0000l) << 64), 1},
      {(static_cast<int128_t>(0xffff'ffff'ffff'ffffl) << 64),
       (static_cast<int128_t>(-1l) << 64) + 0x8000'0000'0000'0000l,
       (static_cast<int128_t>(1) << 127), 1},
      {(static_cast<int128_t>(0xffff'ffff'ffff'ffffl) << 64),
       (static_cast<int128_t>(-1l) << 64) + 0xc000'0000'0000'0000l,
       (static_cast<int128_t>(1) << 126), 0},
  };
  for (auto &d : data) {
    auto [x, y, expect_result, expect_overflow] = d;
    std::cout << std::endl << std::endl;
    std::cout << "x=" << ToHexString(x) << std::endl
              << "y=" << ToHexString(y) << std::endl
              << "expect_result=" << ToHexString(expect_result) << std::endl
              << "expect_overflow=" << expect_overflow << std::endl;
    int128_t actual_result;
    int actual_overflow = DorisDecimalOp::multi3(x, y, actual_result);
    ASSERT_EQ(expect_result, actual_result);
    ASSERT_EQ(expect_overflow, actual_overflow);
  }
}

TEST_F(TestDecimal, testMul) {
  int64_t a = 0xffffffffffffffffl;
  int64_t b = 0xffffffffffffffffl;
  int128_wrapper ab = {.s128 = 0};
  int overflow = DorisDecimalOp::asm_mul(a, b, ab);

  std::cout << std::showbase << std::hex << "a=" << a << std::endl
            << "b=" << b << std::endl
            << "ab.high=" << ab.s.high << ", ab.low=" << ab.s.low << std::endl
            << "overflow=" << overflow << std::endl;
}

} // namespace test
#if defined(__x86_64__) && defined(__GNUC__)
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
#endif
