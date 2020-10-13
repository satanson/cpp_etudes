//
// Created by grakra on 2020/9/29.
//

#include <benchmark/benchmark.h>
#include <decimal_microbm/decimal_microbm.hh>
#include <random>
#include <iostream>
#include <cstring>
#include <include/util/defer.hh>

size_t batch_size = 8192;
int128_t zero = static_cast<int128_t>(0);
std::vector<int128_t> lhs;
std::vector<int128_t> rhs;
std::vector<int128_t> result;

struct PrepareData {
  PrepareData() {
    auto batch_size_env_value = getenv("batch_size");
    if (batch_size_env_value != nullptr) {
      batch_size = strtoul(batch_size_env_value, nullptr, 10);
      if (batch_size <= 0) {
        batch_size = 8192;
      }
    } else {
      batch_size = 8192;
    }

    auto fill_zero_env_value = getenv("fill_zero");
    auto fill_zero = false;
    if (fill_zero_env_value != nullptr && strncasecmp(fill_zero_env_value, "true", 4)==0) {
      fill_zero = true;
    }

    auto fill_max_env_value = getenv("fill_max");
    auto fill_max = false;
    if (fill_max_env_value != nullptr && strncasecmp(fill_max_env_value, "true", 4)==0) {
      fill_max = true;
    }

    std::cout << std::boolalpha << "prepare data: batch_size=" << batch_size
              << ", fill_zero=" << fill_zero
              << ", fill_max=" << fill_max
              << std::endl;

    DEFER([]() {
      std::cout << "prepare data: Done" << std::endl;
    })

    lhs.resize(batch_size, zero);
    rhs.resize(batch_size, zero);
    result.resize(batch_size, zero);

    if (fill_zero) {
      return;
    }

    auto max_decimal = DorisDecimalOp::MAX_DECIMAL_VALUE;
    if (fill_max) {
      lhs.resize(0);
      rhs.resize(0);
      lhs.resize(batch_size, max_decimal);
      rhs.resize(batch_size, max_decimal);
      return;
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    /*
    std::uniform_int_distribution<int64_t> rand(INT64_MIN, INT64_MAX);


    for (int i = 0; i < batch_size; ++i) {
      int64_t hi = rand(gen);
      int64_t lo = rand(gen);
      lhs[i] = (static_cast<int128_t>(hi) << 64) + lo;
      do {
        hi = rand(gen);
        lo = rand(gen);
        rhs[i] = (static_cast<int128_t>(hi) << 64) + lo;
      } while (rhs[i] == zero);
    }
    */

    std::uniform_int_distribution<int64_t> ip_rand(-99999, 99999);
    std::uniform_int_distribution<int64_t> fp_rand(0, 99);
    for (auto i = 0; i < batch_size; ++i) {
      auto gen_int128=[&]()->int128_t {
        auto a = ip_rand(gen);
        auto b = fp_rand(gen);
        while (a == 0 && b == 0) {
          a = ip_rand(gen);
          b = fp_rand(gen);
        }
        if (b < 0) { b = -b; }
        auto positive = static_cast<int128_t>(a < 0 ? -1 : 1);
        return (static_cast<int128_t>(a) * 100 + b) * positive;
      };
      lhs[i] = gen_int128();
      rhs[i] = gen_int128();
    }
  }
} _;

static void BM_Int128_Add(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(), [](auto x, auto y) { return x + y; });
}

static void BM_DorisDecimal_Add(benchmark::State &state) {
  DorisDecimalOp addOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](auto x, auto y) { return addOp.add(x, y); });
}

static void BM_CKDecimal_Add_AdjustScale(benchmark::State &state) {
  CKDecimalOp<true, true, false, false> addOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](auto x, auto y) { return addOp.add(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_Add_AdjustScale_CheckOverflow(benchmark::State &state) {
  CKDecimalOp<true, true, true, true> addOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](auto x, auto y) { return addOp.add(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_Add(benchmark::State &state) {
  CKDecimalOp<false, true, false, false> addOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](auto x, auto y) { return addOp.add(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_Add_CheckOverflow(benchmark::State &state) {
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

static void BM_CKDecimal_Mul(benchmark::State &state) {
  CKDecimalOp<false, true, false, false> mulOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return mulOp.mul(x, y); });
}

static void BM_CKDecimal_Mul_CheckOverflow(benchmark::State &state) {
  CKDecimalOp<false, true, true, true> mulOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return mulOp.mul(x, y); });
}

static void BM_DorisDecimal_Mul(benchmark::State &state) {
  DorisDecimalOp mulOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return mulOp.mul(x, y); });
}

static void BM_Int128_Div1(benchmark::State &state) {
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [](int128_t x, int128_t y) { return x * static_cast<int128_t>(100) / y; });
}

static void BM_Int128_Div2(benchmark::State &state) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int64_t> rand(INT64_MIN, INT64_MAX);
  int128_t scale = rand(gen);
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [=](int128_t x, int128_t y) { return x * scale * scale / y; });
}

static void BM_DorisDecimal_Div(benchmark::State &state) {
  DorisDecimalOp divOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return divOp.div(x, y); });
}

static void BM_CKDecimal_DecimalDivDecimal(benchmark::State &state) {
  CKDecimalOp<true, true, false, false> divOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return divOp.div<true>(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_NonDecimalDivDecimal(benchmark::State &state) {
  CKDecimalOp<true, true, false, false> divOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return divOp.div<false>(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_DecimalDivDecimal_CheckOverflow(benchmark::State &state) {
  CKDecimalOp<true, true, true, false> divOp;
  for (auto _ : state)
    batch_compute(batch_size, lhs.data(), rhs.data(), result.data(),
                  [&](int128_t x, int128_t y) { return divOp.div<true>(x, y, static_cast<int128_t>(100)); });
}

static void BM_CKDecimal_NonDecimalDivDecimal_CheckOverflow(benchmark::State &state) {
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
