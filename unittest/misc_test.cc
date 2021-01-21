// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/9/23.
//

#include <gtest/gtest.h>
#include <guard.hh>
#include <immintrin.h>
#include <iostream>
#include <memory>
using namespace std;
class MiscTest : public ::testing::Test {};
namespace abc {
class MiscTest {};
} // namespace abc
TEST_F(MiscTest, testTypeId) {
  cout << typeid(int).name() << endl;
  cout << typeid(0).name() << endl;
  cout << typeid(double).name() << endl;
  cout << typeid(0.1f).name() << endl;
  cout << typeid(0.1).name() << endl;
  cout << typeid(MiscTest).name() << endl;
  cout << typeid(abc::MiscTest).name() << endl;
  cout << typeid(int *).name() << endl;
  cout << typeid(int **).name() << endl;
  cout << typeid(int ***).name() << endl;
  cout << typeid(const int *).name() << endl;
  cout << typeid(volatile int **).name() << endl;
  cout << typeid(const volatile int ***).name() << endl;
  cout << typeid(int &).name() << endl;
  cout << sizeof(long long) << endl;
}

struct Str {
  char *p;
  size_t n;
  explicit Str(string const &s) {
    cout << "Str::ctor invoked" << endl;
    this->n = s.size();
    this->p = new char[n];
    std::copy(s.begin(), s.end(), p);
  }

  std::string ToString() {
    std::string s(n, '0');
    std::copy(p, p + n, s.begin());
    return s;
  }

  Str(Str &&that) {
    std::cout << "move ctor" << endl;
    this->n = that.n;
    this->p = that.p;
    that.p = nullptr;
    that.n = 0;
  }

  ~Str() {
    cout << "Str::dtor invoked" << endl;
    if (p != nullptr) {
      delete p;
      p = nullptr;
    }
    n = 0;
  }
};

Str returnStr(std::string const &a) { return Str(a); }

Str &&returnStrRvalueRef(Str &a) { return static_cast<Str &&>(a); }

struct A {
  int a;
  int b;
};

template <typename... Args> void print0(Args &&... args) {
  (std::cout << ... << std::forward<Args>(args)) << std::endl;
}

TEST_F(MiscTest, testRValueReference) { print0(1, 2, 4, 5, "abc"); }

TEST_F(MiscTest, floatAdd) {
  float a = 0.3f;
  float b = 0;
  for (int i = 0; i < 1000'0000; ++i) {
    b += a;
  }
  std::cout << b << std::endl;
}

TEST_F(MiscTest, tuple) {}

template <bool abc, typename F, typename... Args>
int foobar(int a, F f, Args &&... args) {
  if constexpr (abc) {
    return a * f(std::forward<Args>(args)...);
  } else {
    return a + f(std::forward<Args>(args)...);
  }
}

template <bool is_abc> struct AA {
  ;
  static void evaluate() {
    if constexpr (is_abc) {
      std::cout << "is_abc" << std::endl;
    } else {
      std::cout << "!is_abc" << std::endl;
    }
  }
};

template <template <bool> typename F> void g(bool abc) {
  if (abc) {
    F<true>::evalute();
  } else {
    F<false>::evalute();
  }
}

template <template <typename, size_t...> typename Collector, size_t... Args>
std::shared_ptr<void> create_abc() {
  using Array = Collector<int, Args...>;
  return (std::shared_ptr<void>)std::make_shared<Array>();
}

TEST_F(MiscTest, foobar) {
  std::shared_ptr<void> a = create_abc<std::array, 10>();
}

template <typename T, typename = guard::Guard> struct AAA {
  static inline void apply() { std::cout << "AAA T" << std::endl; }
};
template <typename T> struct AAA<T, guard::TypeGuard<T, float, double>> {
  static inline void apply() {
    std::cout << "AAA float or double" << std::endl;
  }
};
template <typename T>
struct AAA<T, guard::TypeGuard<T, char, short, long, long long>> {
  static inline void apply() { std::cout << "AAA other int" << std::endl; }
};
template <> struct AAA<int, int> {
  static inline void apply() { std::cout << "AAA int" << std::endl; }
};

TEST_F(MiscTest, testConcept) {
  AAA<char>::apply();
  AAA<float>::apply();
  AAA<double>::apply();
  AAA<int>::apply();
  AAA<long>::apply();
  AAA<bool>::apply();
  AAA<unsigned int>::apply();
  AAA<unsigned long>::apply();
}
using FieldType = int32_t;
union ExtendFieldType {
  FieldType field_type;
  struct {
#if __BYTE_ORDER == LITTLE_ENDIAN
    int16_t type;
    int8_t precision;
    int8_t scale;
#else
    int8_t scale;
    int8_t precision;
    int16_t type;
#endif
  } extend;

  ExtendFieldType(FieldType field_type) : field_type(field_type) {}
  ExtendFieldType(FieldType field_type, int precision, int scale)
      : extend({.type = (int16_t)field_type,
                .precision = (int8_t)precision,
                .scale = (int8_t)scale}) {}
  ExtendFieldType(ExtendFieldType const &) = default;
  ExtendFieldType &operator=(ExtendFieldType &) = default;
  FieldType type() const { return extend.type; }
  int precision() const { return extend.precision; }
  int scale() const { return extend.scale; }
};

TEST_F(MiscTest, testExtendField) {
  auto field = ExtendFieldType(10, 27, 9);
  ASSERT_EQ(field.type(), 10);
  ASSERT_EQ(field.precision(), 27);
  ASSERT_EQ(field.scale(), 9);
}

template <typename Op> struct FunctorA {
  template <typename T, typename... Args>
  static inline T evaluate(const T &t, Args &&... args) {
    return Op::template evaluate(t, 'A', std::forward<Args>(args)...);
  }
};

template <typename Op> struct FunctorB {
  template <typename T, typename... Args>
  static inline T evaluate(const T &t, Args &&... args) {
    return Op::evaluate(t, 'B', std::forward<Args>(args)...);
  }
};

struct FunctorC {
  template <typename... Args>
  static inline std::string evaluate(std::string const &s, Args &&... args) {
    std::string result(s);
    ((result.append(std::to_string(std::forward<Args>(args)))), ...);
    return result;
  }
};

TEST_F(MiscTest, testEvaluate) {
  auto s = FunctorA<FunctorB<FunctorC>>::evaluate<std::string>("abc", 1, 2);
  std::cout << s << std::endl;
  const int arg0 = 100;
  const int arg1 = 999;
  const int &arg0_ref = arg0;
  const int &arg1_ref = arg1;
  auto s1 =
      FunctorA<FunctorB<FunctorC>>::evaluate<std::string>("abc", arg0, arg1);
  std::cout << s1 << std::endl;
  auto s2 = FunctorA<FunctorB<FunctorC>>::evaluate<std::string>("abc", arg0_ref,
                                                                arg1_ref);
  std::cout << s2 << std::endl;
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
