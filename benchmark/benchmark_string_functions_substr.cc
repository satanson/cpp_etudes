//
// Created by grakra on 2020/10/20.
//

#include <benchmark/benchmark.h>
#include <string_functions.hh>
#include <memory>
prepare_utf8_data prepare;
auto &data = prepare.binary_column;

void BM_substr_pos_len_old(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 10, dst.bytes.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr_old(data, dst, 5, 10);
  }
}

void BM_substr_1_len_old(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 3, dst.bytes.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr_old(data, dst, 1, 3);
  }
}

void BM_substr_pos_len_new2(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 10, dst.bytes.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr_new(data, dst, 5, 10);
  }
}

void BM_substr_pos_len_check_ascii_new(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 10, dst.bytes.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<true, true,false>(data, dst, 5, 10);
  }
}

void BM_substr_pos_len_check_ascii_lookup_table_new(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 10, dst.bytes.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<true, true,true>(data, dst, 5, 10);
  }
}

void BM_substr_1_len_check_ascii_lookup_table_new(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 10, dst.bytes.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<true, true,true>(data, dst, 1, 3);
  }
}

void BM_substr_pos_len_new(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 10, dst.bytes.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<false, true,false>(data, dst, 5, 10);
  }
}

void BM_substr_neg_len_old(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 10, dst.bytes.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr_old(data, dst, -5, 10);
  }
}

void BM_substr_neg_len_new2(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 10, dst.bytes.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr_new(data, dst, -5, 10);
  }
}

void BM_substr_neg_len_check_ascii_new(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 10, dst.bytes.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<true, true,false>(data, dst, -5, 10);
  }
}

void BM_substr_neg_len_check_ascii_lookup_table_new(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 10, dst.bytes.size()));
  dst.offsets.reserve(data.size() + 1);
  for (auto _ : state) {
    StringFunctions::substr<true, true,true>(data, dst, -5, 10);
  }
}

void BM_substr_neg_len_new(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(std::min(data.size() * 10, dst.bytes.size()));
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

void BM_vector_insert_uint8_uint8(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<uint8_t> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), (uint8_t*)s.begin(), (uint8_t*)s.begin()+3);
    }
  }
}

void BM_vector_insert_uint8_int8(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<uint8_t> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), (int8_t*)s.begin(), (int8_t*)s.begin()+3);
    }
  }
}

void BM_vector_insert_uint8_default(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<uint8_t> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), s.begin(), s.begin()+3);
    }
  }
}

void BM_vector_insert_uint8_char(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<uint8_t> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), (char*)s.begin(), (char*)s.begin()+3);
    }
  }
}

void BM_vector_insert_int8_uint8(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<int8_t> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), (uint8_t*)s.begin(), (uint8_t*)s.begin()+3);
    }
  }
}

void BM_vector_insert_int8_int8(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<int8_t> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), (int8_t*)s.begin(), (int8_t*)s.begin()+3);
    }
  }
}

void BM_vector_insert_int8_default(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<int8_t> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), s.begin(), s.begin()+3);
    }
  }
}

void BM_vector_insert_int8_char(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes = data.bytes.size();
    std::vector<int8_t> result;
    result.reserve(bytes);
    for (size_t i = 0; i < data.size(); ++i) {
      Slice s = data.get_slice(i);
      result.insert(result.end(), (char *) s.begin(), (char *) s.begin() + 3);
    }
  }
}
void BM_vector_insert_char_uint8(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<char> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), (uint8_t*)s.begin(), (uint8_t*)s.begin()+3);
    }
  }
}

void BM_vector_insert_char_int8(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<char> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), (int8_t*)s.begin(), (int8_t*)s.begin()+3);
    }
  }
}

void BM_vector_insert_char_default(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<char> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), s.begin(), s.begin()+3);
    }
  }
}

void BM_vector_insert_char_char(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<char> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      result.insert(result.end(), (char*)s.begin(), (char*)s.begin()+3);
    }
  }
}


template <typename T>
using Buffer = std::vector<T>;
using Bytes = Buffer<uint8_t>;

template<typename T,typename U>
void BM_vector_insert(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    std::vector<T> result;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      if constexpr(std::is_void_v<U>) {
        result.insert(result.end(), s.begin(),  s.begin() + 3);
      } else{
        result.insert(result.end(), (U*)s.begin(),  (U*)s.begin() + 3);
      }
    }
  }
}

template<typename T>
void BM_Bytes_insert(benchmark::State &state) {
  for (auto _ : state) {
    const auto bytes= data.bytes.size();
    auto underlying=std::make_unique<Bytes>();
    Bytes& result=*underlying;
    result.reserve(bytes);
    for (size_t i=0; i < data.size(); ++i){
      Slice s = data.get_slice(i);
      if constexpr(std::is_void_v<T>) {
        result.insert(result.end(), s.begin(),  s.begin() + 3);
      } else{
        result.insert(result.end(), (T*)s.begin(),  (T*)s.begin() + 3);
      }
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
BENCHMARK_TEMPLATE(BM_vector_insert, uint8_t, int8_t);
BENCHMARK_TEMPLATE(BM_vector_insert, uint8_t, uint8_t);
BENCHMARK_TEMPLATE(BM_vector_insert, uint8_t, void);
BENCHMARK_TEMPLATE(BM_vector_insert, uint8_t, char);

BENCHMARK_TEMPLATE(BM_vector_insert, int8_t, int8_t);
BENCHMARK_TEMPLATE(BM_vector_insert, int8_t, uint8_t);
BENCHMARK_TEMPLATE(BM_vector_insert, int8_t, void);
BENCHMARK_TEMPLATE(BM_vector_insert, int8_t, char);

BENCHMARK_TEMPLATE(BM_vector_insert, char, int8_t);
BENCHMARK_TEMPLATE(BM_vector_insert, char, uint8_t);
BENCHMARK_TEMPLATE(BM_vector_insert, char, void);
BENCHMARK_TEMPLATE(BM_vector_insert, char, char);

BENCHMARK_TEMPLATE(BM_Bytes_insert,  int8_t);
BENCHMARK_TEMPLATE(BM_Bytes_insert,  uint8_t);
BENCHMARK_TEMPLATE(BM_Bytes_insert,  void);
BENCHMARK_TEMPLATE(BM_Bytes_insert,  char);
BENCHMARK_MAIN();