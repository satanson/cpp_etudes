// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com

//
// Created by grakra on 2020/10/20.
//

#include <benchmark/benchmark.h>
#include <string_functions.hh>
prepare_utf8_data prepare;
auto &data = prepare.data;
auto &result = prepare.result;

void BM_utf8_length(benchmark::State &state) {
  for (auto _ : state) {
    apply_vector(data, result, [](std::string &s) {
      return StringFunctions::utf8_length(s);
    });
  }
}

void BM_utf8_length2(benchmark::State &state) {
  for (auto _ : state) {
    apply_vector(data, result, [](std::string &s) {
      return StringFunctions::utf8_length2(s);
    });
  }
}

void BM_utf8_length3(benchmark::State &state) {
  for (auto _ : state) {
    apply_vector(data, result, [](std::string &s) {
      return StringFunctions::utf8_length3(s);
    });
  }
}

void BM_utf8_length4(benchmark::State &state) {
  for (auto _ : state) {
    apply_vector(data, result, [](std::string &s) {
      return StringFunctions::utf8_length4(s);
    });
  }
}
void BM_utf8_length_simd_sse2(benchmark::State &state) {
  for (auto _ : state) {
    apply_vector(data, result, [](std::string &s) {
      return StringFunctions::utf8_length_simd_sse2(s);
    });
  }
}
void BM_utf8_length_simd_avx2(benchmark::State &state) {
  for (auto _ : state) {
    apply_vector(data, result, [](std::string &s) {
      return StringFunctions::utf8_length_simd_avx2(s);
    });
  }
}
BENCHMARK(BM_utf8_length);
BENCHMARK(BM_utf8_length2);
BENCHMARK(BM_utf8_length3);
BENCHMARK(BM_utf8_length4);
BENCHMARK(BM_utf8_length);
BENCHMARK(BM_utf8_length2);
BENCHMARK(BM_utf8_length_simd_sse2);
BENCHMARK(BM_utf8_length_simd_avx2);

BENCHMARK_MAIN();
