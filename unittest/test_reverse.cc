// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/9.
//

#include <gtest/gtest.h>
#include <reverse.hh>

namespace test {
class TestReverse : public testing::Test {};
TEST_F(TestReverse, testSIMD) {
  std::vector<std::string> cases = {"", "abcd",
                                    "abcd_efgh_igkl_mnop_qrst_uvwx_yz"};

  for (auto &c : cases) {
    auto begin = c.data();
    auto end = begin + c.size();
    std::string s1, s2;
    s1.resize(c.size());
    s2.resize(c.size());
    ascii_reverse<false>(begin, end, s1.data());
    ascii_reverse<true>(begin, end, s2.data());
    std::cout << "s1=" << s1 << std::endl;
    std::cout << "s2=" << s2 << std::endl;
  }
}
TEST_F(TestReverse, testutf8) {
  std::vector<std::string> cases = {
      "",
      "abcd",
      "abcd_efgh_igkl_mnop_qrst_uvwx_yz"
      "abcd牛马龙翔",
      "abcd博学笃志efg切问静思",
      "abcd_efgh_三十年终生牛马igkl六十年_mnop_qrst_uvwx_yz",
      "三十年终生牛马，六十年诸佛龙象"};

  for (auto &c : cases) {
    auto begin = c.data();
    auto end = begin + c.size();
    std::string s1, s2;
    s1.resize(c.size());
    utf8_reverse_per_slice(begin, end, s1.data());
    std::cout << "s1=" << s1 << std::endl;
  }
}

TEST_F(TestReverse, testReverse) {
  std::string abc("abcdefg");
  std::string cba(abc.rbegin(), abc.rend());
  std::cout << cba << std::endl;
}
} // namespace test

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
