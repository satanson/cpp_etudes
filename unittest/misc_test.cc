//
// Created by grakra on 2020/9/23.
//

#include <gtest/gtest.h>
#include <iostream>
using namespace std;
class MiscTest : public ::testing::Test {

};
namespace abc{
class MiscTest{};
}
TEST_F(MiscTest, testTypeId){
  cout<<typeid(int).name()<<endl;
  cout<<typeid(0).name()<<endl;
  cout<<typeid(double).name()<<endl;
  cout<<typeid(0.1f).name()<<endl;
  cout<<typeid(0.1).name()<<endl;
  cout<<typeid(MiscTest).name()<<endl;
  cout<<typeid(abc::MiscTest).name()<<endl;
  cout<<typeid(int*).name()<<endl;
  cout<<typeid(int**).name()<<endl;
  cout<<typeid(int***).name()<<endl;
  cout<<typeid(const int*).name()<<endl;
  cout<<typeid(volatile int**).name()<<endl;
  cout<<typeid(const volatile int***).name()<<endl;
  cout<<typeid(int&).name()<<endl;
  cout<<sizeof(long long)<<endl;
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
