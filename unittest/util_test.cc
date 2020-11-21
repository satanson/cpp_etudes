// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com

//
// Created by grakra on 2020/7/2.
//
#include<gtest/gtest.h>
#include<util/bits_op.hh>
#include <random>
namespace com {
namespace grakra {
namespace util {
class TestUtil : public ::testing::Test {

};
TEST_F(TestUtil, testReverseBits) {
  GTEST_LOG_(INFO) << reverse_bits(0x1);
  GTEST_LOG_(INFO) << reverse_bits(0x80000000);
  GTEST_LOG_(INFO) << reverse_bits(0xf0000000);
  std::vector<uint32_t> nums{
      0, 0x1, 0x9, 0xff, 0x11, 0x99, 0xff, 0x1111, 0x9999, 0xffff,
      0x80000000, 0x90000000, 0xf0000000, 0x88000000, 0x99000000,
      0xff000000, 0xffff0000, 0xffffffff, 0x99999999, 0x11111111,
      0x88888888, 0x55555555, 0x59595959};
  for (auto n: nums) {
    ASSERT_EQ(n, reverse_bits(reverse_bits(n)));
  }
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint32_t> rand(0, UINT32_MAX);
  for (auto i = 0; i < 10000; ++i) {
    auto n = rand(gen);
    ASSERT_EQ(n, reverse_bits(reverse_bits(n)));
  }
}

} // namespace com
} // namespace grakra
} // namespace util

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
