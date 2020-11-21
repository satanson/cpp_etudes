// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com

//
// Created by grakra on 20-6-28.
//

#include <gtest/gtest.h>
#include <concurrent/list.hh>
#include <thread>
#include <vector>
#include <functional>
#include <random>

namespace com {
namespace grakra {
namespace concurrent {

//typedef void(*ThreadFunc)(MichaelList &, size_t, int);
//typedef void(*CheckFunc)(MichaelList &);

using ThreadFunc = std::function<void(MichaelList &, size_t, int)>;
using CheckFunc = std::function<void(MichaelList &)>;
typedef void(*SameKeyThreadFunc)(std::shared_ptr<MichaelList>, uint32_t, size_t);
typedef void(*SameKeyCheckFunc)(std::shared_ptr<MichaelList>, uint32_t);

ThreadFunc generate_thread_func(size_t rounds, size_t thread_nr, size_t key_nr) {
  auto f = [=](MichaelList &list, size_t n, int start) {
    std::vector<int> keys;
    keys.reserve(key_nr);
    for (auto i = 0; i < key_nr; ++i) {
      keys.push_back(start + i * thread_nr);
    }
    std::random_shuffle(keys.begin(), keys.end());
    for (auto r = 0; r < rounds; ++r) {
      for (auto k: keys) {
        uint32_t value = 0;
        auto node = std::make_unique<NodeType>(k, k * k);
        if (list.Search(k, value)) {
          ASSERT_TRUE(list.Remove(k));
        }
        ASSERT_TRUE(list.Insert(node.release()));
        ASSERT_TRUE(list.Search(k, value));
        ASSERT_EQ(value, k * k);
        ASSERT_TRUE(list.Remove(k));

        node = std::make_unique<NodeType>(k, k * k - 1);
        ASSERT_TRUE(list.Insert(node.release()));
        node = std::make_unique<NodeType>(k, k * k + 1);
        ASSERT_FALSE(list.Insert(node.get()));
        ASSERT_TRUE(list.Search(k, value));
        ASSERT_EQ(value, k * k - 1);
        ASSERT_TRUE(list.Remove(k));
        ASSERT_FALSE(list.Search(k, value));
        ASSERT_TRUE(list.Insert(node.release()));
        ASSERT_TRUE(list.Search(k, value));
        ASSERT_EQ(value, k * k + 1);
        ASSERT_TRUE(list.Remove(k));
        ASSERT_FALSE(list.Remove(k));
        ASSERT_FALSE(list.Search(k, value));
        node = std::make_unique<NodeType>(k, k);

        ASSERT_TRUE(list.Insert(node.release()));
      }
    }
  };
  return f;
}

CheckFunc generate_check_func(size_t thread_nr, size_t key_nr) {
  auto f = [=](MichaelList &list) {
    uint32_t value;
    for (uint32_t key = 0; key < thread_nr * key_nr; ++key) {
      ASSERT_TRUE(list.Search(key, value));
      ASSERT_EQ(value, key);
      ASSERT_TRUE(list.Remove(key));
    }
    ASSERT_TRUE(list.IsEmpty());
  };
  return f;
}

class TestList : public ::testing::Test {
 public:
  void multi_thread_run(
      MichaelList &list, size_t thread_nr, ThreadFunc thread_func, CheckFunc check_func) {
    assert(thread_nr > 0);
    std::vector<std::thread> threads;
    threads.reserve(thread_nr);
    for (auto i = 0; i < thread_nr; ++i) {
      threads.push_back(std::thread(thread_func, std::ref(list), thread_nr, i));
    }
    for (auto &thread: threads) {
      thread.join();
    }
    check_func(list);
  }

  void multi_thread_run_same_key(
      std::shared_ptr<MichaelList> list,
      size_t thread_nr,
      size_t rounds,
      uint32_t key,
      SameKeyThreadFunc thread_func,
      SameKeyCheckFunc check_func) {

    std::vector<std::thread> threads;
    threads.reserve(thread_nr);
    for (auto i = 0; i < thread_nr; ++i) {
      threads.push_back(std::thread(thread_func, list, key, rounds));
    }
    for (auto &thread: threads) {
      thread.join();
    }
    check_func(list, key);
  }
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
  uint32_t value;
  for (int i = 0; i < 20; ++i) {
    ASSERT_TRUE(list.Search(i, value));
    ASSERT_EQ(value, i * i);
    ASSERT_FALSE(list.Search(i + 20, value));
  }

  GTEST_LOG_(INFO) << list.ToString();
  for (int i = 0; i < 20; ++i) {
    if (i % 3 == 0 || i % 2 == 0) {

      GTEST_LOG_(INFO) << "Remove: key=" << i;
      ASSERT_TRUE(list.Remove(i));
      ASSERT_FALSE(list.Search(i, value));
    } else {
      ASSERT_TRUE(list.Search(i, value));
      ASSERT_EQ(value, i * i);
    }
  }
  GTEST_LOG_(INFO) << list.ToString();
}

TEST_F(TestList, testMultiThreadAccessDifferentKeys) {
  MichaelList list;
  auto thread_nr = 100;
  auto key_nr = 10;
  auto thread_func = generate_thread_func(100, thread_nr, key_nr);
  auto check_func = generate_check_func(thread_nr, key_nr);
  this->multi_thread_run(list, thread_nr, thread_func, check_func);
}

void same_key_thread_func(std::shared_ptr<MichaelList> list, uint32_t key, size_t rounds) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<size_t> rand(0, 2);
  for (auto r = 0; r < rounds; ++r) {
    auto i = rand(gen);
    //GTEST_LOG_(INFO)<<"thread#"<<start<<": round#"<<r<<": "<<i;
    //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    switch (i) {
      case 0: {
        auto node = std::make_unique<NodeType>(key, key * key);
        list->Insert(node.release());
        break;
      }
      case 1: {
        // TODO: Remove crashes
        // list->Remove(key);
        break;
      }
      case 2: {
        uint32_t value = 0;
        list->Search(key, value);
      }
    }
  }
  auto node = std::make_unique<NodeType>(key, key * key);
  list->Insert(node.release());
}

void same_key_check_func(std::shared_ptr<MichaelList> list, uint32_t key) {
  auto value = uint32_t(0);
  ASSERT_TRUE(list->Search(key, value));
  ASSERT_EQ(value, key * key);
  ASSERT_TRUE(list->Remove(key));
  ASSERT_FALSE(list->Search(key, value));
}
// FAIL: writing into de-allocated object cause __malloc_arena_thread_freeres crashes in glibc when pthread clean thread
// local storage before exit.
TEST_F(TestList, testMultiThreadAccessSameKey) {
  auto list = std::make_shared<MichaelList>();
  auto thread_nr = 2;
  auto rounds = 1000;
  auto key = uint32_t(10);
  this->multi_thread_run_same_key(list, thread_nr, rounds, key, same_key_thread_func, same_key_check_func);
  key = key + 1;
  this->multi_thread_run_same_key(list, thread_nr, rounds, key, same_key_thread_func, same_key_check_func);
  key = key - 1;
  this->multi_thread_run_same_key(list, thread_nr, rounds, key, same_key_thread_func, same_key_check_func);
  for (auto i = uint32_t(0); i < 1000; ++i) {
    auto node = std::make_unique<NodeType>(i, 2 * i + 1);
    list->Insert(node.release());
  }
  key = 2000;
  this->multi_thread_run_same_key(list, thread_nr, rounds, key, same_key_thread_func, same_key_check_func);
}

thread_local MarkPtrType *p = nullptr;
thread_local MarkPtrType *q = nullptr;
// access de-allocated object cause crashes in __malloc_arena_thread_freeres of glibc
// MichaelList::Remove has the same issue
TEST_F(TestList, testThreadLocal) {
  std::vector<std::thread> threads;
  threads.reserve(2);
  for (int i = 0; i < 2; i++) {
    threads.push_back(std::thread([]() {
      GTEST_LOG_(INFO) << p;
      for (int n = 0; n < 10000; ++n) {
        auto p0 = std::make_unique<NodeType>(1, 1);
        p = &p0->next;
        delete p0.release();
        *p = MarkPtrType(0, 1, 0);
        q = list_next(*p);
      }
    }));
  }
  for (auto &thd:threads) {
    thd.join();
  }
}
} // concurrent
} // grakra
} // com

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
