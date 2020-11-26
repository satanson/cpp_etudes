//
// Created by grakra on 2020/11/26.
//

#include <include/string_functions.hh>
#include <memequal.hh>
#include <benchmark/benchmark.h>
prepare_utf8_data col_5_10_0(4096, {1, 0, 0, 0, 0, 0}, 5, 10);
prepare_utf8_data col_5_10_1(4096, {1, 0, 0, 0, 0, 0}, 5, 10);

prepare_utf8_data col_1_20_0(4096, {1, 0, 0, 0, 0, 0}, 1, 20);
prepare_utf8_data col_1_20_1(4096, {1, 0, 0, 0, 0, 0}, 1, 20);

auto data0 =  col_5_10_0.binary_column;
auto data1 =  col_5_10_1.binary_column;

auto data2 =  col_1_20_0.binary_column;
auto data3 =  col_1_20_1.binary_column;

void BM_memcmp_5_10(benchmark::State& state){
  const auto size = data0.size();
  std::vector<bool> result;
  result.resize(size);
  for (auto _:state){
    for (auto i=0; i<size; ++i){
      auto s0 = data0.get_slice(i);
      auto s1 = data1.get_slice(i);
      result[i] = mem_equal_memcpy(s0.data, s0.size, s1.data, s1.size);
    }
  }
}
void BM_memcmp_optimized_5_10(benchmark::State& state){
  const auto size = data0.size();
  std::vector<bool> result;
  result.resize(size);
  for (auto _:state){
    for (auto i=0; i<size; ++i){
      auto s0 = data0.get_slice(i);
      auto s1 = data1.get_slice(i);
      result[i] = mem_equal_optimized(s0.data, s0.size, s1.data, s1.size);
    }
  }
}

void BM_memcmp_1_20(benchmark::State& state){
  const auto size = data2.size();
  std::vector<bool> result;
  result.resize(size);
  for (auto _:state){
    for (auto i=0; i<size; ++i){
      auto s0 = data2.get_slice(i);
      auto s1 = data3.get_slice(i);
      result[i] = mem_equal_memcpy(s0.data, s0.size, s1.data, s1.size);
    }
  }
}
void BM_memcmp_optimized_1_20(benchmark::State& state){
  const auto size = data2.size();
  std::vector<bool> result;
  result.resize(size);
  for (auto _:state){
    for (auto i=0; i<size; ++i){
      auto s0 = data2.get_slice(i);
      auto s1 = data3.get_slice(i);
      result[i] = mem_equal_optimized(s0.data, s0.size, s1.data, s1.size);
    }
  }
}
BENCHMARK(BM_memcmp_5_10);
BENCHMARK(BM_memcmp_optimized_5_10);
BENCHMARK(BM_memcmp_1_20);
BENCHMARK(BM_memcmp_optimized_1_20);

BENCHMARK(BM_memcmp_5_10);
BENCHMARK(BM_memcmp_optimized_5_10);
BENCHMARK(BM_memcmp_1_20);
BENCHMARK(BM_memcmp_optimized_1_20);

BENCHMARK_MAIN();
