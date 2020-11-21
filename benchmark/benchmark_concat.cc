// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com

//
// Created by grakra on 2020/11/19.
//

#include <benchmark/benchmark.h>
#include <concat.hh>
#include <string_functions.hh>
#include <iostream>
#include <memory>

struct prepare {
  prepare_utf8_data col_5_10;
  prepare_utf8_data col_10_20;
  prepare_utf8_data col_50_100;
  prepare_utf8_data col_100_500;
  Columns columns_10_10;
  Columns columns_10_20;
  Columns columns_10_100;
  Columns columns_10_500;
  Columns columns_20_100;
  Columns columns_20_500;
  Columns columns_100_500;
  std::unordered_map<uint32_t, Columns *> columns_map;
  prepare() :
      col_5_10(4096, {1, 0, 0, 0, 0, 0}, 5, 10),
      col_10_20(4096, {1, 0, 0, 0, 0, 0}, 10, 20),
      col_50_100(4096, {1, 0, 0, 0, 0, 0}, 50, 100),
      col_100_500(4096, {1, 0, 0, 0, 0, 0}, 100, 500),
      columns_10_10{
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_5_10.binary_column)),
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_5_10.binary_column))
      },
      columns_10_20{
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_5_10.binary_column)),
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_10_20.binary_column))
      },
      columns_10_100{
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_5_10.binary_column)),
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_50_100.binary_column))
      },
      columns_10_500{
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_5_10.binary_column)),
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_100_500.binary_column))
      },
      columns_20_100{

          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_10_20.binary_column)),
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_50_100.binary_column))
      },
      columns_20_500{
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_10_20.binary_column)),
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_100_500.binary_column))

      },
      columns_100_500{

          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_50_100.binary_column)),
          std::shared_ptr<BinaryColumn>(new BinaryColumn(col_100_500.binary_column))
      } {
    columns_map[(10 << 16) + 10] = &columns_10_10;
    columns_map[(10 << 16) + 20] = &columns_10_20;
    columns_map[(10 << 16) + 100] = &columns_10_100;
    columns_map[(10 << 16) + 500] = &columns_10_500;
    columns_map[(20 << 16) + 100] = &columns_20_100;
    columns_map[(20 << 16) + 500] = &columns_20_500;
    columns_map[(100 << 16) + 500] = &columns_100_500;
  }
} data;

void BM_concat(benchmark::State &state) {
  uint32_t key = (state.range(0) << 16) + state.range(1);

  Columns &columns = *(data.columns_map[key]);
  for (auto _:state) {
    concat_horizontally(columns);
  }
}

void BM_group_by_multi_column_concat(benchmark::State &state) {
  uint32_t key = (state.range(0) << 16) + state.range(1);
  Columns &columns = *(data.columns_map[key]);
  std::vector<uint32_t> slice_sizes;
  std::vector<uint8_t> buffer;
  for (auto _:state) {
    concat_vertically(columns, buffer, slice_sizes);
  }
}

BENCHMARK(BM_concat)
    ->Args({10, 10})
    ->Args({10, 20})
    ->Args({10, 100})
    ->Args({10, 500})
    ->Args({20, 100})
    ->Args({20, 500})
    ->Args({100, 500});

BENCHMARK(BM_group_by_multi_column_concat)
    ->Args({10, 10})
    ->Args({10, 20})
    ->Args({10, 100})
    ->Args({10, 500})
    ->Args({20, 100})
    ->Args({20, 500})
    ->Args({100, 500});
BENCHMARK_MAIN();
