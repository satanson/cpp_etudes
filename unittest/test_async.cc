// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/07/21.
//

#include <cstdlib>
#include <folly/MPMCQueue.h>
#include <future>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <random>
#include <thread>
using namespace std;
class AsyncTest : public ::testing::Test {};

TEST_F(AsyncTest, testPromise) {
  std::promise<int> prom;
  auto future = prom.get_future();
  std::thread thd(
      [](promise<int> prom) {
        std::this_thread::sleep_for(50ms);
        prom.set_value(10);
        std::cout << "prom.set_value" << std::endl;
      },
      std::move(prom));
  auto a = future.get();
  std::cout << "future.get()=" << a << std::endl;
  ASSERT_EQ(a, 10);
  thd.detach();
}

TEST_F(AsyncTest, testPromise2) {
  std::promise<int> prom;
  auto future = prom.get_future();
  std::thread thd(
      [](promise<int> prom) {
        std::this_thread::sleep_for(50ms);
        prom.set_value(10);
        std::cout << "prom.set_value" << std::endl;
      },
      std::move(prom));
  auto a = future.get();
  std::cout << "future.get()=" << a << std::endl;
  ASSERT_EQ(a, 10);
  thd.detach();
}

TEST_F(AsyncTest, testMPMCQueue) {
  auto q = std::make_shared<folly::MPMCQueue<int>>(1024);
  std::vector<std::thread> threads;
  std::atomic<bool> cancel(false);
  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([&q, &cancel](){
      while(!cancel.load()){
        int e;
        q->blockingRead(e);
        std::cout << "thread_id="<<std::this_thread::get_id() <<", e="<<e<<std::endl;
      }
    });
  }
  for (int i=0; i<10; ++i){
    q->write(i+1);
  }
  std::this_thread::sleep_for(std::chrono::seconds(1));
  cancel.store(true);
  for (int i=0; i<10;++i){
    q->write(-1);
  }
  for (int i=0; i<10;++i){
    threads[i].join();
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}