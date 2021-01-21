// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/10/13.
//

#include <random>
#include <sstream>
#include <string>
#include <iostream>
#include <iomanip>
typedef __int128 int128_t;
int128_t gen_decimal(int p, int s) {
  std::random_device rd;
  std::mt19937 gen(rd());
  const int64_t max_int_part = exp10(p - s);
  const int64_t max_frac_part = exp10(s);
  std::uniform_int_distribution<int64_t> ip_rand(-(max_int_part - 1), max_int_part - 1);
  std::uniform_int_distribution<int64_t> fp_rand(-(max_frac_part - 1), max_frac_part - 1);
  auto a = ip_rand(gen);
  auto b = fp_rand(gen);
  while (a == 0 && b == 0) {
    a = ip_rand(gen);
    b = fp_rand(gen);
  }
  if (b < 0) { b = -b; }
  auto positive = static_cast<int128_t>(a < 0 ? -1 : 1);
  if (a < 0) { a = -a; }
  return (static_cast<int128_t>(a) * max_frac_part + b) * positive;
}

std::string to_string(int128_t decimal, int p, int s) {
  const int64_t max_int_part = exp10(p - s);
  const int64_t max_frac_part = exp10(s);
  auto frac_part = abs(static_cast<int64_t>(decimal % max_frac_part));
  auto int_part = static_cast<int64_t>(decimal / max_frac_part);
  auto sign = int_part < 0 ? "-" : "";
  std::stringstream ss;
  ss<<std::showpos <<std::setw(p-s)<<std::setfill('0')<< std::right<<int_part;
  ss <<".";
  ss<<std::noshowpos<<std::setw(s)<<std::setfill('0') << std::right <<frac_part;
  return ss.str();
}

int main() {
  auto a = gen_decimal(27,9);
  auto b = gen_decimal(27,9);
  std::cout<<"a="<<to_string(a, 27, 9)<<std::endl;
  std::cout<<"b="<<to_string(b, 27, 9)<<std::endl;

  printf("begin a+b\n");
  auto c0 = a+b;
  printf("end a+b=%s\n",to_string(c0, 27,9).c_str());

  printf("begin a-b\n");
  auto c1 = a-b;
  printf("end a-b=%s\n",to_string(c1, 27,9).c_str());

  printf("begin a*b\n");
  auto c2 = a*b;
  printf("end a*b=%s\n",to_string(c2, 27,9).c_str());

  printf("begin a+b\n");
  auto c3 = a/b;
  printf("end a/b=%s\n",to_string(c3, 27,9).c_str());
  return 0;
}
