// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/07/21.
//

#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <random>
#include <cstdlib>
#include <thread>
#include <future>
using namespace std;
class AsyncTest : public ::testing::Test {};

TEST_F(AsyncTest, testPromise){
  std::promise<int> prom;
  auto future = prom.get_future();
  std::thread thd([](promise<int> prom){
    std::this_thread::sleep_for(50ms);
    prom.set_value(10);
    std::cout<<"prom.set_value" <<std::endl;
  }, std::move(prom));
  auto a = future.get();
  std::cout<<"future.get()="<<a<<std::endl;
  ASSERT_EQ(a,10);
  thd.detach();
}
int main(int argc, char**argv){
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}