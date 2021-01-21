// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/10.
//

#ifndef CPP_ETUDES_INCLUDE_REPEAT_HH_
#define CPP_ETUDES_INCLUDE_REPEAT_HH_
#include <cstdint>
#include <utility>
#include <string>
#include <cstring>
#include <raw_container.hh>
#include <immintrin.h>
#include <clickhouse/FastMemcpy.h>

template<typename InitFunc, typename DeltaFunc, typename...Args>
void fast_repeat(int num, InitFunc init_func, DeltaFunc delta_func, Args &&... args) {
  int k = 0;
  int last_bit = num & 1;
  num >>= 1;
  init_func(1, std::forward<Args>(args)...);
  for (; num > 0; k += 1, last_bit = num & 1, num >>= 1) {
    int32_t n = 1 << (k + last_bit);
    delta_func(n, std::forward<Args>(args)...);
  }
}

static inline void count_n_init(int i, int &m) {
  m = i;
}

static inline void count_n_delta(int n, int &m) {
  m += n;
}

int count_n(int num) {
  int m;
  fast_repeat(num, count_n_init, count_n_delta, m);
  return m;
}

// The standard memcpy operation is slow for variable small sizes.
// This implementation inlines the optimal realization for sizes 1 to 16.
// To avoid code bloat don't use it in case of not performance-critical spots,
// nor when you don't expect very frequent values of size <= 16.
inline void memcpy_inlined(void *dst, const void *src, size_t size) {
  // Compiler inlines code with minimal amount of data movement when third
  // parameter of memcpy is a constant.
  switch (size) {
    case 1: memcpy(dst, src, 1);
      break;
    case 2: memcpy(dst, src, 2);
      break;
    case 3: memcpy(dst, src, 3);
      break;
    case 4: memcpy(dst, src, 4);
      break;
    case 5: memcpy(dst, src, 5);
      break;
    case 6: memcpy(dst, src, 6);
      break;
    case 7: memcpy(dst, src, 7);
      break;
    case 8: memcpy(dst, src, 8);
      break;
    case 9: memcpy(dst, src, 9);
      break;
    case 10: memcpy(dst, src, 10);
      break;
    case 11: memcpy(dst, src, 11);
      break;
    case 12: memcpy(dst, src, 12);
      break;
    case 13: memcpy(dst, src, 13);
      break;
    case 14: memcpy(dst, src, 14);
      break;
    case 15: memcpy(dst, src, 15);
      break;
    case 16: memcpy(dst, src, 16);
      break;
    default: memcpy(dst, src, size);
      break;
  }
}

inline void simd_memcpy_inline(void *dst, const void *src, size_t size) {
  void *end = (char *) src + size;
#if defined (__SSE2__)
  constexpr auto SSE2_SIZE = sizeof(__m128i);
  const void *sse2_end = (char *) src + (size & ~(SSE2_SIZE - 1));
  for (;
      src < sse2_end;
      src = (char *) src + SSE2_SIZE, dst = (char *) dst + SSE2_SIZE) {
    _mm_storeu_si128((__m128i *) dst, _mm_loadu_si128((__m128i *) src));
  }
#endif
  for (; src < end; src = (char *) src + 1, dst = (char *) dst + 1) {
    *(char *) dst = *(char *) src;
  }
}

inline void simd_memcpy_inline_memcpy_rest(void *dst, const void *src, size_t size) {
  char *end = (char *) src + size;
#if defined (__SSE2__)
  constexpr auto SSE2_SIZE = sizeof(__m128i);
  const void *sse2_end = (char *) src + (size & ~(SSE2_SIZE - 1));
  for (;
      src < sse2_end;
      src = (char *) src + SSE2_SIZE, dst = (char *) dst + SSE2_SIZE) {
    _mm_storeu_si128((__m128i *) dst, _mm_loadu_si128((__m128i *) src));
  }
#endif
  memcpy(dst, src, end - (char *) src);
}

inline void simd_memcpy_inline_gutil_memcpy_rest(void *dst, const void *src, size_t size) {
#if defined (__SSE2__)
  constexpr auto SSE2_SIZE = sizeof(__m128i);
  const void *sse2_end = (char *) src + (size & ~(SSE2_SIZE - 1));
  for (;
      src < sse2_end;
      src = (char *) src + SSE2_SIZE, dst = (char *) dst + SSE2_SIZE) {
    _mm_storeu_si128((__m128i *) dst, _mm_loadu_si128((__m128i *) src));
  }
#endif
  memcpy_inlined(dst, src, size & (SSE2_SIZE - 1));
}

std::string repeat_string_logn(std::string const &s, int n) {
  std::string result;
  const auto s_size = s.size();
  result.resize(s_size * n);
  int k = 0;
  int last_bit = n & 1;
  n >>= 1;
  result.insert(result.end(), s.begin(), s.end());
  for (; n > 0; k += 1, last_bit = n & 1, n >>= 1) {
    int32_t len = s_size * (1 << k);
    result.insert(result.end(), result.data(), result.data() + len);
    if (last_bit) {
      result.insert(result.end(), result.data(), result.data() + len);
    }
  }
  return result;
}
typedef uint8_t UInt8;
typedef uint64_t UInt64;
static void fast_repeat(const UInt8 *src, UInt8 *dst, UInt64 size, UInt64 repeat_time) {
  if (__builtin_expect(repeat_time <= 0, 0)) {
    *dst = 0;
    return;
  }

  size -= 1;
  UInt64 k = 0;
  UInt64 last_bit = repeat_time & 1;
  repeat_time >>= 1;

  const UInt8 *dst_hdr = dst;
  memcpy_fast(dst, src, size);
  dst += size;

  while (repeat_time > 0) {
    UInt64 cpy_size = size * (1 << k);
    memcpy_fast(dst, dst_hdr, cpy_size);
    dst += cpy_size;
    if (last_bit) {
      memcpy_fast(dst, dst_hdr, cpy_size);
      dst += cpy_size;
    }
    k += 1;
    last_bit = repeat_time & 1;
    repeat_time >>= 1;
  }
  *dst = 0;
}

static void original_repeat(const UInt8 *src, UInt8 *dst, UInt64 size, UInt64 repeat_time) {
  for (UInt64 i = 0; i < repeat_time; ++i){
    memcpy_fast(dst, src, size-1);
    dst += size -1;
  }
  *dst = 0;
}

std::string repeat_string_logn_gutil_memcpy_inline(std::string const &s, int n) {
  std::string result;
  const auto s_size = s.size();
  raw::make_room(result, s_size * n);
  int k = 0;
  int last_bit = n & 1;
  n >>= 1;
  char *result_begin = result.data();
  char *result_curr = result_begin;
  const char *s_begin = s.data();
  memcpy_inlined(result_curr, s_begin, s_size);
  result_curr += s_size;
  for (; n > 0; k += 1, last_bit = n & 1, n >>= 1) {
    int32_t len = s_size * (1 << k);
    memcpy_inlined(result_curr, result_begin, len);
    result_curr += len;
    if (last_bit) {
      memcpy_inlined(result_curr, result_begin, len);
      result_curr += len;
    }
  }
  return result;
}

std::string repeat_string_logn_simd_memcpy_inline_1(std::string const &s, int n) {
  std::string result;
  const auto s_size = s.size();
  raw::make_room(result, s_size * n);
  int k = 0;
  int last_bit = n & 1;
  n >>= 1;
  char *result_begin = result.data();
  char *result_curr = result_begin;
  const char *s_begin = s.data();
  simd_memcpy_inline_gutil_memcpy_rest(result_curr, s_begin, s_size);
  result_curr += s_size;
  for (; n > 0; k += 1, last_bit = n & 1, n >>= 1) {
    int32_t len = s_size * (1 << k);
    simd_memcpy_inline_gutil_memcpy_rest(result_curr, result_begin, len);
    result_curr += len;
    if (last_bit) {
      simd_memcpy_inline_gutil_memcpy_rest(result_curr, result_begin, len);
      result_curr += len;
    }
  }
  return result;
}

std::string repeat_string_logn_memcpy(std::string const &s, int n) {
  std::string result;
  const auto s_size = s.size();
  raw::make_room(result, s_size * n);
  int k = 0;
  int last_bit = n & 1;
  n >>= 1;
  char *result_begin = result.data();
  char *result_curr = result_begin;
  const char *s_begin = s.data();
  memcpy(result_curr, s_begin, s_size);
  result_curr += s_size;
  for (; n > 0; k += 1, last_bit = n & 1, n >>= 1) {
    int32_t len = s_size * (1 << k);
    memcpy(result_curr, result_begin, len);
    result_curr += len;
    if (last_bit) {
      memcpy(result_curr, result_begin, len);
      result_curr += len;
    }
  }
  return result;
}

std::string repeat_string_logn_simd_memcpy_inline_2(std::string const &s, int n) {
  std::string result;
  const auto s_size = s.size();
  raw::make_room(result, s_size * n);
  int k = 0;
  int last_bit = n & 1;
  n >>= 1;
  char *result_begin = result.data();
  char *result_curr = result_begin;
  const char *s_begin = s.data();
  simd_memcpy_inline_memcpy_rest(result_curr, s_begin, s_size);
  result_curr += s_size;
  for (; n > 0; k += 1, last_bit = n & 1, n >>= 1) {
    int32_t len = s_size * (1 << k);
    simd_memcpy_inline_memcpy_rest(result_curr, result_begin, len);
    result_curr += len;
    if (last_bit) {
      simd_memcpy_inline_memcpy_rest(result_curr, result_begin, len);
      result_curr += len;
    }
  }
  return result;
}

std::string repeat_string_n(std::string const &s, int n) {
  std::string result;
  const auto s_size = s.size();
  result.reserve(s_size * n);
  for (auto i = 0; i < n; ++i) {
    result.insert(result.end(), s.begin(), s.end());
  }
  return result;
}

#endif //CPP_ETUDES_INCLUDE_REPEAT_HH_
