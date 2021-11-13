// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/10.
//

#include <benchmark/benchmark.h>
#include <raw_container.hh>
#include <iostream>
void BM_string_std_allocator(benchmark::State &state) {
    size_t n = state.range(0);
    for (auto _ : state) {
        std::string s;
        s.resize(n);
        s.resize(0);
    }

}
void BM_string_initialize_x(benchmark::State &state) {
    size_t n = state.range(0);
    for (auto _ : state) {
        std::string s;
        s.resize( n, 'x');
    }

}
void BM_string_raw_allocator(benchmark::State &state) {
    size_t n = state.range(0);
    for (auto _ : state) {
        std::string s;
        raw::make_room(s, n);
        s.resize(0);
    }
}
BENCHMARK(BM_string_initialize_x)->RangeMultiplier(8)->Range(1, 32<<10);
BENCHMARK(BM_string_raw_allocator)->RangeMultiplier(8)->Range(1, 32<<10);
BENCHMARK(BM_string_std_allocator)->RangeMultiplier(8)->Range(1, 32<<10);
BENCHMARK_MAIN();
