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

static void BM_Int128_Mul(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [](int128_t x, int128_t y) { return x * y; });
}

static void BM_DorisDecimal_Mul(benchmark::State &state) {
  DorisDecimalOp mulOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return mulOp.mul(x, y); });
}

static void BM_DorisDecimal_Mul2(benchmark::State &state) {
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

static void BM_CKDecimal_Mul_CheckOverflow(benchmark::State &state) {
  CKDecimalOp<false, true, true, true> mulOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return mulOp.mul(x, y); });
}

BENCHMARK(BM_Int128_Mul);
BENCHMARK(BM_Int128_Div);
BENCHMARK(BM_DorisDecimal_Mul);
BENCHMARK(BM_DorisDecimal_Mul2);
BENCHMARK(BM_CKDecimal_Mul_CheckOverflow);

BENCHMARK_MAIN();
