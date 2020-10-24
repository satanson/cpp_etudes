//
// Created by grakra on 2020/9/29.
//

#include <benchmark/benchmark.h>
#include <include/decimal/decimal.hh>
#include <random>
#include <iostream>
#include <cstring>
#include <include/util/defer.hh>

PrepareData data;
size_t batch_size = data.batch_size;
std::vector<int128_t> &lhs = data.lhs;
std::vector<int128_t> &rhs = data.rhs;
std::vector<int128_t> &result = data.result;

std::vector<int64_t> &lhs64 = data.lhs64;
std::vector<int64_t> &rhs64 = data.rhs64;
std::vector<int64_t> &result64 = data.result64;

std::vector<int32_t> &lhs32 = data.lhs32;
std::vector<int32_t> &rhs32 = data.rhs32;
std::vector<int32_t> &result32 = data.result32;

static void BM_Int128_And(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(), [](auto x, auto y) { return x & y; });
}

static void BM_Int128_Shift(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(), [](auto x, auto y) { return x >> 16; });
}

static void BM_Int128_Add(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(), [](auto x, auto y) { return x + y; });
}
static void BM_Int128_Sub(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(), [](auto x, auto y) { return x - y; });
}

static void BM_Int128_Mul(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [](int128_t x, int128_t y) { return x * y; });
}

static void BM_Int128_Div(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [](int128_t x, int128_t y) { return x / y; });
}

static void BM_Int128_Mod(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [](int128_t x, int128_t y) { return x % y; });
}

static void BM_Int128_DivMod(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [](int128_t x, int128_t y) { return (x / y) & (x % y); });
}

static void BM_Int64_And(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs64.data(), rhs64.data(), result64.data(), [](auto x, auto y) { return x & y; });
}

static void BM_Int64_Shift(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs64.data(), rhs64.data(), result64.data(), [](auto x, auto y) { return x >> 16; });
}

static void BM_Int64_Add(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs64.data(), rhs64.data(), result64.data(), [](auto x, auto y) { return x + y; });
}
static void BM_Int64_Sub(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs64.data(), rhs64.data(), result64.data(), [](auto x, auto y) { return x - y; });
}

static void BM_Int64_Mul(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs64.data(), rhs64.data(), result64.data(),
                  [](int64_t x, int64_t y) { return x * y; });
}

static void BM_Int64_Div(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs64.data(), rhs64.data(), result64.data(),
                  [](int64_t x, int64_t y) { return x / y; });
}

static void BM_Int64_Mod(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs64.data(), rhs64.data(), result64.data(),
                  [](int64_t x, int64_t y) { return x % y; });
}

static void BM_Int64_DivMod(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs64.data(), rhs64.data(), result64.data(),
                  [](int64_t x, int64_t y) { return (x / y) & (x % y); });
}

static void BM_Int32_And(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs32.data(), rhs32.data(), result32.data(), [](auto x, auto y) { return x & y; });
}

static void BM_Int32_Shift(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs32.data(), rhs32.data(), result32.data(), [](auto x, auto y) { return x >> 16; });
}

static void BM_Int32_Add(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs32.data(), rhs32.data(), result32.data(), [](auto x, auto y) { return x + y; });
}
static void BM_Int32_Sub(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs32.data(), rhs32.data(), result32.data(), [](auto x, auto y) { return x - y; });
}

static void BM_Int32_Mul(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs32.data(), rhs32.data(), result32.data(),
                  [](int32_t x, int32_t y) { return x * y; });
}

static void BM_Int32_Div(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs32.data(), rhs32.data(), result32.data(),
                  [](int32_t x, int32_t y) { return x / y; });
}

static void BM_Int32_Mod(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs32.data(), rhs32.data(), result32.data(),
                  [](int32_t x, int32_t y) { return x % y; });
}

static void BM_Int32_DivMod(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs32.data(), rhs32.data(), result32.data(),
                  [](int32_t x, int32_t y) { return (x / y) & (x % y); });
}
BENCHMARK(BM_Int128_Shift);
BENCHMARK(BM_Int128_And);
BENCHMARK(BM_Int128_Add);
BENCHMARK(BM_Int128_Sub);
BENCHMARK(BM_Int128_Mul);
BENCHMARK(BM_Int128_Div);
BENCHMARK(BM_Int128_Mod);
BENCHMARK(BM_Int128_DivMod);

BENCHMARK(BM_Int64_Shift);
BENCHMARK(BM_Int64_And);
BENCHMARK(BM_Int64_Add);
BENCHMARK(BM_Int64_Sub);
BENCHMARK(BM_Int64_Mul);
BENCHMARK(BM_Int64_Div);
BENCHMARK(BM_Int64_Mod);
BENCHMARK(BM_Int64_DivMod);

BENCHMARK(BM_Int32_Shift);
BENCHMARK(BM_Int32_And);
BENCHMARK(BM_Int32_Add);
BENCHMARK(BM_Int32_Sub);
BENCHMARK(BM_Int32_Mul);
BENCHMARK(BM_Int32_Div);
BENCHMARK(BM_Int32_Mod);
BENCHMARK(BM_Int32_DivMod);

BENCHMARK_MAIN();
