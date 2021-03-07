// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/7/3.
//

#include <concurrent/hash.hh>
#include <gtest/gtest.h>

namespace com {
namespace grakra {
namespace concurrent {
class TestHash : public testing::Test {};
TEST_F(TestHash, testSingleThread) {
  Hash hash(0x100000, 4);
  for (uint32_t key = 0; key < 1000; ++key) {
    // GTEST_LOG_(INFO) << "PUT: key=" << key;
    ASSERT_TRUE(hash.Put(key, key + 1));
  }
  ASSERT_EQ(hash.get_size(), 1000);
  uint32_t value;
  for (uint32_t key = 0; key < 1000; ++key) {
    ASSERT_TRUE(hash.Get(key, value));
    ASSERT_EQ(value, key + 1);
    // GTEST_LOG_(INFO) << "GET: key=" << key << ", value=" << value;
  }

  for (uint32_t key = 0; key < 1000; ++key) {
    // GTEST_LOG_(INFO) << "PUT: key=" << key;
    ASSERT_FALSE(hash.Put(key, key + 2));
  }
  ASSERT_EQ(hash.get_size(), 1000);

  for (uint32_t key = 0; key < 1000; ++key) {
    ASSERT_TRUE(hash.Get(key, value));
    ASSERT_EQ(value, key + 1);
    // GTEST_LOG_(INFO) << "GET: key=" << key << ", value=" << value;
  }
}
TEST_F(TestHash, testConstExpr) {
  GTEST_LOG_(INFO) << "SLOT_INDEX_OFFSET=" << SLOT_INDEX_SHIFT;
}
TEST_F(TestHash, testHashResizing) {
  auto f = [](size_t nr, size_t factor) {
    Hash hash(nr, factor);
    GTEST_LOG_(INFO) << "expect_max_size=" << hash.get_expect_max_size()
                     << ", load_factor=" << hash.get_load_factor()
                     << ", max_slot_nr=" << hash.get_max_slot_nr()
                     << ", level_nr=" << hash.get_level_nr();

    uint32_t max_key = hash.get_expect_max_size() + 10;
    size_t count = 0;
    for (auto key = uint32_t(0); key < max_key; ++key) {
      ++count;
      if (count % SLOT_INDEX_NR == 0) {
        GTEST_LOG_(INFO) << "Put " << count << " keys";
      }
      ASSERT_TRUE(hash.Put(key, 2 * key + 1));
    }

    uint32_t value = 0;
    for (auto key = uint32_t(0); key < max_key; ++key) {
      ASSERT_TRUE(hash.Get(key, value));
      ASSERT_EQ(value, 2 * key + 1);
    }
  };
  f(SLOT_INDEX_NR - 1, 1);
  f(SLOT_INDEX_NR, 1);
  f(SLOT_INDEX_NR + 1, 1);
  f(SLOT_INDEX_NR * 100, 1);
  // f((SLOT_INDEX_NR + 1) * SLOT_INDEX_NR, 1);
  // f(SLOT_INDEX_NR * SLOT_INDEX_NR * SLOT_INDEX_NR, 1);
}

} // namespace concurrent
} // namespace grakra
} // namespace com

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
