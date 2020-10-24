//
// Created by grakra on 2020/10/15.
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

static void BM_Int128_Add(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(), [](auto x, auto y) { return x + y; });
}

static void BM_DorisDB_Add_old(benchmark::State &state) {
  DorisDecimalOp addOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](auto x, auto y) { return addOp.add(x, y); });
}

static void BM_DorisDB_Add_new(benchmark::State &state) {
  DorisDecimalOp addOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](auto x, auto y) { return addOp.add2(x, y); });
}

static void BM_CK_Add(benchmark::State &state) {
  CKDecimalOp<false, true, true, true> addOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](auto x, auto y) { return addOp.add(x, y, static_cast<int128_t>(100)); });
}

static void BM_Int128_Mul(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [](int128_t x, int128_t y) { return x * y; });
}

static void BM_DorisDB_Mul_old(benchmark::State &state) {
  DorisDecimalOp mulOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return mulOp.mul(x, y); });
}

static void BM_DorisDB_Mul_new(benchmark::State &state) {
  DorisDecimalOp mulOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) {
                    /*
                                  int128_wrapper *wx = (int128_wrapper *)(&x);
                                  int128_wrapper *wy = (int128_wrapper *)(&y);
                                  std::cout << std::showbase << std::hex
                                            << "x.high=" << wx->s.high
                                            << ", x.low=" << wx->s.low
                                            << ", y.high=" << wy->s.high
                                            << ", y.low=" << wy->s.low
                                            << std::endl;
                                            */
                    return mulOp.mul2(x, y);
                  });
}

static void BM_Int128_Div(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [](int128_t x, int128_t y) { return x / y; });
}

static void BM_CK_Mul(benchmark::State &state) {
  CKDecimalOp<false, true, true, true> mulOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return mulOp.mul(x, y); });
}

static void BM_CK_Div(benchmark::State &state) {
  CKDecimalOp<true, true, true, false> divOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return divOp.div<true>(x, y, static_cast<int128_t>(100)); });
}

static void BM_DorisDB_Div_old(benchmark::State &state) {
  DorisDecimalOp divOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return divOp.div(x, y); });
}

static void BM_DorisDB_Div_new(benchmark::State &state) {
  DorisDecimalOp divOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return divOp.div2(x, y); });
}

BENCHMARK(BM_Int128_Add);
BENCHMARK(BM_CK_Add);
BENCHMARK(BM_DorisDB_Add_old);
BENCHMARK(BM_DorisDB_Add_new);

BENCHMARK(BM_Int128_Mul);
BENCHMARK(BM_DorisDB_Mul_old);
BENCHMARK(BM_DorisDB_Mul_new);
BENCHMARK(BM_CK_Mul);

BENCHMARK(BM_Int128_Div);
BENCHMARK(BM_CK_Div);
BENCHMARK(BM_DorisDB_Div_old);
BENCHMARK(BM_DorisDB_Div_new);

BENCHMARK_MAIN();
