// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/10/19.
//
#include <decimal/decimal.hh>
int main(int argc, char **argv) {
  auto a = gen_int128(80);
  auto b = gen_int128(80);
  auto a1 = gen_int128(80);
  auto b1 = gen_int128(80);
  auto a2 = gen_int128(80);
  auto b2 = gen_int128(80);
  std::cout << "a=" << ToHexString(a) << std::endl;
  std::cout << "b=" << ToHexString(a) << std::endl;
  auto q0 = a / b;
  auto r0 = a % b;
  int128_t q1;
  uint128_t r1;
  DorisDecimalOp::divmodti3(a, b, q1, r1);
  assert(q0 == q1);
  assert(r0 == r1);

  auto q2 = a1 / b1;
  auto r2 = a2 % b2;
  std::cout << "q2=" << ToHexString(q2) << std::endl;
  std::cout << "r2=" << ToHexString(r2) << std::endl;
  return 0;
}
