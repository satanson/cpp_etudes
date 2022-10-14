//
// Created by grakra on 2020/11/26.
//

#include <benchmark/benchmark.h>

#include <include/string_functions.hh>
#include <memequal.hh>
prepare_utf8_data col_5_10_0(4096, {1, 0, 0, 0, 0, 0}, 5, 10);
prepare_utf8_data col_5_10_1(4096, {1, 0, 0, 0, 0, 0}, 5, 10);

prepare_utf8_data col_1_20_0(4096, {1, 0, 0, 0, 0, 0}, 1, 20);
prepare_utf8_data col_1_20_1(4096, {1, 0, 0, 0, 0, 0}, 1, 20);

auto data0 = col_5_10_0.binary_column;
auto data1 = col_5_10_1.binary_column;

auto data2 = col_1_20_0.binary_column;
auto data3 = col_1_20_1.binary_column;
prepare_utf8_data col_1_1_1(4096, {1, 0, 0, 0, 0, 0}, 1, 1);
prepare_utf8_data col_2_2_1(4096, {1, 0, 0, 0, 0, 0}, 2, 2);
prepare_utf8_data col_3_3_1(4096, {1, 0, 0, 0, 0, 0}, 3, 3);
prepare_utf8_data col_4_4_1(4096, {1, 0, 0, 0, 0, 0}, 4, 4);
prepare_utf8_data col_5_5_1(4096, {1, 0, 0, 0, 0, 0}, 5, 5);
prepare_utf8_data col_6_6_1(4096, {1, 0, 0, 0, 0, 0}, 6, 6);
prepare_utf8_data col_7_7_1(4096, {1, 0, 0, 0, 0, 0}, 7, 7);
prepare_utf8_data col_8_8_1(4096, {1, 0, 0, 0, 0, 0}, 8, 8);
prepare_utf8_data col_9_9_1(4096, {1, 0, 0, 0, 0, 0}, 9, 9);
prepare_utf8_data col_10_10_1(4096, {1, 0, 0, 0, 0, 0}, 10, 10);
prepare_utf8_data col_11_11_1(4096, {1, 0, 0, 0, 0, 0}, 11, 11);
prepare_utf8_data col_12_12_1(4096, {1, 0, 0, 0, 0, 0}, 12, 12);
prepare_utf8_data col_13_13_1(4096, {1, 0, 0, 0, 0, 0}, 13, 13);
prepare_utf8_data col_14_14_1(4096, {1, 0, 0, 0, 0, 0}, 14, 14);
prepare_utf8_data col_15_15_1(4096, {1, 0, 0, 0, 0, 0}, 15, 15);
prepare_utf8_data col_16_16_1(4096, {1, 0, 0, 0, 0, 0}, 16, 16);
prepare_utf8_data col_17_17_1(4096, {1, 0, 0, 0, 0, 0}, 17, 17);
prepare_utf8_data col_18_18_1(4096, {1, 0, 0, 0, 0, 0}, 18, 18);
prepare_utf8_data col_19_19_1(4096, {1, 0, 0, 0, 0, 0}, 19, 19);
prepare_utf8_data col_20_20_1(4096, {1, 0, 0, 0, 0, 0}, 20, 20);
prepare_utf8_data col_1_1_0(4096, {1, 0, 0, 0, 0, 0}, 1, 1);
prepare_utf8_data col_2_2_0(4096, {1, 0, 0, 0, 0, 0}, 2, 2);
prepare_utf8_data col_3_3_0(4096, {1, 0, 0, 0, 0, 0}, 3, 3);
prepare_utf8_data col_4_4_0(4096, {1, 0, 0, 0, 0, 0}, 4, 4);
prepare_utf8_data col_5_5_0(4096, {1, 0, 0, 0, 0, 0}, 5, 5);
prepare_utf8_data col_6_6_0(4096, {1, 0, 0, 0, 0, 0}, 6, 6);
prepare_utf8_data col_7_7_0(4096, {1, 0, 0, 0, 0, 0}, 7, 7);
prepare_utf8_data col_8_8_0(4096, {1, 0, 0, 0, 0, 0}, 8, 8);
prepare_utf8_data col_9_9_0(4096, {1, 0, 0, 0, 0, 0}, 9, 9);
prepare_utf8_data col_10_10_0(4096, {1, 0, 0, 0, 0, 0}, 10, 10);
prepare_utf8_data col_11_11_0(4096, {1, 0, 0, 0, 0, 0}, 11, 11);
prepare_utf8_data col_12_12_0(4096, {1, 0, 0, 0, 0, 0}, 12, 12);
prepare_utf8_data col_13_13_0(4096, {1, 0, 0, 0, 0, 0}, 13, 13);
prepare_utf8_data col_14_14_0(4096, {1, 0, 0, 0, 0, 0}, 14, 14);
prepare_utf8_data col_15_15_0(4096, {1, 0, 0, 0, 0, 0}, 15, 15);
prepare_utf8_data col_16_16_0(4096, {1, 0, 0, 0, 0, 0}, 16, 16);
prepare_utf8_data col_17_17_0(4096, {1, 0, 0, 0, 0, 0}, 17, 17);
prepare_utf8_data col_18_18_0(4096, {1, 0, 0, 0, 0, 0}, 18, 18);
prepare_utf8_data col_19_19_0(4096, {1, 0, 0, 0, 0, 0}, 19, 19);
prepare_utf8_data col_20_20_0(4096, {1, 0, 0, 0, 0, 0}, 20, 20);

std::vector<BinaryColumn*> columns0{nullptr,
                                    &col_1_1_0.binary_column,
                                    &col_2_2_0.binary_column,
                                    &col_3_3_0.binary_column,
                                    &col_4_4_0.binary_column,
                                    &col_5_5_0.binary_column,
                                    &col_6_6_0.binary_column,
                                    &col_7_7_0.binary_column,
                                    &col_8_8_0.binary_column,
                                    &col_9_9_0.binary_column,
                                    &col_10_10_0.binary_column,
                                    &col_11_11_0.binary_column,
                                    &col_12_12_0.binary_column,
                                    &col_13_13_0.binary_column,
                                    &col_14_14_0.binary_column,
                                    &col_15_15_0.binary_column,
                                    &col_16_16_0.binary_column,
                                    &col_17_17_0.binary_column,
                                    &col_18_18_0.binary_column,
                                    &col_19_19_0.binary_column,
                                    &col_20_20_0.binary_column};

std::vector<BinaryColumn*> columns1{nullptr,
                                    &col_1_1_1.binary_column,
                                    &col_2_2_1.binary_column,
                                    &col_3_3_1.binary_column,
                                    &col_4_4_1.binary_column,
                                    &col_5_5_1.binary_column,
                                    &col_6_6_1.binary_column,
                                    &col_7_7_1.binary_column,
                                    &col_8_8_1.binary_column,
                                    &col_9_9_1.binary_column,
                                    &col_10_10_1.binary_column,
                                    &col_11_11_1.binary_column,
                                    &col_12_12_1.binary_column,
                                    &col_13_13_1.binary_column,
                                    &col_14_14_1.binary_column,
                                    &col_15_15_1.binary_column,
                                    &col_16_16_1.binary_column,
                                    &col_17_17_1.binary_column,
                                    &col_18_18_1.binary_column,
                                    &col_19_19_1.binary_column,
                                    &col_20_20_1.binary_column};

template <size_t n0, size_t n1>
void batch_memcmp_optimized_const(std::vector<bool>& result) {
    auto col0 = columns0[n0];
    auto col1 = columns1[n1];
    const auto size = col0->size();
    for (auto i = 0; i < size; ++i) {
        auto s0 = col0->get_slice(i);
        auto s1 = col1->get_slice(i);
        result[i] = mem_equal_optimized(s0.data, n0, s1.data, n1);
    }
}

template <size_t n0, size_t n1>
void BM_batch_memcmp_optimized_const(benchmark::State& state) {
    auto col0 = columns0[n0];
    std::vector<bool> result;
    result.resize(col0->size());
    for (auto _ : state) {
        batch_memcmp_optimized_const<n0, n1>(result);
    }
}

template <size_t n0, size_t n1>
void batch_memcmp_optimized_variable(std::vector<bool>& result) {
    auto col0 = columns0[n0];
    auto col1 = columns1[n1];
    const auto size = col0->size();
    for (auto i = 0; i < size; ++i) {
        auto s0 = col0->get_slice(i);
        auto s1 = col1->get_slice(i);
        result[i] = mem_equal_optimized(s0.data, s0.size, s1.data, s1.size);
    }
}

template <size_t n0, size_t n1>
void BM_batch_memcmp_optimized_variable(benchmark::State& state) {
    auto col0 = columns0[n0];
    std::vector<bool> result;
    result.resize(col0->size());
    for (auto _ : state) {
        batch_memcmp_optimized_variable<n0, n1>(result);
    }
}

void BM_memcmp_5_10(benchmark::State& state) {
    const auto size = data0.size();
    std::vector<bool> result;
    result.resize(size);
    for (auto _ : state) {
        for (auto i = 0; i < size; ++i) {
            auto s0 = data0.get_slice(i);
            auto s1 = data1.get_slice(i);
            result[i] = mem_equal_memcpy(s0.data, s0.size, s1.data, s1.size);
        }
    }
}

void BM_memcmp_optimized_5_10(benchmark::State& state) {
    const auto size = data0.size();
    std::vector<bool> result;
    result.resize(size);
    for (auto _ : state) {
        for (auto i = 0; i < size; ++i) {
            auto s0 = data0.get_slice(i);
            auto s1 = data1.get_slice(i);
            result[i] = mem_equal_optimized(s0.data, s0.size, s1.data, s1.size);
        }
    }
}

void BM_memcmp_1_20(benchmark::State& state) {
    const auto size = data2.size();
    std::vector<bool> result;
    result.resize(size);
    for (auto _ : state) {
        for (auto i = 0; i < size; ++i) {
            auto s0 = data2.get_slice(i);
            auto s1 = data3.get_slice(i);
            result[i] = mem_equal_memcpy(s0.data, s0.size, s1.data, s1.size);
        }
    }
}
void BM_memcmp_optimized_1_20(benchmark::State& state) {
    const auto size = data2.size();
    std::vector<bool> result;
    result.resize(size);
    for (auto _ : state) {
        for (auto i = 0; i < size; ++i) {
            auto s0 = data2.get_slice(i);
            auto s1 = data3.get_slice(i);
            result[i] = mem_equal_optimized(s0.data, s0.size, s1.data, s1.size);
        }
    }
}

#if 0
BENCHMARK(BM_memcmp_5_10);
BENCHMARK(BM_memcmp_optimized_5_10);
BENCHMARK(BM_memcmp_1_20);
BENCHMARK(BM_memcmp_optimized_1_20);

BENCHMARK(BM_memcmp_5_10);
BENCHMARK(BM_memcmp_optimized_5_10);
BENCHMARK(BM_memcmp_1_20);
BENCHMARK(BM_memcmp_optimized_1_20);
#endif
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 15, 15);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 9, 9);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 17, 17);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 19, 19);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 18, 18);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 15, 15);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 16, 16);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 4, 4);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 13, 13);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 5, 5);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 3, 3);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 4, 4);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 19, 19);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 16, 16);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 7, 7);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 14, 14);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 3, 3);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 11, 11);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 17, 17);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 1, 1);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 6, 6);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 12, 12);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 6, 6);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 11, 11);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 12, 12);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 1, 1);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 2, 2);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 18, 18);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 13, 13);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 20, 20);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 14, 14);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 10, 10);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 8, 8);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 10, 10);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 2, 2);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 20, 20);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 5, 5);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_const, 9, 9);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 7, 7);
BENCHMARK_TEMPLATE(BM_batch_memcmp_optimized_variable, 8, 8);

BENCHMARK_MAIN();
