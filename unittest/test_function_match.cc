// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/03/07.
//

#include "function_match.hh"
#include <gtest/gtest.h>

namespace test {
class FunctionMatchTest : public ::testing::Test {};
TEST_F(FunctionMatchTest, test_partial_order_relation) {
  PartialOrderRelation<int> por;
  por.Put(1, 2, 3, 4);
  ASSERT_TRUE(por.Contains(1, 2));
  ASSERT_TRUE(por.Contains(1, 3));
  ASSERT_TRUE(por.Contains(1, 4));
  ASSERT_FALSE(por.Contains(2, 4));
  ASSERT_FALSE(por.Contains(4, 1));
}

TEST_F(FunctionMatchTest, test_partial_order_relation_fail) {
  PartialOrderRelation<int> por;
  por.Put(1, 2, 3, 4);
  ASSERT_ANY_THROW(por.Put(1,2));
  ASSERT_ANY_THROW(por.Put(4,1));
}

} // namespace test

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}