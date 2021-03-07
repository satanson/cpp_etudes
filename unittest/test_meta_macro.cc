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
#define DEF_FOOBAR_CTOR(a, b) std::pair<int, int>(a, b)
#define DEF_FOOBAR(n, ...)                                                     \
  DEF_BINARY_RELATION_ENTRY_SEP_COMMA(1, DEF_FOOBAR_CTOR, n, ##__VA_ARGS__)

TEST_F(MetaMacroTest, test_macro_3) {
    using pair_int_int = std::pair<int, int>;
    std::vector<std::pair<int, int>> pairs = {
        DEF_FOOBAR(1, 1),
        DEF_FOOBAR(2, 1, 2),
        DEF_FOOBAR(3, 1, 2, 3),
        DEF_FOOBAR(4, 1, 2, 3, 4),
        DEF_FOOBAR(5, 1, 2, 3, 4, 5),
        DEF_FOOBAR(6, 1, 2, 3, 4, 5, 6),
        DEF_FOOBAR(7, 1, 2, 3, 4, 5, 6, 7),
        DEF_FOOBAR(8, 1, 2, 3, 4, 5, 6, 7, 8),
        DEF_FOOBAR(9, 1, 2, 3, 4, 5, 6, 7, 8, 9),
        DEF_FOOBAR(10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10),
    };
    for (int i = 1; i <= 10; ++i) {
        for (int j = 1; j <= i; ++j) {
            int k = ((i - 1) * i / 2 + j) - 1;
            auto &p = pairs[k];
            std::cout << "k=" << k << ", (i=" << i << ", j=" << j
                      << "), (p.first=" << p.first << ", p.second=" << p.second
                      << ")" << std::endl;
            ASSERT_EQ(i, p.first);
            ASSERT_EQ(j, p.second);
        }
    }
}

template<typename T1, typename T2> struct IsAssignable {
    static constexpr bool value = false;
};

#define IS_ASSIGNABLE_CTOR(a, b)                                               \
  template <> struct IsAssignable<a, b> { static constexpr bool value = true; };

template<typename T1, typename T2>
constexpr bool is_assignable = IsAssignable<T1, T2>::value;

#define IS_ASSIGNABLE(a, ...)                                                  \
  DEF_BINARY_RELATION_ENTRY_SEP_NONE(1, IS_ASSIGNABLE_CTOR, a, ##__VA_ARGS__)
#define IS_ASSIGNABLE_R(a, ...)                                                \
  DEF_BINARY_RELATION_ENTRY_SEP_SEMICOLON(0, IS_ASSIGNABLE_CTOR, a,            \
                                          ##__VA_ARGS__)

IS_ASSIGNABLE(int8_t, int16_t, int32_t, int64_t);
IS_ASSIGNABLE_R(double, float, int8_t, int16_t, int32_t, uint8_t, uint16_t,
                uint32_t);

TEST_F(MetaMacroTest, test_constexpr) {
    static_assert(is_assignable < int8_t, int16_t > );
    static_assert(!is_assignable < int16_t, int8_t > );
    static_assert(is_assignable < int8_t, int32_t > );
    static_assert(!is_assignable < int32_t, int8_t > );
    static_assert(is_assignable < int8_t, int64_t > );
    static_assert(!is_assignable < int64_t, int8_t > );
    static_assert(!is_assignable < uint64_t, uint8_t > );
    static_assert(is_assignable < float, double > );
    static_assert(!is_assignable < double, float > );
    std::cout << "TEST PASS" << std::endl;
}
enum type_enum {
    TYPE_INT8,
    TYPE_INT16,
    TYPE_INT32,
    TYPE_INT64,
    TYPE_UINT8,
    TYPE_UINT16,
    TYPE_UINT32,
    TYPE_UINT64,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_DECIMAL32,
    TYPE_DECIMAL64,
    TYPE_DECIMAL128,
};
enum type2_enum {
    TYPE2_INVALID,
    TYPE2_INT64,
    TYPE2_DOUBLE,
};

#define IS_ASSIGNABLE_ENTRY_CTOR(a, b)                                         \
  { a, b }
#define IS_ASSIGNABLE_ENTRY(a, ...)                                            \
  DEF_BINARY_RELATION_ENTRY_SEP_COMMA(1, IS_ASSIGNABLE_ENTRY_CTOR, a,          \
                                      ##__VA_ARGS__)
#define IS_ASSIGNABLE_ENTRY_R(a, ...)                                          \
  DEF_BINARY_RELATION_ENTRY_SEP_COMMA(0, IS_ASSIGNABLE_ENTRY_CTOR, a,          \
                                      ##__VA_ARGS__)

static std::unordered_map<type_enum, type2_enum> global_assignable_table{
    IS_ASSIGNABLE_ENTRY(TYPE_INT8, TYPE2_INT64),
    IS_ASSIGNABLE_ENTRY_R(TYPE2_INT64, TYPE_UINT8, TYPE_INT16, TYPE_UINT16,
                          TYPE_INT32, TYPE_UINT32, TYPE_INT64, TYPE_UINT64),
    IS_ASSIGNABLE_ENTRY_R(TYPE2_DOUBLE, TYPE_FLOAT, TYPE_DOUBLE),
};

TEST_F(MetaMacroTest, test_assignable_table) {
    type2_enum type2 = TYPE2_INVALID;
    std::vector<type_enum> type1_ints = {TYPE_INT8, TYPE_UINT8, TYPE_INT16,
        TYPE_UINT16, TYPE_INT32, TYPE_UINT32,
        TYPE_INT64, TYPE_UINT64};
    for (auto &type1 : type1_ints) {
        ASSERT_TRUE(global_assignable_table.count(type1) > 0);
        type2 = global_assignable_table[type1];
        ASSERT_EQ(type2, TYPE2_INT64);
    }

    std::vector<type_enum> type1_floats = {TYPE_FLOAT, TYPE_DOUBLE};
    for (auto &type1 : type1_floats) {
        ASSERT_TRUE(global_assignable_table.count(type1) > 0);
        type2 = global_assignable_table[type1];
        ASSERT_EQ(type2, TYPE2_DOUBLE);
    }
    ASSERT_TRUE(global_assignable_table.count(TYPE_DECIMAL32) == 0);
    ASSERT_TRUE(global_assignable_table.count(TYPE_DECIMAL64) == 0);
    ASSERT_TRUE(global_assignable_table.count(TYPE_DECIMAL128) == 0);
}

template<typename T1, typename T2, typename T3>
struct IsBinaryFunction {
    static constexpr bool value = false;
};
template<typename T1, typename T2, typename T3>
constexpr bool is_binary_function = IsBinaryFunction<T1, T2, T3>::value;

#define IS_BINARY_FUNCTION_CTOR(a, b, c) \
template <>                         \
struct IsBinaryFunction<a,b,c> {    \
static constexpr bool value = true;                                    \
};

#define IS_BINARY_FUNCTION(varg_pos, a, b, ...)                                \
  DEF_TERNARY_RELATION_ENTRY_SEP_NONE(IS_BINARY_FUNCTION_CTOR, varg_pos, a, b, \
                                      ##__VA_ARGS__)

IS_BINARY_FUNCTION(0, int, int, float, double, int8_t, int16_t);
IS_BINARY_FUNCTION(1, int, int, float, double, int8_t);
}//namespace test

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
