//
// Created by grakra on 2020/7/3.
//

#include <gtest/gtest.h>
#include <concurrent/hash.hh>

namespace com {
namespace grakra {
namespace concurrent {
class TestHash : public testing::Test {

};
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

TEST_F(TestHash, testing){

}
} // namespace concurrent
} // namespace grakra
} // namespace com

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}