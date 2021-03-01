// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/03/01.
//

#include <gtest/gtest.h>
#include "meta_macro.hh"

#define M_DEF_STMT_1(type, a) \
type a = 1;

#define M_DEF_STMT_2(type, a, b) \
type a = 1; \
type b = 1;

#define M_DEF_STMT_3(type, a, b, c) \
type a = 1; \
type b = 1; \
type c = 1;

#define M_DEF_STMT_4(type, a1, a2, a3, a4) \
M_DEF_STMT_2(type, a1, a2)              \
M_DEF_STMT_2(type, a3, a4)

#define M_DEF_STMT_5(type, a1, a2, a3, a4, a5) \
M_DEF_STMT_4(type, a1, a2, a3, a4)              \
M_DEF_STMT_1(type, a5)

#define M_DEF_STMT_6(type, a1, a2, a3, a4, a5, a6) \
M_DEF_STMT_4(type, a1, a2, a3, a4)              \
M_DEF_STMT_2(type, a5, a6)

#define M_DEF_STMT_7(type, a1, a2, a3, a4, a5, a6, a7) \
M_DEF_STMT_4(type, a1, a2, a3, a4)              \
M_DEF_STMT_3(type, a5, a6,a7)

#define M_DEF_STMT_8(type, a1, a2, a3, a4, a5, a6, a7, a8) \
M_DEF_STMT_4(type, a1, a2, a3, a4)              \
M_DEF_STMT_4(type, a5, a6,a7, a8)

#define M_DEF_STMT_9(type, a1, a2, a3, a4, a5, a6, a7, a8, a9) \
M_DEF_STMT_8(type, a1, a2, a3, a4,a5,a6,a7,a8)              \
M_DEF_STMT_1(type, a9)

#define M_DEF_STMT_10(type, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
M_DEF_STMT_8(type, a1, a2, a3, a4,a5,a6,a7,a8)              \
M_DEF_STMT_2(type, a9, a10)

#define M_DEF_STMT(type, ...) META_MACRO_SELECT(M_DEF_STMT_, 1, type, __VA_ARGS__)

namespace test {
class MetaMacroTest : public ::testing::Test {};

TEST_F(MetaMacroTest, test_va_args_num) {
    std::vector<int> nums = {
        META_MACRO_VA_ARGS_NUM(a1),
        META_MACRO_VA_ARGS_NUM(a1, b2),
        META_MACRO_VA_ARGS_NUM(a1, b2, c3),
        META_MACRO_VA_ARGS_NUM(a1, b2, c3, d4),
        META_MACRO_VA_ARGS_NUM(a1, b2, c3, d4, e5),
        META_MACRO_VA_ARGS_NUM(a1, b2, c3, d4, e5, f6),
        META_MACRO_VA_ARGS_NUM(a1, b2, c3, d4, e5, f6, g7),
        META_MACRO_VA_ARGS_NUM(a1, b2, c3, d4, e5, f6, g7, h8),
        META_MACRO_VA_ARGS_NUM(a1, b2, c3, d4, e5, f6, g7, h8, i9),
        META_MACRO_VA_ARGS_NUM(a1, b2, c3, d4, e5, f6, g7, h8, i9, j10),
    };
    for (int i = 0; i < nums.size(); ++i) {
        std::cout << i << ": " << nums[i] << std::endl;
        ASSERT_EQ(nums[i], i + 1);
    }
    M_DEF_STMT(int, d, e)
    M_DEF_STMT(int, a1, a2, a3, a4)
    M_DEF_STMT(float, b1, b2, b3, b4, b5, b6, b7, b8, b9, b10)
}

TEST_F(MetaMacroTest, test_macro_1) {
    M_DEF_STMT(int, a1);
    M_DEF_STMT(int, b1, b2, b3);
    M_DEF_STMT(int, c1, c2, c3, c4, c5, c6, c7, c8, c9, c10);
    ASSERT_EQ(c10, 1);
    ASSERT_EQ(b2, 1);
    ASSERT_EQ(a1, 1);
}

#define M_MODIFIER_DEF_STMT_1(modifier, type, a1) \
modifier type a1 = type(0);

#define M_MODIFIER_DEF_STMT_2(modifier, type, a1, ...) \
M_MODIFIER_DEF_STMT_1(modifier, type, a1) \
M_MODIFIER_DEF_STMT_1(modifier, type, __VA_ARGS__)

#define M_MODIFIER_DEF_STMT_3(modifier, type, a1, ...) \
M_MODIFIER_DEF_STMT_1(modifier, type, a1) \
M_MODIFIER_DEF_STMT_2(modifier, type, __VA_ARGS__)

#define M_MODIFIER_DEF_STMT_4(modifier, type, a1, a2, ...) \
M_MODIFIER_DEF_STMT_2(modifier,type, a1,a2) \
M_MODIFIER_DEF_STMT_2(modifier,type, __VA_ARGS__)

#define M_MODIFIER_DEF_STMT_5(modifier, type, a1, ...) \
M_MODIFIER_DEF_STMT_1(modifier, type, a1) \
M_MODIFIER_DEF_STMT_4(modifier, type,__VA_ARGS__)

#define M_MODIFIER_DEF_STMT_6(modifier, type, a1, a2, ...) \
M_MODIFIER_DEF_STMT_2(modifier, type, a1,a2) \
M_MODIFIER_DEF_STMT_4(modifier, type, __VA_ARGS__)

#define M_MODIFIER_DEF_STMT_7(modifier, type, a1, ...) \
M_MODIFIER_DEF_STMT_1(modifier,type, a1) \
M_MODIFIER_DEF_STMT_6(modifier, type, __VA_ARGS__)

#define M_MODIFIER_DEF_STMT_8(modifier, type, a1, a2, a3, a4, ...) \
M_MODIFIER_DEF_STMT_4(modifier, type, a1,a2,a3,a4) \
M_MODIFIER_DEF_STMT_4(modifier, type, __VA_ARGS__)

#define M_MODIFIER_DEF_STMT_9(modifier, type, a1, ...) \
M_MODIFIER_DEF_STMT_1(modifier,type, a1) \
M_MODIFIER_DEF_STMT_8(modifier,type, __VA_ARGS__)

#define M_MODIFIER_DEF_STMT_10(modifier, type, a1, a2, ...) \
M_MODIFIER_DEF_STMT_2(modifier,type, a1,a2) \
M_MODIFIER_DEF_STMT_8(modifier,type,__VA_ARGS__)

#define M_MODIFIER_DEF_STMT(modifier, type, ...) META_MACRO_SELECT(M_MODIFIER_DEF_STMT_,2,modifier, type, __VA_ARGS__)

TEST_F(MetaMacroTest, test_macro_2) {
    M_MODIFIER_DEF_STMT(const, int, a1);
    M_MODIFIER_DEF_STMT(const, int, b1, b2);
    M_MODIFIER_DEF_STMT(const, int, c1, c2, c3);
    M_MODIFIER_DEF_STMT(const, int, d1, d2, d3, d4);
    M_MODIFIER_DEF_STMT(const, int, e1, e2, e3, e4, e5, e6, e7, e8, e9, e10);
}

#define DEF_FOOBAR_1(pair, n, m1) pair(n,m1)
#define DEF_FOOBAR_2(pair, n, m1, m2) pair(n,m1),pair(n,m2)
#define DEF_FOOBAR_4(pair, n, m1, m2, m3, m4) pair(n,m1),pair(n,m2),pair(n,m3),pair(n,m4)
#define DEF_FOOBAR_3(pair, n, ...) META_MACRO_CASE_DEF(DEF_FOOBAR_, 2, pair, n, ##__VA_ARGS__)
#define DEF_FOOBAR_5(pair, n, ...) META_MACRO_CASE_DEF(DEF_FOOBAR_, 2, pair, n, ##__VA_ARGS__)
#define DEF_FOOBAR_6(pair, n, ...) META_MACRO_CASE_DEF(DEF_FOOBAR_, 2, pair, n, ##__VA_ARGS__)
#define DEF_FOOBAR_7(pair, n, ...) META_MACRO_CASE_DEF(DEF_FOOBAR_, 2, pair, n, ##__VA_ARGS__)
#define DEF_FOOBAR_8(pair, n, ...) META_MACRO_CASE_DEF(DEF_FOOBAR_, 2, pair, n, ##__VA_ARGS__)
#define DEF_FOOBAR_9(pair, n, ...) META_MACRO_CASE_DEF(DEF_FOOBAR_, 2, pair, n, ##__VA_ARGS__)
#define DEF_FOOBAR_10(pair, n, ...) META_MACRO_CASE_DEF(DEF_FOOBAR_, 2, pair, n, ##__VA_ARGS__)

#define DEF_FOOBAR(pair, n, ...) META_MACRO_SELECT(DEF_FOOBAR_, 2, pair, n, ##__VA_ARGS__)

TEST_F(MetaMacroTest, test_macro_3) {
    using pair_int_int = std::pair<int, int>;
    std::vector<std::pair<int, int>> pairs = {
        DEF_FOOBAR(pair_int_int, 1, 1),
        DEF_FOOBAR(pair_int_int, 2, 1, 2),
        DEF_FOOBAR(pair_int_int, 3, 1, 2, 3),
        DEF_FOOBAR(pair_int_int, 4, 1, 2, 3, 4),
        DEF_FOOBAR(pair_int_int, 5, 1, 2, 3, 4, 5),
        DEF_FOOBAR(pair_int_int, 6, 1, 2, 3, 4, 5, 6),
        DEF_FOOBAR(pair_int_int, 7, 1, 2, 3, 4, 5, 6, 7),
        DEF_FOOBAR(pair_int_int, 8, 1, 2, 3, 4, 5, 6, 7, 8),
        DEF_FOOBAR(pair_int_int, 9, 1, 2, 3, 4, 5, 6, 7, 8, 9),
        DEF_FOOBAR(pair_int_int, 10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10),
    };
    for (int i=1; i <= 10; ++i){
        for (int j=1; j <= i; ++j){
            int k=((i-1)*i/2 + j)-1;
            auto& p=pairs[k];
            std::cout<<"k="<<k<<", (i="<<i<<", j="<<j<<"), (p.first="<<p.first<<", p.second="<<p.second<<")"<<std::endl;
            ASSERT_EQ(i, p.first);
            ASSERT_EQ(j, p.second);
        }
    }
}

//DEF_FOOBAR_7(pair,2,1,2,3,4,5,6,7)

}//namespace test

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}