//
// Created by grakra on 2020/10/20.
//

#include <benchmark/benchmark.h>
#include <string_functions.hh>
prepare_utf8_data prepare;
auto &data = prepare.data;
auto &src = prepare.string_data;

void BM_lower_old(benchmark::State &state) {
  std::vector<std::string> res;
  res.reserve(data.size());
  for (auto _ : state) {
    for (auto &s: data) {
      res.push_back(StringFunctions::lower_old(s));
    }
  }
}

void BM_lower_old2(benchmark::State &state) {
  std::vector<std::string> res;
  res.reserve(data.size());
  for (auto _ : state) {
    for (auto &s: data) {
      res.push_back(StringFunctions::lower_old2(s));
    }
  }
}

void BM_lower_new(benchmark::State &state) {
  std::vector<std::string> res;
  res.reserve(data.size());
  for (auto _ : state) {
    for (auto &s: data) {
      res.push_back(StringFunctions::lower_new(s));
    }
  }
}

void BM_upper_old(benchmark::State &state) {
  std::vector<std::string> res;
  res.reserve(data.size());
  for (auto _ : state) {
    for (auto &s: data) {
      res.push_back(StringFunctions::upper_old(s));
    }
  }
}

void BM_upper_old2(benchmark::State &state) {
  std::vector<std::string> res;
  res.reserve(data.size());
  for (auto _ : state) {
    for (auto &s: data) {
      res.push_back(StringFunctions::upper_old2(s));
    }
  }
}
void BM_upper_new(benchmark::State &state) {
  std::vector<std::string> res;
  res.reserve(data.size());
  for (auto _ : state) {
    for (auto &s: data) {
      res.push_back(StringFunctions::upper_new(s));
    }
  }
}

void BM_lower_dummy(benchmark::State &state) {
  std::vector<std::string> res;
  res.reserve(data.size());
  for (auto _ : state) {
    for (auto &s: data) {
      res.push_back(StringFunctions::lower_dummy(s));
    }
  }
}

void BM_lower_dummy2(benchmark::State &state) {
  std::vector<std::string> res;
  res.reserve(data.size());
  for (auto _ : state) {
    for (auto &s: data) {
      res.push_back(StringFunctions::lower_dummy2(s));
    }
  }
}

void BM_lower_dummy3(benchmark::State &state) {
  std::vector<std::string> res;
  res.reserve(data.size());
  for (auto _ : state) {
    for (auto &s: data) {
      res.push_back(StringFunctions::lower_dummy3(s));
    }
  }
}

void BM_upper_vector_copy_3_times(benchmark::State &state) {
  StringVector dst;
  dst.blob.resize(src.blob.size());
  for (auto _ : state) {
    StringFunctions::upper_vector_copy_3_times (src, dst);
  }
}

void BM_upper_vector_copy_1_times(benchmark::State &state) {
  StringVector dst;
  dst.blob.resize(src.blob.size());
  dst.offsets.reserve(src.size()+1);
  for (auto _ : state) {
    StringFunctions::upper_vector_copy_1_times(src, dst);
  }
}

void BM_upper_vector_copy_2_times(benchmark::State &state) {
  StringVector dst;
  dst.blob.reserve(src.blob.size());
  dst.offsets.reserve(src.size()+1);
  for (auto _ : state) {
    StringFunctions::upper_vector_copy_2_times(src, dst);
  }
}

void BM_upper_vector_new(benchmark::State &state){
  StringVector dst;
  for (auto _ : state) {
    StringFunctions::upper_vector_new(src, dst);
  }
}

void BM_lower_vector_new(benchmark::State &state){
  StringVector dst;
  for (auto _ : state) {
    StringFunctions::lower_vector_new(src, dst);
  }
}

void BM_upper_vector_old(benchmark::State &state){
  for (auto _ : state) {
    StringVector dst;
    StringFunctions::upper_vector_old(src, dst);
  }
}

void BM_lower_vector_old(benchmark::State &state){
  for (auto _ : state) {
    StringVector dst;
    StringFunctions::lower_vector_old(src, dst);
  }
}

/*
BENCHMARK(BM_lower_old);
BENCHMARK(BM_lower_old2);
BENCHMARK(BM_lower_new);
BENCHMARK(BM_upper_old);
BENCHMARK(BM_upper_old2);
BENCHMARK(BM_upper_new);

BENCHMARK(BM_upper_vector_copy_3_times);
BENCHMARK(BM_upper_vector_copy_1_times);
BENCHMARK(BM_upper_vector_copy_2_times);
*/
BENCHMARK(BM_lower_vector_new);
BENCHMARK(BM_upper_vector_new);
BENCHMARK(BM_lower_vector_old);
BENCHMARK(BM_upper_vector_old);

BENCHMARK_MAIN();