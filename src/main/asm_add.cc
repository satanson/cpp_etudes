// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/10/14.
//
#include <include/decimal/decimal.hh>
#include <iostream>
int main(int argc, char **argv) {
  DorisDecimalOp op;
  int64_t x = -1l;
  int64_t y = -1l;
  int64_t res;
  auto carry = op.asm_add(x, y, res);
  std::cout << std::showbase << std::hex << "x=" << x << ", y=" << y
            << ", res=" << res << ", carry=" << carry << std::endl;
  int128_wrapper w;
  auto c = op.asm_mul(x, y, w);
  std::cout << std::showbase << std::hex << "low=" << w.s.low
            << ", high=" << w.s.high << ", c=" << c << std::endl;
  int128_wrapper wx;
  int128_wrapper wy;
  int128_wrapper wz;
  wx.s128 = static_cast<int128_t>(0xffff000000000000l);
  wy.s128 = static_cast<int128_t>(0xfffe000000000000l);
  auto o = op.multi3(wx, wy, wz);
  std::cout << std::showbase << std::hex << " high=" << wz.s.high
            << ", low=" << wz.s.low << ", o=" << o << std::endl;

  int128_t q;
  uint128_t r;
  op.divmodti3(x, y, q, r);
}
