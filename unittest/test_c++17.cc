// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/10/4.
//

#include <gtest/gtest.h>

class TestCpp17 : public ::testing::Test {};

template <typename T, typename... Args>
void push_back_vec0(std::vector<T>& v, Args&&... args) {
    static_assert((std::is_constructible_v<T, Args&&> && ...));
    (v.push_back(std::forward<Args>(args)), ...);
}

template <typename T, typename... Args>
void push_back_vec1(std::vector<T>& v, Args&&... args) {
    static_assert((std::is_constructible_v<T, Args&&> && ...));
    (..., v.push_back(std::forward<Args>(args)));
}
struct MyInt {
    int value;
    explicit MyInt(int value) : value(value) {}
    MyInt operator+(MyInt const& rhs) { return MyInt((this->value + 1) * rhs.value); }
};
template <typename... Args>
MyInt add0(Args&&... args) {
    return (args + ...);
}

template <typename... Args>
MyInt add1(Args&&... args) {
    return (... + args);
}
TEST_F(TestCpp17, testA) {
    std::vector<int32_t> v0;
    std::vector<int32_t> v1;
    push_back_vec0(v0, 1, 2, 3, 4, 5, 6);
    push_back_vec1(v1, 1, 2, 3, 4, 5, 6);
    for (auto& a : v0) {
        std::cout << a << ", ";
    }
    std::cout << std::endl;
    for (auto& a : v1) {
        std::cout << a << ", ";
    }
    std::cout << std::endl;
    std::cout << add0(MyInt(1), MyInt(2), MyInt(3)).value << std::endl;
    std::cout << add1(MyInt(1), MyInt(2), MyInt(3)).value << std::endl;
    std::tie();
    auto a = __is_empty(int);
}

TEST_F(TestCpp17, testLongLong) {
    std::cout << sizeof(__int128) << std::endl;
}
namespace common {
template <typename T>
inline bool mulOverflow(T x, T y, T& res) {
    std::cout << "call generic mulOverflow" << std::endl;
    return __builtin_mul_overflow(x, y, &res);
}

template <>
inline bool mulOverflow(int x, int y, int& res) {
    return __builtin_smul_overflow(x, y, &res);
}

template <>
inline bool mulOverflow(long x, long y, long& res) {
    return __builtin_smull_overflow(x, y, &res);
}

template <>
inline bool mulOverflow(long long x, long long y, long long& res) {
    return __builtin_smulll_overflow(x, y, &res);
}

/*
template<>
inline bool mulOverflow(__int128 x, __int128 y, __int128 &res) {
  std::cout << "call __int128 mulOverflow"<<std::endl;
  res = static_cast<unsigned __int128>(x) * static_cast<unsigned __int128>(y);
/// Avoid signed integer overflow. if (!x || !y) return false;

  unsigned __int128 a = (x > 0) ? x : -x;
  unsigned __int128 b = (y > 0) ? y : -y;
  return (a * b) / b != a;
}
 */
} // namespace common

template <auto value>
constexpr const auto Const = value;
constexpr const auto pi = Const<3>;

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
