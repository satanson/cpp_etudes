// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/03/01.
//

#pragma once

#define META_MACRO_SELECT_10TH(b1, b2, b3, b4, b5, b6, b7, b8, b9, b10, a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, ...) a10

#define META_MACRO_VA_ARGS_NUM_MINUS_0(...)                                                       \
  META_MACRO_SELECT_10TH(1,2,3,4,5,6,7,8,9,10, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_1(...)                                                       \
  META_MACRO_SELECT_10TH(1,2,3,4,5,6,7,8,9, ##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_2(...)                                                       \
  META_MACRO_SELECT_10TH(1,2,3,4,5,6,7,8,##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_3(...)                                                       \
  META_MACRO_SELECT_10TH(1,2,3,4,5,6,7,##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_4(...)                                                       \
  META_MACRO_SELECT_10TH(1,2,3,4,5,6,##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_5(...)                                                       \
  META_MACRO_SELECT_10TH(1,2,3,4,5,##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_6(...)                                                       \
  META_MACRO_SELECT_10TH(1,2,3,4,##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_7(...)                                                       \
  META_MACRO_SELECT_10TH(1,2,3,##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_8(...)                                                       \
  META_MACRO_SELECT_10TH(1,2,##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM_MINUS_9(...)                                                       \
  META_MACRO_SELECT_10TH(1,##__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define META_MACRO_VA_ARGS_NUM(...)  META_MACRO_VA_ARGS_NUM_MINUS_0(__VA_ARGS__)

#define META_MACRO_SELECT_VA(name, num, ...) name##num(__VA_ARGS__)
#define META_MACRO_SELECT_VA_HELPER(name, num, ...) META_MACRO_SELECT_VA(name, num, __VA_ARGS__)
#define META_MACRO_SELECT(name, n, ...) META_MACRO_SELECT_VA_HELPER(name, META_MACRO_VA_ARGS_NUM_MINUS_##n(__VA_ARGS__), __VA_ARGS__)

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
#define META_MACRO_PART(n, m) META_MACRO_ ##n## _PART ## m

#define META_MACRO_TAKE_0(...)
#define META_MACRO_TAKE_1(a1, ...) a1
#define META_MACRO_TAKE_2(a1, a2, ...) a1,a2
#define META_MACRO_TAKE_3(a1, a2, a3, ...) a1,a2,a3
#define META_MACRO_TAKE_4(a1, a2, a3, a4, ...) a1,a2,a3,a4
#define META_MACRO_TAKE_5(a1, a2, a3, a4, a5, ...) a1,a2,a3,a4,a5
#define META_MACRO_TAKE(n, ...) META_MACRO_TAKE_##n(__VA_ARGS__)

#define META_MACRO_DROP_0(...) __VA_ARGS__
#define META_MACRO_DROP_1(a1, ...) __VA_ARGS__
#define META_MACRO_DROP_2(a1, a2, ...) __VA_ARGS__
#define META_MACRO_DROP_3(a1, a2, a3, ...) __VA_ARGS__
#define META_MACRO_DROP_4(a1, a2, a3, a4, ...) __VA_ARGS__
#define META_MACRO_DROP_5(a1, a2, a3, a4, a5, ...) __VA_ARGS__
#define META_MACRO_DROP(n, ...) META_MACRO_DROP_##n(__VA_ARGS__)

#define META_MACRO_CASE_DEF_PART_HELPER2(name, ...) \
    name(__VA_ARGS__)

#define META_MACRO_CASE_DEF_PART0_HELPER1(name, n, m0, m1, m2, ...) \
    META_MACRO_CASE_DEF_PART_HELPER2(name, META_MACRO_TAKE(n,##__VA_ARGS__), META_MACRO_TAKE(m0, META_MACRO_DROP(n, ##__VA_ARGS__)))

#define META_MACRO_CASE_DEF_PART1_HELPER1(name, n, m0, m1, m2, ...) \
    META_MACRO_CASE_DEF_PART_HELPER2(name, META_MACRO_TAKE(n, ##__VA_ARGS__), META_MACRO_TAKE(m1, META_MACRO_DROP(m0, META_MACRO_DROP(n, ##__VA_ARGS__))))

#define META_MACRO_CASE_DEF_PART2_HELPER1(name, n, m0, m1, m2, ...) \
    META_MACRO_CASE_DEF_PART_HELPER2(name, META_MACRO_TAKE(n, ##__VA_ARGS__), META_MACRO_DROP(m1, META_MACRO_DROP(m0, META_MACRO_DROP(n, ##__VA_ARGS__))))

#define META_MACRO_CASE_DEF_DECOMPOSE(name, n, m, m0, m1, m2, ...) \
    META_MACRO_CASE_DEF_PART0_HELPER1(name##m0, n, m0, m1, m2, ##__VA_ARGS__),  \
    META_MACRO_CASE_DEF_PART1_HELPER1(name##m1, n, m0, m1, m2, ##__VA_ARGS__),  \
    META_MACRO_CASE_DEF_PART2_HELPER1(name##m2, n, m0, m1, m2, ##__VA_ARGS__)

#define META_MACRO_CASE_DEF_DECOMPOSE_HELPER(name, n, m, m0, m1, m2, ...) \
    META_MACRO_CASE_DEF_DECOMPOSE(name, n, m, m0, m1, m2, ##__VA_ARGS__)                                                                       \

#define META_MACRO_CASE_DEF_HELPER(name, n, m, ...) \
    META_MACRO_CASE_DEF_DECOMPOSE_HELPER(name, n, m, META_MACRO_PART(m,0), META_MACRO_PART(m,1),META_MACRO_PART(m,2), ##__VA_ARGS__)

#define META_MACRO_CASE_DEF(name, n, ...) \
    META_MACRO_CASE_DEF_HELPER(name, n, META_MACRO_VA_ARGS_NUM_MINUS_##n(__VA_ARGS__), ##__VA_ARGS__)