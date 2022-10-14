// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/10/30.
//

#include <benchmark/benchmark.h>

#include <default_init_allocator.hh>

template <class T, typename A = std::allocator<T>>
static void BM_resize(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<T, A> vector;
        vector.resize(state.range(0));
    }
}

template <class T, class U>
static void BM_raw_vector_move(benchmark::State& state) {
    for (auto _ : state) {
        raw::raw_vector<U> rv0;
        rv0.resize(state.range(0));
        std::vector<T> rv1(std::move(reinterpret_cast<std::vector<T>&>(rv0)));
    }
}

template <class T, class U>
static void BM_vector_move(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<U> rv0;
        rv0.resize(state.range(0));
        std::vector<T> rv1(std::move(reinterpret_cast<std::vector<T>&>(rv0)));
    }
}

// BENCHMARK_TEMPLATE(BM_resize, int8_t,
// default_init_allocator<int8_t>)->RangeMultiplier(2)->Range(8, 8<<10);
// BENCHMARK_TEMPLATE(BM_resize, uint8_t,
// default_init_allocator<uint8_t>)->RangeMultiplier(2)->Range(8, 8<<10);
// BENCHMARK_TEMPLATE(BM_resize, int8_t)->RangeMultiplier(2)->Range(8, 8<<10);
// BENCHMARK_TEMPLATE(BM_resize, uint8_t)->RangeMultiplier(2)->Range(8, 8<<10);
BENCHMARK_TEMPLATE(BM_raw_vector_move, uint8_t, int8_t)->RangeMultiplier(8)->Range(1, 8 << 10);
BENCHMARK_TEMPLATE(BM_vector_move, uint8_t, int8_t)->RangeMultiplier(8)->Range(1, 8 << 10);
BENCHMARK_TEMPLATE(BM_raw_vector_move, uint8_t, uint8_t)->RangeMultiplier(8)->Range(1, 8 << 10);
BENCHMARK_TEMPLATE(BM_vector_move, uint8_t, uint8_t)->RangeMultiplier(8)->Range(1, 8 << 10);
BENCHMARK_TEMPLATE(BM_raw_vector_move, uint8_t, char)->RangeMultiplier(8)->Range(1, 8 << 10);
BENCHMARK_TEMPLATE(BM_vector_move, uint8_t, char)->RangeMultiplier(8)->Range(1, 8 << 10);
BENCHMARK_MAIN();
