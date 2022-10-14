// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/10.
//

#include <benchmark/benchmark.h>

#include <iostream>
#include <repeat.hh>

void BM_repeat_string_logn(benchmark::State& state) {
    std::string s(state.range(0), 'x');
    for (auto _ : state) {
        repeat_string_logn(s, state.range(1));
    }
}

void BM_fast_repeat(benchmark::State& state) {
    auto size = state.range(0);
    auto times = state.range(1);
    std::string src_str(size, 'x');
    std::string dst_str(size * times + 1, 'x');
    uint8_t* dst = (uint8_t*)dst_str.data();
    uint8_t* src = (uint8_t*)src_str.data();
    for (auto _ : state) {
        fast_repeat(src, dst, size + 1, times);
    }
}

void BM_original_repeat(benchmark::State& state) {
    auto size = state.range(0);
    auto times = state.range(1);
    std::string src_str(size, 'x');
    std::string dst_str(size * times + 1, '\0');
    uint8_t* dst = (uint8_t*)dst_str.data();
    uint8_t* src = (uint8_t*)src_str.data();
    for (auto _ : state) {
        original_repeat(src, dst, size + 1, times);
    }
}

void BM_repeat_string_n(benchmark::State& state) {
    std::string s(state.range(0), 'x');
    for (auto _ : state) {
        repeat_string_n(s, state.range(1));
    }
}

void BM_repeat_string_logn_gutil_memcpy_inline(benchmark::State& state) {
    std::string s(state.range(0), 'x');
    for (auto _ : state) {
        repeat_string_logn_gutil_memcpy_inline(s, state.range(1));
    }
}

void BM_repeat_string_logn_memcpy(benchmark::State& state) {
    std::string s(state.range(0), 'x');
    for (auto _ : state) {
        repeat_string_logn_memcpy(s, state.range(1));
    }
}

void BM_repeat_string_logn_simd_memcpy_inline_1(benchmark::State& state) {
    std::string s(state.range(0), 'x');
    for (auto _ : state) {
        repeat_string_logn_simd_memcpy_inline_1(s, state.range(1));
    }
}

void BM_repeat_string_logn_simd_memcpy_inline_2(benchmark::State& state) {
    std::string s(state.range(0), 'x');
    for (auto _ : state) {
        repeat_string_logn_simd_memcpy_inline_2(s, state.range(1));
    }
}
#if 0
BENCHMARK(BM_fast_repeat)
    ->Args({1, 1})
    ->Args({1, 15})
    ->Args({1, 100})
    ->Args({1, 1000})
    ->Args({1, 10000})
    ->Args({10, 1})
    ->Args({10, 15})
    ->Args({10, 100})
    ->Args({10, 1000})
    ->Args({10, 10000});
BENCHMARK(BM_original_repeat)
    ->Args({1, 1})
    ->Args({1, 15})
    ->Args({1, 100})
    ->Args({1, 1000})
    ->Args({1, 10000})
    ->Args({10, 1})
    ->Args({10, 15})
    ->Args({10, 100})
    ->Args({10, 1000})
    ->Args({10, 10000});

#endif

#if 1
BENCHMARK(BM_repeat_string_logn)->Args({1, 10});
// BENCHMARK(BM_repeat_string_n)->Args({1,10});
BENCHMARK(BM_repeat_string_logn_gutil_memcpy_inline)->Args({1, 10});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_1)->Args({1, 10});
BENCHMARK(BM_repeat_string_logn_memcpy)->Args({1, 10});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_2)->Args({1, 10});

BENCHMARK(BM_repeat_string_logn)->Args({1, 50});
// BENCHMARK(BM_repeat_string_n)->Args({1,50});
BENCHMARK(BM_repeat_string_logn_gutil_memcpy_inline)->Args({1, 50});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_1)->Args({1, 50});
BENCHMARK(BM_repeat_string_logn_memcpy)->Args({1, 50});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_2)->Args({1, 50});

BENCHMARK(BM_repeat_string_logn)->Args({1, 100});
// BENCHMARK(BM_repeat_string_n)->Args({1,100});
BENCHMARK(BM_repeat_string_logn_gutil_memcpy_inline)->Args({1, 100});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_1)->Args({1, 100});
BENCHMARK(BM_repeat_string_logn_memcpy)->Args({1, 100});
BENCHMARK(BM_repeat_string_logn_simd_memcpy_inline_2)->Args({1, 100});
#endif
BENCHMARK_MAIN();
