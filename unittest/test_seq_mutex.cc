// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/9.
//

#include <gtest/gtest.h>

#include <iostream>
#include <mutex>
#include <random>
#include <shared_mutex>
#include <thread>

#include "sequential_mutex.h"
namespace test {
struct TestSeqMutex : ::testing::Test {};
TEST_F(TestSeqMutex, testTrivial) {
    std::vector<size_t> orders{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    concurrency::seq_mutex mutex(orders);
    mutex.set_thread_id(0);
    {
        std::shared_lock read_lock(mutex);
        std::cout << "read_lock" << std::endl;
    }
    {
        std::unique_lock write_lock(mutex);
        std::cout << "read_lock" << std::endl;
    }
}
TEST_F(TestSeqMutex, testMultiThread) {
    std::vector<size_t> orders{0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
    std::vector<size_t> result;
    std::atomic<size_t> count(0);
    concurrency::seq_mutex mutex(orders);
    std::vector<std::thread> threads;
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back([i, &mutex, &result, &count]() {
            mutex.set_thread_id(i);
            std::random_device dev;
            std::uniform_int_distribution<int> dist(10, 500);
            for (int k = 0; k < 4; k++) {
                std::unique_lock lock(mutex);
                result.emplace_back(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(dist(dev)));
            }
        });
    }
    for (int i = 0; i < 2; ++i) {
        threads[i].join();
    }
    ASSERT_TRUE(result.size() == 8);
    for (int i = 0; i < result.size(); ++i) {
        std::cout << "i=" << i << ": " << orders[i] << std::endl;
        ASSERT_EQ(result[i], orders[i]);
    }
}
} // namespace test

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}