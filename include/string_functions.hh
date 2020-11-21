// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com

//
// Created by grakra on 2020/10/20.
//

#ifndef CPP_ETUDES_INCLUDE_STRING_FUNCTIONS_HH_
#define CPP_ETUDES_INCLUDE_STRING_FUNCTIONS_HH_

#include<string>
#include<climits>
#include<vector>
#include<cassert>
#include<random>
#include<iostream>
#include <immintrin.h>
#include <default_init_allocator.hh>
#include <binary_column.hh>
static uint8_t *create_utf8_length_table() {
  uint8_t *tbl = new uint8_t[257];
  for (int byte = 0; byte < 257; ++byte) {
    if (byte >= 0xFC) {
      tbl[byte] = 6;
    } else if (byte >= 0xF8) {
      tbl[byte] = 5;
    } else if (byte >= 0xF0) {
      tbl[byte] = 4;
    } else if (byte >= 0xE0) {
      tbl[byte] = 3;
    } else if (byte >= 0xC0) {
      tbl[byte] = 2;
    } else {
      tbl[byte] = 1;
    }
  }

  return tbl;
}

static uint8_t *create_utf8_length_table2() {
  struct uint8_array {
    alignas(128) uint8_t tbl[64];
  };
  uint8_array *data = new uint8_array;
  uint8_t *tbl = data->tbl;
  for (int byte = 0; byte < 64; ++byte) {
    if (byte >= 0b1111'0) {
      tbl[byte] = 4;
    } else if (byte >= 0b1110'0) {
      tbl[byte] = 3;
    } else if (byte >= 0b1100'0) {
      tbl[byte] = 2;
    } else {
      tbl[byte] = 1;
    }
  }
  return tbl;
}
// SIZE: 257 * uint8_t
static const uint8_t UTF8_BYTE_LENGTH_TABLE[256] = {
    // start byte of 1-byte utf8 char: 0b0000'0000 ~ 0b0111'1111
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    // continuation byte: 0b1000'0000 ~ 0b1011'1111
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    // start byte of 2-byte utf8 char: 0b1100'0000 ~ 0b1101'1111
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    // start byte of 3-byte utf8 char: 0b1110'0000 ~ 0b1110'1111
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    // start byte of 4-byte utf8 char: 0b1111'0000 ~ 0b1111'0111
    // invalid utf8 byte: 0b1111'1000~ 0b1111'1111
    4, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1
};
static const uint8_t *UTF8_BYTE_LENGTH_TABLE2 = create_utf8_length_table2();

struct StringFunctions {

  static inline int utf8_length(std::string const &str) {
    int len = 0;
    for (int i = 0, char_size = 0; i < str.size(); i += char_size) {
      char_size = UTF8_BYTE_LENGTH_TABLE[static_cast<unsigned char>(str.data()[i])];
      ++len;
    }
    return len;
  }

  static inline int utf8_length2(std::string const &str) {
    int len = 0;
    for (int i = 0, char_size = 0; i < str.size(); i += char_size) {
      char_size = UTF8_BYTE_LENGTH_TABLE2[static_cast<unsigned char>(str.data()[i]) >> 3];
      ++len;
    }
    return len;
  }

  static inline int utf8_length3(std::string const &str) {
    int len = 0;
    for (int i = 0, char_size = 0; i < str.size(); i += char_size) {
      uint8_t c8 = ~static_cast<unsigned char>(str.data()[i]);
      int c = c8;
      int k = __builtin_clz(c);
      if (k == 28) {
        char_size = 4;
      } else if (k == 27) {
        char_size = 3;
      } else if (k == 26) {
        char_size = 2;
      } else {
        char_size = 1;
      }
      ++len;
    }
    return len;
  }

  static inline int utf8_length4(std::string const &str) {
    int len = 0;
    const char *data = str.data();
    uintptr_t p_start = reinterpret_cast<uintptr_t>(data);
    uintptr_t p_end = p_start + str.size();

    int skip_start = static_cast<int>(p_start & 7);
    p_start = p_start & ~7;

    int limit_end = static_cast<int>(p_end & 7);
    p_end = p_end & ~7;

    int char_size = 0;
    int k = skip_start;
    uintptr_t p = p_start;
    for (; p < p_end; p += 8) {
      register uint64_t oct asm("r12") = *reinterpret_cast<uint64_t *>(p);
      do {
        k &= 7;
        oct >>= k << 3;
        uint8_t c = static_cast<uint8_t>(oct ^ 0xff);
        int zeros = __builtin_clz(c);

        if (zeros == 28) {
          char_size = 4;
        } else if (zeros == 27) {
          char_size = 3;
        } else if (zeros == 26) {
          char_size = 2;
        } else {
          char_size = 1;
        }
        ++len;
        k = k + char_size;
      } while (k < 8);
    }

    if (limit_end == 0 || p_start == p_end) {
      return len;
    }

    uint64_t oct = *reinterpret_cast<uint64_t *>(p_end);
    do {
      oct >>= k << 3;
      uint8_t c = static_cast<uint8_t>(oct ^ 0xff);
      int zeros = __builtin_clz(c);

      if (zeros == 28) {
        char_size = 4;
      } else if (zeros == 27) {
        char_size = 3;
      } else if (zeros == 26) {
        char_size = 2;
      } else {
        char_size = 1;
      }
      ++len;
      k = k + char_size;
    } while (k < limit_end);
    return len;
  }

  static inline int utf8_length_simd_avx2(std::string const &str) {
    int len = 0;
    const char *p = str.data();
    const char *end = p + str.size();
#if defined(__AVX2__)
    constexpr auto bytes_avx2 = sizeof(__m256i);
    const auto src_end_avx2 = p + (str.size() & ~(bytes_avx2 - 1));
    const auto threshold = _mm256_set1_epi8(0b1011'1111);
    for (; p < src_end_avx2; p += bytes_avx2)
      len += __builtin_popcount(
          _mm256_movemask_epi8(
              _mm256_cmpgt_epi8(
                  _mm256_loadu_si256(reinterpret_cast<const __m256i *>(p)), threshold)));
#elif defined(__SSE2__)
    constexpr auto bytes_sse2 = sizeof(__m128i);
    const auto src_end_sse2 = p + (str.size() & ~(bytes_sse2 - 1));
    const auto threshold = _mm_set1_epi8(0b1011'1111);
    for (; p < src_end_sse2; p += bytes_sse2) {
      len += __builtin_popcount(
          _mm_movemask_epi8(
              _mm_cmpgt_epi8(
                  _mm_loadu_si128(reinterpret_cast<const __m128i *>(p)), threshold)));
    }
#endif
    for (; p < end; ++p) {
      len += static_cast<int8_t>(*p) > static_cast<int8_t>(0xbf);
    }
    return len;
  }

  static inline int utf8_length_simd_sse2(std::string const &str) {
    int len = 0;
    const char *p = str.data();
    const char *end = p + str.size();
#if defined(__SSE2__)
    constexpr auto bytes_sse2 = sizeof(__m128i);
    const auto src_end_sse2 = p + (str.size() & ~(bytes_sse2 - 1));
    const auto threshold = _mm_set1_epi8(0b1011'1111);
    for (; p < src_end_sse2; p += bytes_sse2) {
      len += __builtin_popcount(
          _mm_movemask_epi8(
              _mm_cmpgt_epi8(
                  _mm_loadu_si128(reinterpret_cast<const __m128i *>(p)), threshold)));
    }
#endif
    for (; p < end; ++p) {
      len += static_cast<int8_t>(*p) > static_cast<int8_t>(0xbf);
    }
    return len;
  }

  static constexpr uint8_t UTF8_FIRST_CHAR[7] = {
      0b0000'0000,
      0b1000'0000,
      0b1100'0000,
      0b1110'0000,
      0b1111'0000,
      0b1111'1000,
      0b1111'1100,
  };

  static inline bool validate_ascii_fast(const char *src, size_t len) {
#ifdef __AVX2__
    size_t i = 0;
    __m256i has_error = _mm256_setzero_si256();
    if (len >= 32) {
      for (; i <= len - 32; i += 32) {
        __m256i current_bytes = _mm256_loadu_si256((const __m256i *) (src + i));
        has_error = _mm256_or_si256(has_error, current_bytes);
      }
    }
    int error_mask = _mm256_movemask_epi8(has_error);

    char tail_has_error = 0;
    for (; i < len; i++) {
      tail_has_error |= src[i];
    }
    error_mask |= (tail_has_error & 0x80);

    return !error_mask;
#elif defined(__SSE2__)
    size_t i = 0;
    __m128i has_error = _mm_setzero_si128();
    if (len >= 16) {
      for (; i <= len - 16; i += 16) {
        __m128i current_bytes = _mm_loadu_si128((const __m128i *) (src + i));
        has_error = _mm_or_si128(has_error, current_bytes);
      }
    }
    int error_mask = _mm_movemask_epi8(has_error);

    char tail_has_error = 0;
    for (; i < len; i++) {
      tail_has_error |= src[i];
    }
    error_mask |= (tail_has_error & 0x80);

    return !error_mask;
#else
    char tail_has_error = 0;
    for (size_t i = 0; i < len; i++) {
        tail_has_error |= src[i];
    }
    return !(tail_has_error & 0x80);
#endif
  }

  template<bool negative_offset>
  static inline void ascii_substr(BinaryColumn const &src, BinaryColumn &dst, int offset, int len) {
    const auto n = src.size();
    for (auto i = 0; i < n; ++i) {
      Slice s = src.get_slice(i);
      if (s.size == 0) {
        dst.append("");
        continue;
      }
      auto from_pos = offset;
      auto begin = s.data;
      if constexpr (negative_offset) {
        from_pos = from_pos + s.size;
        if (from_pos < 0) {
          dst.append("");
          continue;
        }
      } else {
        if (from_pos > s.size) {
          dst.append("");
          continue;
        }
      }
      auto to_pos = from_pos + len;
      if (to_pos < from_pos || to_pos > s.size) {
        to_pos = s.size;
      }
      dst.append(begin + from_pos, begin + to_pos);
    }
  }

  template<bool negative_offset>
  static inline void ascii_substr_by_ref(
      BinaryColumn const &src,
      std::string &bytes,
      std::vector<int> &offsets,
      int off, int len) {
    const auto n = src.size();
    for (auto i = 0; i < n; ++i) {
      Slice s = src.get_slice(i);
      if (s.size == 0) {
        offsets[i + 1] = bytes.size();
        continue;
      }
      auto from_pos = off;
      auto begin = s.data;
      if constexpr (negative_offset) {
        from_pos = from_pos + s.size;
        if (from_pos < 0) {
          offsets[i + 1] = bytes.size();
          continue;
        }
      } else {
        if (from_pos > s.size) {
          offsets[i + 1] = bytes.size();
          continue;
        }
      }
      auto to_pos = from_pos + len;
      if (to_pos < from_pos || to_pos > s.size) {
        to_pos = s.size;
      }
      bytes.insert(bytes.end(), begin + from_pos, begin + to_pos);
      offsets[i + 1] = bytes.size();
    }
  }

  template<bool negative_offset>
  static inline void ascii_substr_by_ptr(
      BinaryColumn *src,
      std::string *bytes,
      std::vector<int> *offsets,
      int off, int len) {
    const auto n = src->size();
    for (auto i = 0; i < n; ++i) {
      Slice s = src->get_slice(i);
      if (s.size == 0) {
        (*offsets)[i + 1] = bytes->size();
        continue;
      }
      auto from_pos = off;
      auto begin = s.data;
      if constexpr (negative_offset) {
        from_pos = from_pos + s.size;
        if (from_pos < 0) {
          (*offsets)[i + 1] = bytes->size();
          continue;
        }
      } else {
        if (from_pos > s.size) {
          (*offsets)[i + 1] = bytes->size();
          continue;
        }
      }
      auto to_pos = from_pos + len;
      if (to_pos < from_pos || to_pos > s.size) {
        to_pos = s.size;
      }
      bytes->insert(bytes->end(), begin + from_pos, begin + to_pos);
      (*offsets)[i + 1] = bytes->size();
    }
  }

  template<bool lookup_table>
  static inline const char *skip_leading_utf8(const char *p, const char *end, size_t n) {
    int char_size = 0;
    for (auto i = 0; i < n && p < end; ++i, p += char_size) {
      if constexpr (lookup_table) {
        char_size = UTF8_BYTE_LENGTH_TABLE[static_cast<uint8_t>(*p)];
      } else {
        uint8_t b = ~static_cast<uint8_t>(*p);
        if (b >> 7 == 1) {
          char_size = 1;
        } else {
          char_size = __builtin_clz(b) - 24;
        }
      }
    }
    return p;
  }

  template<bool lookup_table>
  static inline const char *skip_trailing_utf8(const char *p, const char *begin, size_t n) {
    constexpr auto threshold = static_cast<int8_t>(0b1011'1111);
    for (auto i = 0; i < n && p >= begin; ++i) {
      --p;
      while (p >= begin && static_cast<int8_t>(*p) <= threshold)--p;
    }
    return p;
  }

  static inline std::string upper_old(std::string const &s) {
    std::string v = s;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) { return std::toupper(c); });
    return v;
  }
  static inline std::string lower_old(std::string const &s) {
    std::string v = s;
    std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c) { return std::tolower(c); });
    return v;
  }

  static inline std::string lower_old2(std::string const &s) {
    std::string v = s;
    for (auto p = v.begin(); p < v.end(); p += 1) {
      if ('A' <= *p && *p <= 'Z')
        *p ^= 32;
    }
    return v;
  }

  static inline std::string upper_old2(std::string const &s) {
    std::string v = s;
    for (auto p = v.begin(); p < v.end(); p += 1) {
      if ('a' <= *p && *p <= 'z')
        *p ^= 32;
    }
    return v;
  }

  static inline std::string upper_new(std::string const &s) {
    std::string result = s;
    char *begin = result.data();
    char *end = result.data() + s.size();
    const size_t size = result.size();
#if defined(__SSE2__)
    static constexpr int SSE2_BYTES = sizeof(__m128i);
    const char *sse2_end = begin + (size & ~(SSE2_BYTES - 1));
    char *p = begin;
    const auto a_minus1 = _mm_set1_epi8('a' - 1);
    const auto z_plus1 = _mm_set1_epi8('z' + 1);
    const auto delta = _mm_set1_epi8('a' - 'A');
    for (; p > sse2_end; p += SSE2_BYTES) {
      auto bytes = _mm_loadu_si128((const __m128i *) p);
      _mm_maskmoveu_si128(
          _mm_sub_epi8(bytes, delta),
          _mm_and_si128(
              _mm_cmpgt_epi8(bytes, a_minus1),
              _mm_cmpgt_epi8(z_plus1, bytes)),
          p);
    }
#endif
    for (; p < end; p += 1) {
      if ('a' <= (*p) && (*p) <= 'z')
        (*p) ^= 32;
    }
    return result;
  }

  static inline void upper_vector_old(BinaryColumn const &src, BinaryColumn &dst) {
    const auto n = src.size();
    for (auto i = 0; i < n; ++i) {
      auto ret = upper_old(src.get_slice(i).to_string());
      dst.bytes.reserve(ret.size());
      dst.append(ret);
    }
  }

  static inline void lower_vector_old(BinaryColumn const &src, BinaryColumn &dst) {
    const auto n = src.size();
    for (auto i = 0; i < n; ++i) {
      auto ret = lower_old(src.get_slice(i).to_string());
      dst.bytes.reserve(ret.size());
      dst.append(ret);
    }
  }

  static inline void upper_vector_copy_3_times(BinaryColumn const &src, BinaryColumn &dst) {
    const auto n = src.size();
    for (auto i = 0; i < n; ++i) {
      std::string result = src.get_slice(i).to_string();
      char *begin = result.data();
      char *end = result.data() + result.size();
      const size_t size = result.size();
#if defined(__SSE2__)
      static constexpr int SSE2_BYTES = sizeof(__m128i);
      const char *sse2_end = begin + (size & ~(SSE2_BYTES - 1));
      char *p = begin;
      const auto a_minus1 = _mm_set1_epi8('a' - 1);
      const auto z_plus1 = _mm_set1_epi8('z' + 1);
      const auto delta = _mm_set1_epi8('a' - 'A');
      for (; p > sse2_end; p += SSE2_BYTES) {
        auto bytes = _mm_loadu_si128((const __m128i *) p);
        _mm_maskmoveu_si128(
            _mm_xor_si128(bytes, delta),
            _mm_and_si128(
                _mm_cmpgt_epi8(bytes, a_minus1),
                _mm_cmpgt_epi8(z_plus1, bytes)),
            p);
      }
#endif
      std::transform(p, end, p, [](unsigned char c) { return std::toupper(c); });
      dst.append(result);
    }
  }

  static inline void upper_vector_copy_1_times(BinaryColumn const &src, BinaryColumn &dst) {
    const auto n = src.size();
    char *q = (char *) dst.bytes.data();
    for (auto i = 0; i < n; ++i) {
      auto slice = src.get_slice(i);
      char *begin = const_cast<char *>(slice.begin());
      char *end = const_cast<char *>(slice.end());
      const size_t size = slice.size;
      dst.offsets.push_back(size);

#if defined(__SSE2__)
      static constexpr int SSE2_BYTES = sizeof(__m128i);
      const char *sse2_end = begin + (size & ~(SSE2_BYTES - 1));
      char *p = begin;
      const auto a_minus1 = _mm_set1_epi8('a' - 1);
      const auto z_plus1 = _mm_set1_epi8('z' + 1);
      const auto delta = _mm_set1_epi8(32);
      const auto ones = _mm_set1_epi8(0xff);

      for (; p > sse2_end; p += SSE2_BYTES, q += SSE2_BYTES) {
        auto bytes = _mm_loadu_si128((const __m128i *) p);
        auto masks = _mm_and_si128(
            _mm_cmpgt_epi8(bytes, a_minus1),
            _mm_cmpgt_epi8(z_plus1, bytes));

        _mm_maskmoveu_si128(
            _mm_xor_si128(bytes, delta),
            masks,
            q);
        _mm_maskmoveu_si128(
            bytes,
            _mm_xor_si128(masks, ones),
            q);
      }
#endif
      std::transform(p, end, q, [](unsigned char c) { return std::toupper(c); });
    }
  }

  static inline void upper_vector_copy_2_times(BinaryColumn const &src, BinaryColumn &dst) {
    const auto n = src.size();
    for (auto i = 0; i < n; ++i) {
      auto slice = src.get_slice(i);
      dst.append(slice.begin(), slice.end());
      auto slice2 = dst.get_last_slice();
      char *begin = const_cast<char *>(slice2.begin());
      char *end = const_cast<char *>(slice2.end());
      const size_t size = slice.size;

#if defined(__SSE2__)
      static constexpr int SSE2_BYTES = sizeof(__m128i);
      const char *sse2_end = begin + (size & ~(SSE2_BYTES - 1));
      char *p = begin;
      const auto a_minus1 = _mm_set1_epi8('a' - 1);
      const auto z_plus1 = _mm_set1_epi8('z' + 1);
      const auto delta = _mm_set1_epi8(32);

      for (; p > sse2_end; p += SSE2_BYTES) {
        auto bytes = _mm_loadu_si128((const __m128i *) p);
        auto masks = _mm_and_si128(
            _mm_cmpgt_epi8(bytes, a_minus1),
            _mm_cmpgt_epi8(z_plus1, bytes));

        _mm_maskmoveu_si128(
            _mm_xor_si128(bytes, delta),
            masks,
            p);
      }
#endif
      std::transform(p, end, p, [](char c) -> char {
        if ('a' <= c && c <= 'z')
          return c ^ static_cast<char>(32);
        else
          return c;
      });
    }
  }

  template<char C0, char C1>
  static inline void case_vector_new1(BinaryColumn const &src, BinaryColumn &dst) {
    const auto n = src.size();
    dst.offsets = src.offsets;
    dst.bytes = src.bytes;
    auto begin = dst.bytes.data();
    const size_t size = dst.bytes.size();
    auto end = dst.bytes.data() + size;

#if defined(__SSE2__)
    static constexpr int SSE2_BYTES = sizeof(__m128i);
    auto sse2_end = begin + (size & ~(SSE2_BYTES - 1));
    auto p = begin;
    const auto a_minus1 = _mm_set1_epi8(C0 - 1);
    const auto z_plus1 = _mm_set1_epi8(C1 + 1);
    const auto delta = _mm_set1_epi8(32);

    for (; p > sse2_end; p += SSE2_BYTES) {
      auto bytes = _mm_loadu_si128((const __m128i *) p);
      auto masks = _mm_and_si128(
          _mm_cmpgt_epi8(bytes, a_minus1),
          _mm_cmpgt_epi8(z_plus1, bytes));

      _mm_maskmoveu_si128(
          _mm_xor_si128(bytes, delta),
          masks,
          (char *) p);
    }
#endif
    for (; p < end; p += 1) {
      if (C0 <= *p && *p <= C1)
        *p ^= 32;
    }
  }

  template<bool use_raw, char C0, char C1>
  static inline void case_vector_new2(BinaryColumn const &src, BinaryColumn &dst) {
    const auto n = src.size();
    dst.offsets = src.offsets;
    raw::raw_vector<uint8_t> buffer;
    const size_t size = src.bytes.size();

    uint8_t *q = nullptr;
    if constexpr (use_raw) {
      buffer.resize(size);
      q = buffer.data();
    } else {
      dst.bytes.resize(size);
      q = dst.bytes.data();
    }

    char *begin = (char *) (src.bytes.data());
    char *end = (char *) (src.bytes.data() + size);

#if defined(__SSE2__)
    static constexpr int SSE2_BYTES = sizeof(__m128i);
    const char *sse2_end = begin + (size & ~(SSE2_BYTES - 1));
    char *p = begin;
    const auto a_minus1 = _mm_set1_epi8(C0 - 1);
    const auto z_plus1 = _mm_set1_epi8(C1 + 1);
    const auto delta = _mm_set1_epi8(32);

    for (; p > sse2_end; p += SSE2_BYTES, q += SSE2_BYTES) {
      auto bytes = _mm_loadu_si128((const __m128i *) p);
      auto masks = _mm_and_si128(
          _mm_cmpgt_epi8(bytes, a_minus1),
          _mm_cmpgt_epi8(z_plus1, bytes));

      _mm_storeu_si128(
          (__m128i *) q,
          _mm_xor_si128(bytes,
                        _mm_and_si128(masks, delta)));
    }
#endif
    for (; p < end; p += 1, q += 1) {
      if (C0 <= *p && *p <= C1)
        *q = *p ^ 32;
      else
        *q = *p;
    }

    if constexpr(use_raw) {
      dst.bytes = std::move(reinterpret_cast<std::vector<uint8_t> &>(buffer));
    }
  }

  static inline void lower_vector_new1(BinaryColumn const &src, BinaryColumn &dst) {
    case_vector_new1<'A', 'Z'>(src, dst);
  }

  static inline void upper_vector_new1(BinaryColumn const &src, BinaryColumn &dst) {
    case_vector_new1<'a', 'z'>(src, dst);
  }

  template<bool use_raw = true>
  static inline void lower_vector_new2(BinaryColumn const &src, BinaryColumn &dst) {
    case_vector_new2<use_raw, 'A', 'Z'>(src, dst);
  }

  template<bool use_raw = true>
  static inline void upper_vector_new2(BinaryColumn const &src, BinaryColumn &dst) {
    case_vector_new2<use_raw, 'a', 'z'>(src, dst);
  }

  static inline std::string lower_dummy(std::string const &s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return c; });
    return result;
  }

  static inline std::string lower_dummy2(std::string const &s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return c >> 1; });
    return result;
  }
  static inline std::string lower_dummy3(std::string const &s) {
    std::string result = s;
    return result;
  }

  static inline std::string lower_new(std::string const &s) {
    std::string result = s;
    char *begin = result.data();
    char *end = result.data() + s.size();
    const size_t size = result.size();
#if defined(__SSE2__)
    static constexpr int SSE2_BYTES = sizeof(__m128i);
    const char *sse2_end = begin + (size & ~(SSE2_BYTES - 1));
    char *p = begin;
    const auto a_minus1 = _mm_set1_epi8('A' - 1);
    const auto z_plus1 = _mm_set1_epi8('Z' + 1);
    const auto delta = _mm_set1_epi8('a' - 'A');
    for (; p > sse2_end; p += SSE2_BYTES) {
      auto bytes = _mm_loadu_si128((const __m128i *) p);
      _mm_maskmoveu_si128(
          _mm_xor_si128(bytes, delta),
          _mm_and_si128(
              _mm_cmpgt_epi8(bytes, a_minus1),
              _mm_cmpgt_epi8(z_plus1, bytes)),
          p);
    }
#endif
    for (; p < end; p += 1) {
      if ('A' <= (*p) && (*p) <= 'Z')
        (*p) ^= 32;
    }
    return result;
  }

  template<bool use_length, bool lookup_table>
  static inline void utf8_substr_from_left(BinaryColumn const &src,
                                           BinaryColumn &dst,
                                           int offset,
                                           [[maybe_unused]]int len) {
    auto const size = src.size();
    for (auto i = 0; i < size; ++i) {
      auto s = src.get_slice(i);
      auto begin = s.begin();
      auto end = s.end();
      auto from_ptr = skip_leading_utf8<lookup_table>(begin, end, offset);
      //std::cout<<s.to_string()<<", offset="<<offset<<", from_pos="<<from_ptr-begin<<std::endl;
      if constexpr (use_length) {
        if (from_ptr >= end) {
          dst.append("");
        } else {
          auto to_ptr = skip_leading_utf8<lookup_table>(from_ptr, end, len);
          //std::cout<<s.to_string()<<", offset="<<offset<<", end_pos="<<to_ptr-begin<<std::endl;
          dst.append(from_ptr, to_ptr);
        }
      } else {
        dst.append(from_ptr, end);
      }
    }
  }

  static inline size_t get_utf8_small_index(const Slice &str, uint8_t *small_index) {
    size_t n = 0;
    for (uint8_t i = 0, char_size = 0; i < str.size; i += char_size) {
      char_size = UTF8_BYTE_LENGTH_TABLE[static_cast<unsigned char>(str.data[i])];
      small_index[n++] = i;
    }
    return n;
  }

  template<bool use_length, bool lookup_table>
  static inline void utf8_substr_from_right(BinaryColumn const &src,
                                            BinaryColumn &dst,
                                            int offset,
                                            [[maybe_unused]]int len) {
    const auto size = src.size();
    constexpr size_t SMALL_INDEX_MAX = 32;
    uint8_t small_index[SMALL_INDEX_MAX] = {0};
    for (auto i = 0; i < size; ++i) {
      auto s = src.get_slice(i);
      auto begin = s.begin();
      auto end = s.end();

      if (offset > s.size) {
        dst.append("");
        continue;
      }

      if (s.size <= SMALL_INDEX_MAX) {
        auto small_index_size = get_utf8_small_index(s, small_index);
        if (offset > small_index_size) {
          dst.append("");
          continue;
        }
        auto from_idx = small_index_size - offset;
        const char *from_ptr = begin + small_index[from_idx];
        const char *to_ptr = end;
        // take the first `len` bytes from the trailing `off` bytes, so if
        // len >= off, at most `off` bytes can be taken.
        if constexpr (use_length) {
          auto to_idx = from_idx + len;
          if (len < offset) {
            to_ptr = begin + small_index[to_idx];
          }
        }
        dst.append(from_ptr, to_ptr);
      } else {

        auto from_ptr = skip_trailing_utf8<lookup_table>(end, begin, offset);

        if (from_ptr < begin) {
          dst.append("");
          continue;
        }

        if constexpr(use_length) {
          //std::cout << "end-from_ptr=" << end - from_ptr << ", len=" << len << std::endl;
          if (len > end - from_ptr) {
            dst.append(from_ptr, end);
          } else {
            auto to_ptr = skip_leading_utf8<lookup_table>(from_ptr, end, len);
            //std::cout << "to_ptr=" << to_ptr - begin << std::endl;
            dst.append(from_ptr, to_ptr);
          }
        } else {
          dst.append(from_ptr, end);
        }
      }
    }
  }

  template<bool check_ascii, bool use_length, bool lookup_table = false>
  static inline void substr(BinaryColumn const &src, BinaryColumn &dst, int offset, [[maybe_unused]] int len) {
    if (offset == 0) {
      return;
    }
    if constexpr(check_ascii) {
      auto is_ascii = validate_ascii_fast((const char *) src.bytes.data(), src.bytes.size());
      //std::cout << std::boolalpha << "is_ascii=" << is_ascii << std::endl;
      if (is_ascii) {
        if (offset > 0) {
          ascii_substr<false>(src, dst, offset - 1, len);
        } else {
          ascii_substr<true>(src, dst, offset, len);
        }
      } else {
        //std::cout << "enter substr<false, use_length>" << std::endl;
        substr<false, use_length, lookup_table>(src, dst, offset, len);
      }
    } else {
      if (offset > 0) {
        //std::cout << "enter utf8_substr_from_left" << std::endl;
        utf8_substr_from_left<use_length, lookup_table>(src, dst, offset - 1, len);
      } else if (offset < 0) {
        utf8_substr_from_right<use_length, lookup_table>(src, dst, -offset, len);
      }
    }
  }
  static inline size_t get_utf8_index(const Slice &str, std::vector<size_t> *index) {
    for (int i = 0, char_size = 0; i < str.size; i += char_size) {
      char_size = UTF8_BYTE_LENGTH_TABLE[static_cast<unsigned char>(str.data[i])];
      index->push_back(i);
    }
    return index->size();
  }

  static inline size_t get_utf8_index2(const Slice &str, std::vector<size_t> *index) {
    for (int i = 0, char_size = 0; i < str.size; i += char_size) {
      uint8_t b = ~static_cast<uint8_t>(str.data[i]);
      if (b >> 7 == 1) {
        char_size = 1;
      } else {
        char_size = __builtin_clz(b) - 24;
      }
      index->push_back(i);
    }
    return index->size();
  }

  template<bool negative_offset = false, bool use_length = true>
  static inline void substr_new(BinaryColumn const &src, BinaryColumn &dst, int offset, [[maybe_unused]] int len) {
    std::vector<size_t> index;
    auto is_ascii = validate_ascii_fast((const char *) src.bytes.data(), src.bytes.size());
    if (is_ascii) {
      ascii_substr<true>(src, dst, offset, len);
    } else {
      if (offset > 0) {
        offset -= 1;
      }
      auto size = src.size();
      for (size_t i = 0; i < size; ++i) {
        Slice value = src.get_slice(i);
        if (__builtin_expect((value.size == 0), 0)) {
          dst.append("");
          continue;
        }
        index.clear();
        get_utf8_index2(value, &index);
        auto pos = offset;
        if constexpr(negative_offset) {
          if (pos < 0) {
            pos += index.size();
          }
        }

        if (pos >= index.size() || pos < 0) {
          dst.append("");
          continue;
        }

        int byte_pos = index[pos];
        int result_length = value.size - byte_pos;
        if (pos + len < index.size()) {
          result_length = index[pos + len] - byte_pos;
        }
        dst.append(value.begin() + byte_pos, value.begin() + byte_pos + result_length);
      }
    }
  }
  template<bool negative_offset = false, bool use_length = true>
  static inline void substr_old(BinaryColumn const &src, BinaryColumn &dst, int offset, [[maybe_unused]] int len) {
    std::vector<size_t> index;
    auto is_ascii = validate_ascii_fast((char *) src.bytes.data(), src.bytes.size());
    if (is_ascii) {
      const size_t size = src.size();
      for (int i = 0; i < size; ++i) {
        Slice value = src.get_slice(i);
        if (__builtin_expect((value.size == 0 || offset > value.size), 0)) {
          dst.append("");
          continue;
        }

        int byte_pos = offset - 1;
        int result_length = len;
        if (offset + len > value.size) {
          result_length = value.size - byte_pos;
        }

        dst.append(value.data + byte_pos, value.data + byte_pos + result_length);
      }
    } else {
      if (offset > 0) {
        offset -= 1;
      }
      auto size = src.size();
      for (size_t i = 0; i < size; ++i) {
        Slice value = src.get_slice(i);
        if (__builtin_expect((value.size == 0), 0)) {
          dst.append("");
          continue;
        }
        index.clear();
        get_utf8_index(value, &index);
        auto pos = offset;
        if constexpr(negative_offset) {
          if (pos < 0) {
            pos += index.size();
          }
        }

        if (pos >= index.size() || pos < 0) {
          dst.append("");
          continue;
        }

        int byte_pos = index[pos];
        int result_length = value.size - byte_pos;
        if (pos + len < index.size()) {
          result_length = index[pos + len] - byte_pos;
        }
        dst.append(value.begin() + byte_pos, value.begin() + byte_pos + result_length);
      }
    }
  }

  static std::string gen_utf8(std::vector<int> const &weights, int n1, int n2) {
    assert(weights.size() == 6);
    assert(n1 >= 0 && n2 >= 0 && n1 <= n2);

    assert(std::all_of(weights.begin(), weights.end(), [](auto i) {
      return i >= 0;
    }));

    assert(std::any_of(weights.begin(), weights.end(), [](auto i) {
      return i > 0;
    }));

    if (n2 == 0) {
      return std::string();
    }
    std::vector<int> pdf(weights.size() + 1, 0);
    pdf[0] = 0;
    for (auto i = 1; i < pdf.size(); ++i) {
      pdf[i] = pdf[i - 1] + weights[i - 1];
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> rand_n(n1, n2);
    std::uniform_int_distribution<int> rand_pdf(1, pdf.back());
    std::uniform_int_distribution<uint8_t> rand_char(0, 255);
    int n = rand_n(gen);
    std::string text(n, '\0');
    auto p = text.begin();
    while (p < text.end()) {
      auto sample = rand_pdf(gen);
      int char_bytes = 0;
      for (size_t i = 1; i < pdf.size(); ++i) {
        if (pdf[i - 1] < sample && sample <= pdf[i]) {
          char_bytes = i;
          break;
        }
      }

      assert(0 <= char_bytes && char_bytes < pdf.size());
      char_bytes = std::min(char_bytes, static_cast<int>(text.end() - p));
      if (char_bytes == 1) {
        *p = rand_char(gen) & 0b0111'1111;
        ++p;
      } else {
        *p = (rand_char(gen) >> (char_bytes + 1)) | UTF8_FIRST_CHAR[char_bytes];
        ++p;
        while (--char_bytes > 0) {
          *p = (rand_char(gen) >> 2) | 0b1000'0000;
          ++p;
        }
      }
    }
    return text;
  }

  static inline std::vector<std::string> gen_utf8_vector(std::vector<int> const &weights, int size, int n1, int n2) {
    assert(size > 0);
    std::vector<std::string> batch;
    batch.resize(size);
    for (auto i = 0; i < size; ++i) {
      batch[i] = gen_utf8(weights, n1, n2);
    }
    return batch;
  }

  static inline int utf8_char_bytes(uint8_t c) {
    return UTF8_BYTE_LENGTH_TABLE[c];
  }
  static inline void stat_utf8(std::vector<std::string> batch) {
    std::vector<int> stat(7, 0);
    for (auto &text: batch) {
      if (text.empty()) {
        continue;
      }
      auto p = text.begin();
      while (p < text.end()) {
        int len = utf8_char_bytes(*p);
        ++stat[len];
        p += len;
      }
    }
    for (int i = 1; i < stat.size(); ++i) {
      std::cout << "utf8-" << i << "byte: " << stat[i] << std::endl;
    }
  }

};

int env_to_int(std::string const &name, std::string const &default_value) {
  const char *val = getenv(name.c_str());
  if (val == nullptr) {
    val = default_value.c_str();
  }
  auto n = static_cast<int>(strtoul(val, nullptr, 10));
  std::cerr << "ENV " << name << "=" << n << std::endl;
  return n;
}

std::vector<int> env_to_csv_int(std::string const &name, std::string const &default_value) {
  const char *val = getenv(name.c_str());
  if (val == nullptr) {
    val = default_value.c_str();
  }
  std::string s(val);
  std::string::size_type start = 0;
  std::vector<int> csv;
  while (true) {
    auto end = s.find(',', start);
    if (end == std::string::npos) {
      auto subs = s.substr(start, s.size() - start);
      auto n = static_cast<int>(strtoul(subs.c_str(), nullptr, 10));
      csv.push_back(n);
      break;
    } else {
      auto subs = s.substr(start, end - start);
      auto n = static_cast<int>(strtoul(subs.c_str(), nullptr, 10));
      csv.push_back(n);
      start = end + 1;
    }
  }
  std::cerr << "ENV " << name << "=";
  if (csv.empty()) {
    return csv;
  }
  auto it = csv.begin();
  std::cerr << *it;
  while (++it != csv.end()) {
    std::cerr << "," << *it;
  }
  std::cerr << std::endl;
  return csv;
}

struct prepare_utf8_data {
  int vector_size = 8192;
  std::vector<int> weights{10, 10, 4, 3, 0, 0};
  int min_length = 100;
  int max_length = 1000;
  std::vector<std::string> data;
  BinaryColumn binary_column;
  std::vector<int> result;
  prepare_utf8_data(
      int vector_size,
      std::vector<int> weights,
      int min_length,
      int max_length) :
      vector_size(vector_size),
      weights(weights), min_length(min_length), max_length(max_length) {

    data = std::move(StringFunctions::gen_utf8_vector(weights, vector_size, min_length, max_length));
    for (auto &s: data) {
      binary_column.append(s);
    }

    std::cerr << "generate " << data.size() << " strings: " << std::endl;
    StringFunctions::stat_utf8(data);
    result.resize(data.size());
  }

  prepare_utf8_data() {
    vector_size = env_to_int("VECTOR_SIZE", "8192");
    weights = std::move(env_to_csv_int("WEIGHTS", "10,10,4,3,0,0"));
    min_length = env_to_int("MIN_LENGTH", "100");
    max_length = env_to_int("MAX_LENGTH", "1000");
    data = std::move(StringFunctions::gen_utf8_vector(weights, vector_size, min_length, max_length));

    for (auto &s: data) {
      binary_column.append(s);
    }

    std::cerr << "generate " << data.size() << " strings: " << std::endl;
    StringFunctions::stat_utf8(data);
    result.resize(data.size());
  }
};

template<typename T, typename U, typename F, typename... Args>
void apply_vector(std::vector<T> &data, std::vector<U> &result, F f, Args &&... args) {
  for (auto i = 0; i < data.size(); ++i) {
    auto &e = data[i];
    result[i] = f(e, std::forward<Args>(args)...);
  }
}

#endif //CPP_ETUDES_INCLUDE_STRING_FUNCTIONS_HH_
