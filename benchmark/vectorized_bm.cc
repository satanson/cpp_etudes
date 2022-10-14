// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/9/29.
//

#include <benchmark/benchmark.h>

#include <cstring>
#include <include/decimal/decimal.hh>
#include <include/util/defer.hh>
#include <iostream>
#include <random>

PrepareData data;
size_t batch_size = data.batch_size;
std::vector<int128_t>& lhs = data.lhs;
std::vector<int128_t>& rhs = data.rhs;
std::vector<int128_t>& result = data.result;

static void BM_Int128_Add(benchmark::State& state) {
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(), [](auto x, auto y) { return x + y; });
}

static void BM_DorisDecimal_Add(benchmark::State& state) {
    DorisDecimalOp addOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](auto x, auto y) { return addOp.add(x, y); });
}

static void BM_CKDecimal_Add_AdjustScale(benchmark::State& state) {
    CKDecimalOp<true, true, false, false> addOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](auto x, auto y) { return addOp.add(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_Add_AdjustScale_CheckOverflow(benchmark::State& state) {
    CKDecimalOp<true, true, true, true> addOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](auto x, auto y) { return addOp.add(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_Add(benchmark::State& state) {
    CKDecimalOp<false, true, false, false> addOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](auto x, auto y) { return addOp.add(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_Add_CheckOverflow(benchmark::State& state) {
    CKDecimalOp<false, true, true, true> addOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](auto x, auto y) { return addOp.add(x, y, static_cast<int128_t>(100)); });
}

static void BM_Int128_Mul(benchmark::State& state) {
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(), [](int128_t x, int128_t y) { return x * y; });
}

static void BM_CKDecimal_Mul(benchmark::State& state) {
    CKDecimalOp<false, true, false, false> mulOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](int128_t x, int128_t y) { return mulOp.mul(x, y); });
}

static void BM_CKDecimal_Mul_CheckOverflow(benchmark::State& state) {
    CKDecimalOp<false, true, true, true> mulOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](int128_t x, int128_t y) { return mulOp.mul(x, y); });
}

static void BM_DorisDecimal_Mul(benchmark::State& state) {
    DorisDecimalOp mulOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](int128_t x, int128_t y) { return mulOp.mul(x, y); });
}

static void BM_Int128_Div1(benchmark::State& state) {
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [](int128_t x, int128_t y) { return x * static_cast<int128_t>(100) / y; });
}

static void BM_Int128_Div2(benchmark::State& state) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int64_t> rand(INT64_MIN, INT64_MAX);
    int128_t scale = rand(gen);
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [=](int128_t x, int128_t y) { return x * scale * scale / y; });
}

static void BM_DorisDecimal_Div(benchmark::State& state) {
    DorisDecimalOp divOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](int128_t x, int128_t y) { return divOp.div(x, y); });
}

static void BM_CKDecimal_DecimalDivDecimal(benchmark::State& state) {
    CKDecimalOp<true, true, false, false> divOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](int128_t x, int128_t y) { return divOp.div<true>(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_NonDecimalDivDecimal(benchmark::State& state) {
    CKDecimalOp<true, true, false, false> divOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](int128_t x, int128_t y) { return divOp.div<false>(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_DecimalDivDecimal_CheckOverflow(benchmark::State& state) {
    CKDecimalOp<true, true, true, false> divOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](int128_t x, int128_t y) { return divOp.div<true>(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_NonDecimalDivDecimal_CheckOverflow(benchmark::State& state) {
    CKDecimalOp<true, true, true, false> divOp;
    for (auto _ : state)
        batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                      [&](int128_t x, int128_t y) { return divOp.div<false>(x, y, static_cast<int128_t>(100)); });
}

BENCHMARK(BM_Int128_Add);
BENCHMARK(BM_CKDecimal_Add);
BENCHMARK(BM_CKDecimal_Add_CheckOverflow);
BENCHMARK(BM_CKDecimal_Add_AdjustScale);
BENCHMARK(BM_CKDecimal_Add_AdjustScale_CheckOverflow);
BENCHMARK(BM_DorisDecimal_Add);

BENCHMARK(BM_Int128_Mul);
BENCHMARK(BM_DorisDecimal_Mul);
BENCHMARK(BM_CKDecimal_Mul);
BENCHMARK(BM_CKDecimal_Mul_CheckOverflow);

BENCHMARK(BM_Int128_Div1);
BENCHMARK(BM_Int128_Div2);
BENCHMARK(BM_DorisDecimal_Div);
BENCHMARK(BM_CKDecimal_DecimalDivDecimal);
BENCHMARK(BM_CKDecimal_NonDecimalDivDecimal);
BENCHMARK(BM_CKDecimal_NonDecimalDivDecimal_CheckOverflow);
BENCHMARK(BM_CKDecimal_DecimalDivDecimal_CheckOverflow);

BENCHMARK_MAIN();
