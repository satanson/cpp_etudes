//
// Created by grakra on 2020/11/10.
//

#include <benchmark/benchmark.h>
#include <repeat.hh>

void BM_repeat_string_logn(benchmark::State &state){
  std::string s = "hello";
  for (auto _:state) {
    repeat_string_logn(s, 100);
  }
}

void BM_repeat_string_n(benchmark::State &state){
  std::string s = "hello";
  for (auto _:state) {
    repeat_string_n(s, 100);
  }
}

void BM_repeat_string_logn_gutil_memcpy_inline(benchmark::State &state){
  std::string s = "hello";
  for (auto _:state) {
    repeat_string_logn_gutil_memcpy_inline(s, 100);
  }
}

void BM_repeat_string_logn_simd_memcpy_inline_1(benchmark::State &state){
  std::string s = "hello";
  for (auto _:state) {
    repeat_string_logn_simd_memcpy_inline_1(s, 100);
  }
}

void BM_repeat_string_logn_simd_memcpy_inline_2(benchmark::State &state){
  std::string s = "hello";
  for (auto _:state) {
    repeat_string_logn_simd_memcpy_inline_2(s, 100);
  }
}

BENCHMARK(BM_repeat_string_logn);
BENCHMARK(BM_repeat_string_n);
BENCHMARK(BM_repeat_string_logn_gutil_memcpy_inline);
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_1);
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_2);
BENCHMARK_MAIN();