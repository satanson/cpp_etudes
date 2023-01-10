//
// Created by grakra on 23-1-9.
//
#include <benchmark/benchmark.h>
#include "hll.hh"
std::vector<int8_t> data14(2<<14);
std::vector<int8_t> data17(2<<14);
std::vector<int8_t> data20(2<<14);

DataInitializer data14_init(data14);
DataInitializer data17_init(data17);
DataInitializer data20_init(data20);
constexpr size_t num_round = 10;
void BM_calc_harmonic_mean1_14(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i=0; i<num_round;++i)
        benchmark::DoNotOptimize(calc_harmonic_mean1(data14.data(), data14.size()));
    }
}

void BM_calc_harmonic_mean2_14(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i=0; i<num_round;++i)
            benchmark::DoNotOptimize(calc_harmonic_mean2(data14.data(), data14.size()));
    }
}

void BM_calc_harmonic_mean3_14(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i=0; i<num_round;++i)
        benchmark::DoNotOptimize(calc_harmonic_mean3(data14.data(), data14.size()));
    }
}

void BM_calc_harmonic_mean4_14(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i=0; i<num_round;++i)
        benchmark::DoNotOptimize(calc_harmonic_mean4(data14.data(), data14.size()));
    }
}

void BM_calc_harmonic_mean1_20(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i=0; i<num_round;++i)
        benchmark::DoNotOptimize(calc_harmonic_mean1(data20.data(), data20.size()));
    }
}

void BM_calc_harmonic_mean2_20(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i=0; i<num_round;++i)
        benchmark::DoNotOptimize(calc_harmonic_mean2(data20.data(), data20.size()));
    }
}

void BM_calc_harmonic_mean3_20(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i=0; i<num_round;++i)
        benchmark::DoNotOptimize(calc_harmonic_mean3(data20.data(), data20.size()));
    }
}

void BM_calc_harmonic_mean4_20(benchmark::State& state) {
    for (auto _ : state) {
        for (auto i=0; i<num_round;++i)
        benchmark::DoNotOptimize(calc_harmonic_mean4(data20.data(), data20.size()));
    }
}

BENCHMARK(BM_calc_harmonic_mean4_14);
BENCHMARK(BM_calc_harmonic_mean3_14);
BENCHMARK(BM_calc_harmonic_mean1_14);
BENCHMARK(BM_calc_harmonic_mean2_14);

BENCHMARK(BM_calc_harmonic_mean4_20);
BENCHMARK(BM_calc_harmonic_mean3_20);
BENCHMARK(BM_calc_harmonic_mean1_20);
BENCHMARK(BM_calc_harmonic_mean2_20);

BENCHMARK_MAIN();
