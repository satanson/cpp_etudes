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
    std::string s;
    for (auto _ : state) {
        s.resize(state.range(0));
        s.resize(0);
    }

}
void BM_string_raw_allocator(benchmark::State &state) {
    std::string s;
    for (auto _ : state) {
        raw::make_room(s, state.range(0));
        s.resize(0);
    }
}
BENCHMARK(BM_string_raw_allocator)->RangeMultiplier(8)->Range(1, 32<<10);
BENCHMARK(BM_string_std_allocator)->RangeMultiplier(8)->Range(1, 32<<10);
BENCHMARK_MAIN();
