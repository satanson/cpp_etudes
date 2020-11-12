//
// Created by grakra on 2020/11/10.
//

#include <benchmark/benchmark.h>
#include <repeat.hh>
void BM_repeat_string_logn(benchmark::State &state) {
  std::string s(state.range(0), 'x');
  for (auto _:state) {
    repeat_string_logn(s, state.range(1));
  }
}

void BM_repeat_string_n(benchmark::State &state) {
  std::string s(state.range(0), 'x');
  for (auto _:state) {
    repeat_string_n(s, state.range(1));
  }
}

void BM_repeat_string_logn_gutil_memcpy_inline(benchmark::State &state) {
  std::string s(state.range(0), 'x');
  for (auto _:state) {
    repeat_string_logn_gutil_memcpy_inline(s, state.range(1));
  }
}

void BM_repeat_string_logn_simd_memcpy_inline_1(benchmark::State &state) {
  std::string s(state.range(0), 'x');
  for (auto _:state) {
    repeat_string_logn_simd_memcpy_inline_1(s, state.range(1));
  }
}

void BM_repeat_string_logn_simd_memcpy_inline_2(benchmark::State &state) {
  std::string s(state.range(0), 'x');
  for (auto _:state) {
    repeat_string_logn_simd_memcpy_inline_2(s, state.range(1));
  }
}

BENCHMARK(BM_repeat_string_n)
    ->Args({1, 1})
    ->Args({1, 15})
    ->Args({1, 100})
    ->Args({1, 1000})
    ->Args({1, 10000});

BENCHMARK(BM_repeat_string_logn_gutil_memcpy_inline)
    ->Args({1, 1})
    ->Args({1, 15})
    ->Args({1, 100})
    ->Args({1, 1000})
    ->Args({1, 10000});

#if 0
BENCHMARK(BM_repeat_string_logn)->Args({1,10});
BENCHMARK(BM_repeat_string_n)->Args({1,10});
BENCHMARK(BM_repeat_string_logn_gutil_memcpy_inline)->Args({1,10});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_1)->Args({1,10});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_2)->Args({1,10});

BENCHMARK(BM_repeat_string_logn)->Args({1,50});
BENCHMARK(BM_repeat_string_n)->Args({1,50});
BENCHMARK(BM_repeat_string_logn_gutil_memcpy_inline)->Args({1,50});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_1)->Args({1,50});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_2)->Args({1,50});

BENCHMARK(BM_repeat_string_logn)->Args({1,100});
BENCHMARK(BM_repeat_string_n)->Args({1,100});
BENCHMARK(BM_repeat_string_logn_gutil_memcpy_inline)->Args({1,100});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_1)->Args({1,100});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_2)->Args({1,100});
#endif
BENCHMARK_MAIN();