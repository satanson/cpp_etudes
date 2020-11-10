//
// Created by grakra on 2020/11/9.
//

#ifndef CPP_ETUDES_INCLUDE_UTIL_TRIM_HH_
#define CPP_ETUDES_INCLUDE_UTIL_TRIM_HH_
#include <immintrin.h>
#include <cstdint>
template <bool simd_optimization>
static inline const char* skip_leading_spaces(const char* begin, const char* end) {
  auto p = begin;
#if defined(__SSE2__)
  if constexpr (simd_optimization) {
    const auto size = end - begin;
    const auto SSE2_BYTES = sizeof(__m128i);
    const auto sse2_end = begin + (size & ~(SSE2_BYTES - 1));
    const auto spaces = _mm_set1_epi8(' ');
    for (; p < sse2_end; p += SSE2_BYTES) {
      uint32_t masks =
          _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128((__m128i*)p), spaces));
      int pos = __builtin_ctz((1u << SSE2_BYTES) | ~masks);
      if (pos < SSE2_BYTES) {
        return p + pos;
      }
    }
  }
#endif
  for (; p < end && *p == ' '; ++p) {
  }
  return p;
}

template <bool simd_optimization>
static const char* skip_trailing_spaces(const char* begin, const char* end) {
  auto p = end;
#if defined(__SSE2__)
  if constexpr (simd_optimization) {
    const auto size = end - begin;
    const auto SSE2_BYTES = sizeof(__m128i);
    const auto sse2_begin = end - (size & ~(SSE2_BYTES - 1));
    const auto spaces = _mm_set1_epi8(' ');
    for (p = end - SSE2_BYTES; p >= sse2_begin; p -= SSE2_BYTES) {
      uint32_t masks =
          _mm_movemask_epi8(_mm_cmpeq_epi8(_mm_loadu_si128((__m128i*)p), spaces));
      int pos = __builtin_clz(~(masks<<SSE2_BYTES));
      std::cout<<"pos="<<pos<<std::endl;
      if (pos < SSE2_BYTES) {
        return p + SSE2_BYTES - pos;
      }
    }
    p += SSE2_BYTES;
  }
#endif
  std::cout<<"end-p="<<end-p<<std::endl;
  for (--p; p >= begin && *p == ' '; --p) {
  }
  return p + 1;
}

#endif //CPP_ETUDES_INCLUDE_UTIL_TRIM_HH_
