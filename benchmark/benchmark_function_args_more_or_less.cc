//
// Created by grakra on 2020/11/4.
//

//
// Created by grakra on 2020/7/3.
//

#include <benchmark/benchmark.h>
#include <function_args_more_or_less.hh>

void append_value_two() {
  ColumnViewer viewer;
  ColumnBuilder builder;
  builder.reserve();
  for (int j = 0; j < 4000; ++j) {
    builder.append_value(viewer.value(j), viewer.is_null(j));
  }
  builder.reset();
}

void append_value_one() {
  ColumnViewer viewer;
  ColumnBuilder builder;
  builder.reserve();
  for (int j = 0; j < 4000; ++j) {
    builder.append_value(viewer.value(j));
  }
  builder.reset();
}

static void BM_append_value_one(benchmark::State &state) {
  for (auto _ : state)
    append_value_one();
}
static void BM_append_value_two(benchmark::State &state) {
  for (auto _ : state)
    append_value_two();
}
BENCHMARK(BM_append_value_one);

BENCHMARK(BM_append_value_two);

BENCHMARK_MAIN();