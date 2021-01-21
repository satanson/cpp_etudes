// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/10/20.
//

#include <benchmark/benchmark.h>
#include <string_functions.hh>
prepare_utf8_data prepare;
auto &data = prepare.data;
auto &src = prepare.binary_column;

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
  BinaryColumn dst;
  dst.bytes.resize(src.bytes.size());
  for (auto _ : state) {
    StringFunctions::upper_vector_copy_3_times(src, dst);
  }
}

void BM_upper_vector_copy_1_times(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.resize(src.bytes.size());
  dst.offsets.reserve(src.size() + 1);
  for (auto _ : state) {
    StringFunctions::upper_vector_copy_1_times(src, dst);
  }
}

void BM_upper_vector_copy_2_times(benchmark::State &state) {
  BinaryColumn dst;
  dst.bytes.reserve(src.bytes.size());
  dst.offsets.reserve(src.size() + 1);
  for (auto _ : state) {
    StringFunctions::upper_vector_copy_2_times(src, dst);
  }
}

void BM_upper_vector_new1(benchmark::State &state) {
  for (auto _ : state) {
    BinaryColumn dst;
    StringFunctions::upper_vector_new1(src, dst);
  }
}

void BM_lower_vector_new1(benchmark::State &state) {
  for (auto _ : state) {
    BinaryColumn dst;
    StringFunctions::lower_vector_new1(src, dst);
  }
}

template<bool use_raw>
void BM_upper_vector_new2(benchmark::State &state) {
  for (auto _ : state) {
    BinaryColumn dst;
    StringFunctions::upper_vector_new2<use_raw>(src, dst);
  }
}
template<bool use_raw>
void BM_lower_vector_new2(benchmark::State &state) {
  for (auto _ : state) {
    BinaryColumn dst;
    StringFunctions::lower_vector_new2<use_raw>(src, dst);
  }
}

void BM_upper_vector_old(benchmark::State &state) {
  for (auto _ : state) {
    BinaryColumn dst;
    StringFunctions::upper_vector_old(src, dst);
  }
}

void BM_lower_vector_old(benchmark::State &state) {
  for (auto _ : state) {
    BinaryColumn dst;
    StringFunctions::lower_vector_old(src, dst);
  }
}
void BM_no_shift(benchmark::State &state) {
  for (auto _ : state) {
    auto begin = src.bytes.data();
    auto end = begin + src.bytes.size();
    for (unsigned char *p = begin; p < end; ++p) {
      if ('A' <= *p && *p < 'Z') {
        *p = *p ^ 32;
      }
    }
  }
}
void BM_shift_bitand(benchmark::State &state) {
  for (auto _ : state) {
    auto begin = src.bytes.data();
    auto end = begin + src.bytes.size();
    for (unsigned char *p = begin; p < end; ++p) {
      *p = *p ^ (('A' <= *p & *p < 'Z') << 5);
    }
  }
}

void BM_shift_logicaland(benchmark::State &state) {
  for (auto _ : state) {
    auto begin = src.bytes.data();
    auto end = begin + src.bytes.size();
    for (unsigned char *p = begin; p < end; ++p) {
      *p = *p ^ (('A' <= *p & *p < 'Z') << 5);
    }
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
//BENCHMARK(BM_lower_vector_new1);
//BENCHMARK(BM_upper_vector_new1);
//BENCHMARK_TEMPLATE(BM_lower_vector_new2, true);
//BENCHMARK_TEMPLATE(BM_lower_vector_new2, false);
//BENCHMARK_TEMPLATE(BM_upper_vector_new2, true);
//BENCHMARK_TEMPLATE(BM_upper_vector_new2, false);
//BENCHMARK(BM_lower_vector_old);
//BENCHMARK(BM_upper_vector_old);
BENCHMARK(BM_no_shift);
BENCHMARK(BM_shift_bitand);
BENCHMARK(BM_shift_logicaland);
BENCHMARK_MAIN();
