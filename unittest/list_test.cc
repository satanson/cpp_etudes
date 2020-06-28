//
// Created by grakra on 20-6-28.
//

#include <gtest/gtest.h>
#include <concurrent/list.hh>
namespace com {
namespace grakra {
namespace concurrent {
class TestList : public ::testing::Test {

};

TEST_F(TestList, testSimple) {
  MichaelList list;
  auto node1 = std::make_unique<NodeType>(1, 1);
  list.Push(node1.get());
  auto node2 = std::make_unique<NodeType>(2, 4);
  list.Push(node2.get());
  GTEST_LOG_(INFO) << list.ToString();

  auto node3 = std::make_unique<NodeType>(3, 9);
  list.Unshift(node3.get());
  auto node4 = std::make_unique<NodeType>(4, 16);
  list.Unshift(node4.get());
  GTEST_LOG_(INFO) << list.ToString();
  list.Shift();
  list.Pop();
  GTEST_LOG_(INFO) << list.ToString();
  list.Shift();
  list.Pop();
  GTEST_LOG_(INFO) << list.ToString();

  ASSERT_TRUE(list.IsEmpty());
}

TEST_F(TestList, testSingleThread) {
  auto list = MichaelList();
  for (int n = 0; n < 4; ++n) {
    for (int i = n; i < 20; i += 4) {
      auto node = std::make_unique<NodeType>(i, i * i);
      list.Insert(node.release());
    }
  }
  GTEST_LOG_(INFO) << list.ToString();
  int32_t value;
  for (int i = 0; i < 20; ++i) {
    ASSERT_TRUE(list.Search(i, value));
    ASSERT_EQ(value, i * i);
    ASSERT_FALSE(list.Search(i + 20, value));
  }

  for (int i = 0; i < 20; ++i) {
    if (i % 3 == 0 || i % 2 == 0) {
      ASSERT_TRUE(list.Remove(i));
      ASSERT_FALSE(list.Search(i, value));
    } else {
      ASSERT_TRUE(list.Search(i, value));
      ASSERT_EQ(value, i * i);
    }
  }
  GTEST_LOG_(INFO) << list.ToString();
}

} // concurrent
} // grakra
} // com

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}