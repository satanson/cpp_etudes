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

void BM_ascii_substr_by_ref(benchmark::State &state) {
  for (auto _ : state) {
    const auto size = data.size();
    std::string bytes;
    std::vector<int> offsets;
    bytes.reserve(data.size()*3);
    offsets.resize(data.size()+1);
    StringFunctions::ascii_substr_by_ref<false>(data, bytes, offsets, 1,3);
  }
}

void BM_ascii_substr_by_ptr(benchmark::State &state) {
  for (auto _ : state) {
    const auto size = data.size();
    std::string bytes;
    std::vector<int> offsets;
    bytes.reserve(data.size()*3);
    offsets.resize(data.size()+1);
    StringFunctions::ascii_substr_by_ptr<false>(&data, &bytes, &offsets, 1,3);
  }
}

void BM_vector_insert_a(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.blob.size();
    std::vector<uint8_t> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), (uint8_t*)s.begin(), (uint8_t*)s.begin()+3);
    }
  }
}

void BM_vector_insert_b(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.blob.size();
    std::vector<uint8_t> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), s.begin(), s.begin()+3);
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

//BENCHMARK(BM_substr_1_len_old);
//BENCHMARK(BM_substr_1_len_check_ascii_lookup_table_new);
//BENCHMARK(BM_ascii_substr_by_ref);
//BENCHMARK(BM_ascii_substr_by_ptr);
BENCHMARK(BM_vector_insert_a);
BENCHMARK(BM_vector_insert_b);
BENCHMARK(BM_vector_insert_a);
BENCHMARK(BM_vector_insert_a);
BENCHMARK(BM_vector_insert_a);
BENCHMARK(BM_vector_insert_b);
BENCHMARK(BM_vector_insert_b);
BENCHMARK(BM_vector_insert_b);
BENCHMARK(BM_vector_insert_b);

BENCHMARK_MAIN();