// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 20-6-27.
//

#include <gtest/gtest.h>

#include <atomic>
#include <foobar.hh>
namespace test {
class GTestDemo : public testing::Test {};
TEST_F(GTestDemo, assertion) {
    ASSERT_EQ(1, 1);
    ASSERT_TRUE(true);
    GTEST_LOG_(INFO) << "ok";
}

TEST_F(GTestDemo, foobar) {
    ASSERT_EQ(add(1, 2), 3);
    GTEST_LOG_(INFO) << "1+2=" << add(1, 2);
}

TEST_F(GTestDemo, atomic) {
    std::atomic<void*> a(nullptr);
    void* b = a;
    a.compare_exchange_strong(b, (void*)20);
}
} // namespace test

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
