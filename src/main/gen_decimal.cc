// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/10/9.
//

#include <random>
#include <iostream>
#include <cassert>

typedef __int128 int128_t;

int64_t exp10(size_t n) {
  if (n == 0) {
    return static_cast<int128_t>(1);
  } else {
    return exp10(n - 1) * 10;
  }
}

int main(int argc, char **argv) {
  if (argc < 4) {
    std::cerr << "missing arguments!\n"
              << "gen_decimal <count> <p> <s>"
              << std::endl;
    exit(1);
  }

  size_t n = strtoul(argv[1], nullptr, 10);
  size_t p = strtoul(argv[2], nullptr, 10);
  size_t s = strtoul(argv[3], nullptr, 10);

  assert(n > 0 && p > 0 && s > 0);
  assert(p <= 18 && s <= p);

  std::random_device rd;
  std::mt19937 gen(rd());
  const int64_t max_int_part = exp10(p - s)-1;
  const int64_t max_frac_part = exp10(s)-1;
  //std::cout << "max_int_part=" << max_int_part
  //          << ", max_frac_part=" << max_frac_part << std::endl;
  std::uniform_int_distribution<int64_t> ip_rand(-max_int_part, max_int_part);
  std::uniform_int_distribution<int64_t> fp_rand(-max_frac_part, max_frac_part);
  for (auto i = 0; i < n; ++i) {
    auto a = ip_rand(gen);
    auto b = fp_rand(gen);
    while (a == 0 && b == 0) {
      a = ip_rand(gen);
      b = fp_rand(gen);
    }
    if (b < 0) { b = -b; }
    auto positive = a < 0 ? false : true;
    if (a < 0) { a = -a; }
    printf("%s%0lld.%0lld\n", positive ? "" : "-", a, b);
  }
}
