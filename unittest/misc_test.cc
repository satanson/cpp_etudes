//
// Created by grakra on 2020/9/23.
//

#include <gtest/gtest.h>
#include <iostream>
using namespace std;
class MiscTest : public ::testing::Test {

};
namespace abc {
class MiscTest {};
}
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

Str returnStr(std::string const &a) {
  return Str(a);
}

Str &&returnStrRvalueRef(Str &a) {
  return static_cast<Str &&>(a);
}

struct A {
  int a;
  int b;
};

template<typename ...Args>
void print0(Args&& ... args) {
  (std::cout<<...<<std::forward<Args>(args))<<std::endl;
}

TEST_F(MiscTest, testRValueReference) {
  print0(1,2,4,5,"abc");
}

TEST_F(MiscTest, floatAdd) {
  float a = 0.3f;
  float b = 0;
  for (int i=0; i< 1000'0000;++i){
    b+=a;
  }
  std::cout<<b<<std::endl;
}

TEST_F(MiscTest, tuple){
}

template<bool abc, typename F, typename... Args>
int foobar(int a, F f, Args&&... args){
  if constexpr (abc){
    return a * f(std::forward<Args>(args)...);
  } else {
    return a + f(std::forward<Args>(args)...);
  }
}
TEST_F(MiscTest, foobar){
  auto c=foobar<true>(10, [](int a, int b){return a+b;}, 10,20);
  auto d=foobar<false>(10, [](int a, int b){return a+b;}, 10,20);
  std::cout<<"c="<<c<<", d="<<d<<std::endl;
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
