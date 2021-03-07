// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git
//
// Created by grakra on 2021/1/9.
//

#include <delegate.hh>
#include <gtest/gtest.h>
#include <immintrin.h>
#include <memory>
using namespace std;
class DelegationTest : public ::testing::Test {};
TEST_F(DelegationTest, test0) {
  std::shared_ptr<AbsReader> delegation = std::make_shared<DelegateReader>();
  std::shared_ptr<AbsReader> aReader =
      std::make_shared<TypeAReader>(delegation.get(), [](AbsReader *reader) {});
  std::shared_ptr<AbsReader> bReader = std::make_shared<TypeBReader>();
  auto deleter = [](AbsReader *reader) {};
  ReaderObj aReadObj(aReader.get(), deleter);
  ReaderObj bReadObj(bReader.get(), deleter);
  ASSERT_EQ(aReadObj->size(), 100);
  ASSERT_EQ(bReadObj->size(), 99);
}
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
