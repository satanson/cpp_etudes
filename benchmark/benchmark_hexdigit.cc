// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/9/29.
//

#include <benchmark/benchmark.h>

#include <hexdigit.hh>
#include <string>
std::string data;

PrepareData prepare_data(data, 1024000);

static void BM_hexdigit_ord_nonsimd(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(unhex0(data));
    }
}

static void BM_hexdigit_ord_simd(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(unhex1(data));
    }
}

static void BM_hexdigit_ord_simd_group(benchmark::State& state) {
    for (auto _ : state) {
        benchmark::DoNotOptimize(unhex2(data));
    }
}

BENCHMARK(BM_hexdigit_ord_simd_group);
BENCHMARK(BM_hexdigit_ord_nonsimd);
BENCHMARK(BM_hexdigit_ord_simd);

BENCHMARK_MAIN();
