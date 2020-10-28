//
// Created by grakra on 2020/10/20.
//

#include <benchmark/benchmark.h>
#include <string_functions.hh>
prepare_utf8_data prepare;
auto &data = prepare.string_data;

void BM_substr_pos_len_old(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 10, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr_old(data, dst, 5, 10);
  }
}

void BM_substr_1_len_old(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 3, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr_old(data, dst, 1, 3);
  }
}

void BM_substr_pos_len_new2(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 10, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr_new(data, dst, 5, 10);
  }
}

void BM_substr_pos_len_check_ascii_new(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 10, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<true, true,false>(data, dst, 5, 10);
  }
}

void BM_substr_pos_len_check_ascii_lookup_table_new(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 10, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<true, true,true>(data, dst, 5, 10);
  }
}

void BM_substr_1_len_check_ascii_lookup_table_new(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 10, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<true, true,true>(data, dst, 1, 3);
  }
}

void BM_substr_pos_len_new(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 10, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<false, true,false>(data, dst, 5, 10);
  }
}

void BM_substr_neg_len_old(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 10, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr_old(data, dst, -5, 10);
  }
}

void BM_substr_neg_len_new2(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 10, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr_new(data, dst, -5, 10);
  }
}

void BM_substr_neg_len_check_ascii_new(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 10, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<true, true,false>(data, dst, -5, 10);
  }
}

void BM_substr_neg_len_check_ascii_lookup_table_new(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 10, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<true, true,true>(data, dst, -5, 10);
  }
}

void BM_substr_neg_len_new(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(std::min(data.size() * 10, dst.blob.size()));
  dst.offsets.reserve(data.size() + 1);

  for (auto _ : state) {
    StringFunctions::substr<false, true, false>(data, dst, -5, 10);
  }
}

void BM_substr_get_utf8_index_old(benchmark::State &state) {
  std::vector<size_t> index;
  for (auto _ : state) {
    const auto size = data.size();
    for (int i=0; i< size;++i){
      index.clear();
      StringFunctions::get_utf8_index(data.get_slice(i), &index);
    }
  }
}

void BM_substr_get_utf8_index_new(benchmark::State &state) {
  std::vector<size_t> index;
  for (auto _ : state) {
    const auto size = data.size();
    for (int i=0; i< size;++i){
      index.clear();
      StringFunctions::get_utf8_index2(data.get_slice(i), &index);
    }
  }
}

//BENCHMARK(BM_substr_get_utf8_index_new);
//BENCHMARK(BM_substr_get_utf8_index_old);
//BENCHMARK(BM_substr_get_utf8_index_new);
//BENCHMARK(BM_substr_get_utf8_index_old);

//BENCHMARK(BM_substr_neg_len_old);
//BENCHMARK(BM_substr_neg_len_check_ascii_new);
//BENCHMARK(BM_substr_neg_len_check_ascii_lookup_table_new);

//BENCHMARK(BM_substr_pos_len_old);
//BENCHMARK(BM_substr_pos_len_check_ascii_new);
//BENCHMARK(BM_substr_pos_len_check_ascii_lookup_table_new);

BENCHMARK(BM_substr_1_len_old);
BENCHMARK(BM_substr_1_len_check_ascii_lookup_table_new);

BENCHMARK_MAIN();