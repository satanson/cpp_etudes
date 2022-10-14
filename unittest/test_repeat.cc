// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/10.
//

#include <gtest/gtest.h>

#include <repeat.hh>

namespace test {
class TestRepeat : public ::testing::Test {};
TEST_F(TestRepeat, testFoobar) {
    for (int i = 1; i < 100; i++) {
        ASSERT_EQ(i, count_n(i));
        std::cout << i << ": " << count_n(i) << std::endl;
    }
    for (int i = 1; i < 100; i++) {
        int n = INT32_MAX - i;
        ASSERT_EQ(n, count_n(n));
        std::cout << n << ": " << count_n(n) << std::endl;
    }
}

TEST_F(TestRepeat, testRepeatString) {
    std::string s = "hello";
    for (int i = 1; i < 100; ++i) {
        std::cout << repeat_string_logn(s, i) << std::endl;
    }
}

TEST_F(TestRepeat, testSimdCopy) {
    std::string s(1, 'x');
    for (int i = 0; i < 100; ++i) {
        std::string x10 = repeat_string_logn_simd_memcpy_inline_2(s, 10);
        std::cout << "x10=" << x10 << std::endl;
    }
}

} // namespace test

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
