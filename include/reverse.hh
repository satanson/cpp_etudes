// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/9.
//

#ifndef CPP_ETUDES_INCLUDE_REVERSE_HH_
#define CPP_ETUDES_INCLUDE_REVERSE_HH_
#include <algorithm>
#include <cstdint>
#include <immintrin.h>

// SIZE: 256 * uint8_t
static const uint8_t UTF8_BYTE_LENGTH_TABLE[256] = {
    // start byte of 1-byte utf8 char: 0b0000'0000 ~ 0b0111'1111
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1,
    // continuation byte: 0b1000'0000 ~ 0b1011'1111
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    // start byte of 2-byte utf8 char: 0b1100'0000 ~ 0b1101'1111
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2,
    // start byte of 3-byte utf8 char: 0b1110'0000 ~ 0b1110'1111
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    // start byte of 4-byte utf8 char: 0b1111'0000 ~ 0b1111'0111
    // invalid utf8 byte: 0b1111'1000~ 0b1111'1111
    4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1};

static inline const char *utf8_reverse_per_slice(const char *src_begin,
                                                 const char *src_end,
                                                 const char *dst_begin) {
  auto p = src_begin;
  auto const size = src_end - src_begin;
  char *q = (char *)dst_begin + size;
  for (auto char_size = 0; p < src_end; p += char_size) {
    char_size = UTF8_BYTE_LENGTH_TABLE[(uint8_t)*p];
    q -= char_size;
    std::copy(p, p + char_size, q);
  }
  return dst_begin;
}

template <bool is_epi8>
static inline const char *ascii_reverse(const char *src_begin,
                                        const char *src_end,
                                        const char *dst_begin) {
  auto p = src_begin;
  auto const size = src_end - src_begin;
  char *q = (char *)dst_begin + size;
#if defined(__SSSE3__) && defined(__SSE2__)
  if constexpr (is_epi8) {
    constexpr auto SSE_SIZE = sizeof(__m64);
    constexpr auto ctrl_masks = 0x00'01'02'03'04'05'06'07ull;
    const auto sse_end = src_begin + (size & ~(SSE_SIZE - 1));
    for (; p < sse_end; p += SSE_SIZE) {
      q -= SSE_SIZE;
      *(__m64 *)q = _mm_shuffle_pi8(*(__m64 *)p, (__m64)ctrl_masks);
    }
  } else {
    constexpr auto SSE2_SIZE = sizeof(__m128i);
    const auto ctrl_masks = _mm_set_epi64((__m64)0x00'01'02'03'04'05'06'07ull,
                                          (__m64)0x08'09'0a'0b'0c'0d'0e'0full);
    const auto sse2_end = src_begin + (size & ~(SSE2_SIZE - 1));
    for (; p < sse2_end; p += SSE2_SIZE) {
      q -= SSE2_SIZE;
      _mm_storeu_si128(
          (__m128i *)q,
          _mm_shuffle_epi8(_mm_loadu_si128((__m128i *)p), ctrl_masks));
    }
  }
#endif
  for (; p < src_end; ++p) {
    --q;
    *q = *p;
  }
  return dst_begin;
}

#endif // CPP_ETUDES_INCLUDE_REVERSE_HH_
