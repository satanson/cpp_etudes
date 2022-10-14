// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/03/01.
//

#pragma once

#define META_MACRO_SELECT_21ST(a1, a2, a3, a4, a5, b6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, \
                               a20, a21, ...)                                                                        \
    a21

#define META_MACRO_VA_ARGS_NUM_MINUS_0(...) \
    META_MACRO_SELECT_21ST(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_1(...) \
    META_MACRO_SELECT_21ST(1, 2, 3, 4, 5, 6, 7, 8, 9, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_2(...) \
    META_MACRO_SELECT_21ST(1, 2, 3, 4, 5, 6, 7, 8, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_3(...) \
    META_MACRO_SELECT_21ST(1, 2, 3, 4, 5, 6, 7, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_4(...) \
    META_MACRO_SELECT_21ST(1, 2, 3, 4, 5, 6, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_5(...) \
    META_MACRO_SELECT_21ST(1, 2, 3, 4, 5, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_6(...) \
    META_MACRO_SELECT_21ST(1, 2, 3, 4, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_7(...) \
    META_MACRO_SELECT_21ST(1, 2, 3, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_8(...) \
    META_MACRO_SELECT_21ST(1, 2, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_9(...) META_MACRO_SELECT_21ST(1, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM(...) META_MACRO_VA_ARGS_NUM_MINUS_0(__VA_ARGS__)

#define META_MACRO_SELECT_VA(name, num, ...) name##num(__VA_ARGS__)
#define META_MACRO_SELECT_VA_HELPER(name, num, ...) META_MACRO_SELECT_VA(name, num, ##__VA_ARGS__)
#define META_MACRO_SELECT(name, n, ...) \
    META_MACRO_SELECT_VA_HELPER(name, META_MACRO_VA_ARGS_NUM_MINUS_##n(__VA_ARGS__), ##__VA_ARGS__)

#define META_MACRO_3_PART0 1
#define META_MACRO_3_PART1 1
#define META_MACRO_3_PART2 1

#define META_MACRO_5_PART0 1
#define META_MACRO_5_PART1 2
#define META_MACRO_5_PART2 2

#define META_MACRO_6_PART0 1
#define META_MACRO_6_PART1 1
#define META_MACRO_6_PART2 4

#define META_MACRO_7_PART0 1
#define META_MACRO_7_PART1 2
#define META_MACRO_7_PART2 4

#define META_MACRO_8_PART0 2
#define META_MACRO_8_PART1 2
#define META_MACRO_8_PART2 4

#define META_MACRO_9_PART0 1
#define META_MACRO_9_PART1 4
#define META_MACRO_9_PART2 4

#define META_MACRO_10_PART0 2
#define META_MACRO_10_PART1 4
#define META_MACRO_10_PART2 4
#define META_MACRO_PART(n, m) META_MACRO_##n##_PART##m

#define META_MACRO_TAKE_0(...)
#define META_MACRO_TAKE_1(a1, ...) a1
#define META_MACRO_TAKE_2(a1, a2, ...) a1, a2
#define META_MACRO_TAKE_3(a1, a2, a3, ...) a1, a2, a3
#define META_MACRO_TAKE_4(a1, a2, a3, a4, ...) a1, a2, a3, a4
#define META_MACRO_TAKE_5(a1, a2, a3, a4, a5, ...) a1, a2, a3, a4, a5
#define META_MACRO_TAKE(n, ...) META_MACRO_TAKE_##n(__VA_ARGS__)

#define META_MACRO_DROP_0(...) __VA_ARGS__
#define META_MACRO_DROP_1(a1, ...) __VA_ARGS__
#define META_MACRO_DROP_2(a1, a2, ...) __VA_ARGS__
#define META_MACRO_DROP_3(a1, a2, a3, ...) __VA_ARGS__
#define META_MACRO_DROP_4(a1, a2, a3, a4, ...) __VA_ARGS__
#define META_MACRO_DROP_5(a1, a2, a3, a4, a5, ...) __VA_ARGS__
#define META_MACRO_DROP(n, ...) META_MACRO_DROP_##n(__VA_ARGS__)

#define META_MACRO_SEP_NONE_MARK
#define META_MACRO_SEP_COMMA_MARK ,
#define META_MACRO_SEP_SEMICOLON_MARK ;

#define META_MACRO_CASE_DEF_PART_HELPER2(name, ...) name(__VA_ARGS__)

#define META_MACRO_CASE_DEF_PART0_HELPER1(name, n, m0, m1, m2, ...)           \
    META_MACRO_CASE_DEF_PART_HELPER2(name, META_MACRO_TAKE(n, ##__VA_ARGS__), \
                                     META_MACRO_TAKE(m0, META_MACRO_DROP(n, ##__VA_ARGS__)))

#define META_MACRO_CASE_DEF_PART1_HELPER1(name, n, m0, m1, m2, ...)           \
    META_MACRO_CASE_DEF_PART_HELPER2(name, META_MACRO_TAKE(n, ##__VA_ARGS__), \
                                     META_MACRO_TAKE(m1, META_MACRO_DROP(m0, META_MACRO_DROP(n, ##__VA_ARGS__))))

#define META_MACRO_CASE_DEF_PART2_HELPER1(name, n, m0, m1, m2, ...)           \
    META_MACRO_CASE_DEF_PART_HELPER2(name, META_MACRO_TAKE(n, ##__VA_ARGS__), \
                                     META_MACRO_DROP(m1, META_MACRO_DROP(m0, META_MACRO_DROP(n, ##__VA_ARGS__))))

#define META_MACRO_CASE_DEF_DECOMPOSE_SEP_HELPER(name, sep, n, m, m0, m1, m2, ...)                 \
    META_MACRO_CASE_DEF_PART0_HELPER1(name##m0, n, m0, m1, m2, ##__VA_ARGS__)                      \
    META_MACRO_SEP_##sep META_MACRO_CASE_DEF_PART1_HELPER1(name##m1, n, m0, m1, m2, ##__VA_ARGS__) \
            META_MACRO_SEP_##sep                                                                   \
            META_MACRO_CASE_DEF_PART2_HELPER1(name##m2, n, m0, m1, m2, ##__VA_ARGS__)

#define META_MACRO_CASE_DEF_DECOMPOSE_HELPER(name, sep, n, m, m0, m1, m2, ...) \
    META_MACRO_CASE_DEF_DECOMPOSE_SEP_HELPER(name, sep, n, m, m0, m1, m2, ##__VA_ARGS__)

#define META_MACRO_CASE_DEF_HELPER(name, sep, n, m, ...)                                                \
    META_MACRO_CASE_DEF_DECOMPOSE_HELPER(name, sep, n, m, META_MACRO_PART(m, 0), META_MACRO_PART(m, 1), \
                                         META_MACRO_PART(m, 2), ##__VA_ARGS__)

#define META_MACRO_CASE_DEF(name, sep, n, ...) \
    META_MACRO_CASE_DEF_HELPER(name, sep, n, META_MACRO_VA_ARGS_NUM_MINUS_##n(__VA_ARGS__), ##__VA_ARGS__)

// macros for binary relation entry definition
#define DEF_BINARY_RELATION_ENTRY_1_VARY_1(ctor, a, b) ctor(a, b)

#define DEF_BINARY_RELATION_ENTRY_1_VARY_0(ctor, b, a) DEF_BINARY_RELATION_ENTRY_1_VARY_1(ctor, a, b)

#define DEF_BINARY_RELATION_ENTRY_1_VARY_HELPER(macro_name, ctor, a, b0) macro_name(ctor, a, b0)

#define DEF_BINARY_RELATION_ENTRY_1(varg_pos, ctor, sep, a, b0) \
    DEF_BINARY_RELATION_ENTRY_1_VARY_HELPER(DEF_BINARY_RELATION_ENTRY_1_VARY_##varg_pos, ctor, a, b0)

#define DEF_BINARY_RELATION_ENTRY_2_SEP_HELPER(varg_pos, ctor, sep, a, b0, b1) \
    DEF_BINARY_RELATION_ENTRY_1(varg_pos, ctor, sep, a, b0)                    \
    META_MACRO_SEP_##sep DEF_BINARY_RELATION_ENTRY_1(varg_pos, ctor, sep, a, b1)

#define DEF_BINARY_RELATION_ENTRY_2(varg_pos, ctor, sep, a, b0, b1) \
    DEF_BINARY_RELATION_ENTRY_2_SEP_HELPER(varg_pos, ctor, sep, a, b0, b1)

#define DEF_BINARY_RELATION_ENTRY_4_SEP_HELPER(varg_pos, ctor, sep, a, b0, b1, b2, b3) \
    DEF_BINARY_RELATION_ENTRY_2(varg_pos, ctor, sep, a, b0, b1)                        \
    META_MACRO_SEP_##sep DEF_BINARY_RELATION_ENTRY_2(varg_pos, ctor, sep, a, b2, b3)

#define DEF_BINARY_RELATION_ENTRY_4(varg_pos, ctor, sep, a, b0, b1, b2, b3) \
    DEF_BINARY_RELATION_ENTRY_4_SEP_HELPER(varg_pos, ctor, sep, a, b0, b1, b2, b3)

#define DEF_BINARY_RELATION_ENTRY_3(varg_pos, ctor, sep, a, ...) \
    META_MACRO_CASE_DEF(DEF_BINARY_RELATION_ENTRY_, sep, 4, varg_pos, ctor, sep, a, ##__VA_ARGS__)

#define DEF_BINARY_RELATION_ENTRY_5(varg_pos, ctor, sep, a, ...) \
    META_MACRO_CASE_DEF(DEF_BINARY_RELATION_ENTRY_, sep, 4, varg_pos, ctor, sep, a, ##__VA_ARGS__)

#define DEF_BINARY_RELATION_ENTRY_6(varg_pos, ctor, sep, a, ...) \
    META_MACRO_CASE_DEF(DEF_BINARY_RELATION_ENTRY_, sep, 4, varg_pos, ctor, sep, a, ##__VA_ARGS__)

#define DEF_BINARY_RELATION_ENTRY_7(varg_pos, ctor, sep, a, ...) \
    META_MACRO_CASE_DEF(DEF_BINARY_RELATION_ENTRY_, sep, 4, varg_pos, ctor, sep, a, ##__VA_ARGS__)

#define DEF_BINARY_RELATION_ENTRY_8(varg_pos, ctor, sep, a, ...) \
    META_MACRO_CASE_DEF(DEF_BINARY_RELATION_ENTRY_, sep, 4, varg_pos, ctor, sep, a, ##__VA_ARGS__)

#define DEF_BINARY_RELATION_ENTRY_9(varg_pos, ctor, sep, a, ...) \
    META_MACRO_CASE_DEF(DEF_BINARY_RELATION_ENTRY_, sep, 4, varg_pos, ctor, sep, a, ##__VA_ARGS__)

#define DEF_BINARY_RELATION_ENTRY_10(varg_pos, ctor, sep, a, ...) \
    META_MACRO_CASE_DEF(DEF_BINARY_RELATION_ENTRY_, sep, 4, varg_pos, ctor, sep, a, ##__VA_ARGS__)

#define DEF_BINARY_RELATION_ENTRY_SEP_VARY(varg_pos, ctor, sep, a, ...) \
    META_MACRO_SELECT(DEF_BINARY_RELATION_ENTRY_, 4, varg_pos, ctor, sep, a, ##__VA_ARGS__)

#define DEF_BINARY_RELATION_ENTRY_SEP_NONE(varg_pos, ctor, a, ...) \
    DEF_BINARY_RELATION_ENTRY_SEP_VARY(varg_pos, ctor, NONE_MARK, a, ##__VA_ARGS__)

#define DEF_BINARY_RELATION_ENTRY_SEP_COMMA(varg_pos, ctor, a, ...) \
    DEF_BINARY_RELATION_ENTRY_SEP_VARY(varg_pos, ctor, COMMA_MARK, a, ##__VA_ARGS__)

#define DEF_BINARY_RELATION_ENTRY_SEP_SEMICOLON(varg_pos, ctor, a, ...) \
    DEF_BINARY_RELATION_ENTRY_SEP_VARY(varg_pos, ctor, SEMICOLON_MARK, a, ##__VA_ARGS__)

// macro for tenary relation

#define DEF_TERNARY_RELATION_ENTRY_1_VARY_2(ctor, a, b, c) ctor(a, b, c)

#define DEF_TERNARY_RELATION_ENTRY_1_VARY_1(ctor, a, c, b) DEF_TERNARY_RELATION_ENTRY_1_VARY_2(ctor, a, b, c)

#define DEF_TERNARY_RELATION_ENTRY_1_VARY_0(ctor, b, c, a) DEF_TERNARY_RELATION_ENTRY_1_VARY_2(ctor, a, b, c)

#define DEF_TERNARY_RELATION_ENTRY_1_VARY_HELPER(macro_name, ctor, a, b, c) macro_name(ctor, a, b, c)

#define DEF_TERNARY_RELATION_ENTRY_1(varg_pos, ctor, sep, a, b, c) \
    DEF_TERNARY_RELATION_ENTRY_1_VARY_HELPER(DEF_TERNARY_RELATION_ENTRY_1_VARY_##varg_pos, ctor, a, b, c)

#define DEF_TERNARY_RELATION_ENTRY_2_SEP_HELPER(varg_pos, ctor, sep, a, b, c0, c1) \
    DEF_TERNARY_RELATION_ENTRY_1(varg_pos, ctor, sep, a, b, c0)                    \
    META_MACRO_SEP_##sep DEF_TERNARY_RELATION_ENTRY_1(varg_pos, ctor, sep, a, b, c1)

#define DEF_TERNARY_RELATION_ENTRY_2(varg_pos, ctor, sep, a, b, c0, c1) \
    DEF_TERNARY_RELATION_ENTRY_2_SEP_HELPER(varg_pos, ctor, sep, a, b, c0, c1)

#define DEF_TERNARY_RELATION_ENTRY_4_SEP_HELPER(varg_pos, ctor, sep, a, b, c0, c1, c2, c3) \
    DEF_TERNARY_RELATION_ENTRY_2(varg_pos, ctor, sep, a, b, c0, c1)                        \
    META_MACRO_SEP_##sep DEF_TERNARY_RELATION_ENTRY_2(varg_pos, ctor, sep, a, b, c2, c3)

#define DEF_TERNARY_RELATION_ENTRY_4(varg_pos, ctor, sep, a, b, c0, c1, c2, c3) \
    DEF_TERNARY_RELATION_ENTRY_4_SEP_HELPER(varg_pos, ctor, sep, a, b, c0, c1, c2, c3)

#define DEF_TERNARY_RELATION_ENTRY_3(varg_pos, ctor, sep, a, b, ...) \
    META_MACRO_CASE_DEF(DEF_TERNARY_RELATION_ENTRY_, sep, 5, varg_pos, ctor, sep, a, b, ##__VA_ARGS__)

#define DEF_TERNARY_RELATION_ENTRY_5(varg_pos, ctor, sep, a, b, ...) \
    META_MACRO_CASE_DEF(DEF_TERNARY_RELATION_ENTRY_, sep, 5, varg_pos, ctor, sep, a, b, ##__VA_ARGS__)

#define DEF_TERNARY_RELATION_ENTRY_6(varg_pos, ctor, sep, a, b, ...) \
    META_MACRO_CASE_DEF(DEF_TERNARY_RELATION_ENTRY_, sep, 5, varg_pos, ctor, sep, a, b, ##__VA_ARGS__)

#define DEF_TERNARY_RELATION_ENTRY_7(varg_pos, ctor, sep, a, b, ...) \
    META_MACRO_CASE_DEF(DEF_TERNARY_RELATION_ENTRY_, sep, 5, varg_pos, ctor, sep, a, b, ##__VA_ARGS__)

#define DEF_TERNARY_RELATION_ENTRY_8(varg_pos, ctor, sep, a, b, ...) \
    META_MACRO_CASE_DEF(DEF_TERNARY_RELATION_ENTRY_, sep, 5, varg_pos, ctor, sep, a, b, ##__VA_ARGS__)

#define DEF_TERNARY_RELATION_ENTRY_9(varg_pos, ctor, sep, a, b, ...) \
    META_MACRO_CASE_DEF(DEF_TERNARY_RELATION_ENTRY_, sep, 5, varg_pos, ctor, sep, a, b, ##__VA_ARGS__)

#define DEF_TERNARY_RELATION_ENTRY_10(varg_pos, ctor, sep, a, b, ...) \
    META_MACRO_CASE_DEF(DEF_TERNARY_RELATION_ENTRY_, sep, 5, varg_pos, ctor, sep, a, b, ##__VA_ARGS__)

#define DEF_TERNARY_RELATION_ENTRY_SEP_VARY(ctor, sep, varg_pos, a, b, ...) \
    META_MACRO_SELECT(DEF_TERNARY_RELATION_ENTRY_, 5, varg_pos, ctor, sep, a, b, ##__VA_ARGS__)

// separated by semicolon(,) mark
#define DEF_TERNARY_RELATION_ENTRY_SEP_COMMA(ctor, varg_pos, a, b, ...) \
    DEF_TERNARY_RELATION_ENTRY_SEP_VARY(ctor, COMMA_MARK, varg_pos, a, b, ##__VA_ARGS__)

// separated by semicolon(;) mark
#define DEF_TERNARY_RELATION_ENTRY_SEP_SEMICOLON(ctor, varg_pos, a, b, ...) \
    DEF_TERNARY_RELATION_ENTRY_SEP_VARY(ctor, NONE_MARK, varg_pos, a, b, ##__VA_ARGS__)

// separated by none mark
#define DEF_TERNARY_RELATION_ENTRY_SEP_NONE(ctor, varg_pos, a, b, ...) \
    DEF_TERNARY_RELATION_ENTRY_SEP_VARY(ctor, NONE_MARK, varg_pos, a, b, ##__VA_ARGS__)

// pair list processing

#define META_MACRO_PAIR_LIST_MAP_0(ctor)

#define META_MACRO_PAIR_LIST_MAP_2(ctor, a0, b0) ctor(a0, b0)

#define META_MACRO_PAIR_LIST_MAP_4(ctor, a0, b0, a1, b1) ctor(a0, b0), ctor(a1, b1)

#define META_MACRO_PAIR_LIST_MAP_6(ctor, a0, b0, a1, b1, a2, b2) ctor(a0, b0), ctor(a1, b1), ctor(a2, b2)

#define META_MACRO_PAIR_LIST_MAP_8(ctor, a0, b0, a1, b1, a2, b2, a3, b3) \
    ctor(a0, b0), ctor(a1, b1), ctor(a2, b2), ctor(a3, b3)

#define META_MACRO_PAIR_LIST_MAP_10(ctor, a0, b0, a1, b1, a2, b2, a3, b3, a4, b4) \
    ctor(a0, b0), ctor(a1, b1), ctor(a2, b2), ctor(a3, b3), ctor(a4, b4)

#define META_MACRO_PAIR_FIRST(a, b) a

#define META_MACRO_PAIR_LIST_FIRST(...) \
    META_MACRO_SELECT(META_MACRO_PAIR_LIST_MAP_, 1, META_MACRO_PAIR_FIRST, ##__VA_ARGS__)

#define META_MACRO_PAIR_SECOND(a, b) b

#define META_MACRO_PAIR_LIST_SECOND(...) \
    META_MACRO_SELECT(META_MACRO_PAIR_LIST_MAP_, 1, META_MACRO_PAIR_SECOND, ##__VA_ARGS__)

#define META_MACRO_PAIR_CONCAT_WS(a, b) a b

#define META_MACRO_PAIR_LIST_CONCAT_WS(...) \
    META_MACRO_SELECT(META_MACRO_PAIR_LIST_MAP_, 1, META_MACRO_PAIR_CONCAT_WS, ##__VA_ARGS__)
