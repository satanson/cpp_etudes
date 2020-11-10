//
// Created by grakra on 2020/11/9.
//

#include <gtest/gtest.h>
#include <trim.hh>
namespace test {
class TestTrim:public testing::Test {

};
TEST_F(TestTrim, TestSkipLeadingSpaces){
  std::string spaces(100, ' ');
  std::string s = spaces + "abcd100" + spaces;
  const char* begin = s.data();
  const char* end = s.data()+s.size();
  auto  p = begin;
  p = skip_leading_spaces<true>(p, end);
  auto q = skip_trailing_spaces<true>(p, end);
  std::string s2(p,q);
  std::cout<<"x"<<s2<<"x"<<std::endl;
}
}

int main(int argc, char** argv){
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}