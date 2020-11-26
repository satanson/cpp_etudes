//
// Created by grakra on 2020/11/26.
//

#ifndef CPP_ETUDES_INCLUDE_MEMEQUAL_HH_
#define CPP_ETUDES_INCLUDE_MEMEQUAL_HH_
#include <immintrin.h>
#include <cstdint>
#include <cstring>

inline bool mem_equal_optimized(const char* p1, size_t size1, const char* p2, size_t size2) {
  if (size1 != size2) {
    return false;
  }

  if (size1 == 0) {
    return true;
  }

  // const char * p1_end = p1 + size1;
  const char * p1_end_16 = p1 + size1 / 16 * 16;

  __m128i zero16 = _mm_setzero_si128();

  while (p1 < p1_end_16)
  {
    if (!_mm_testc_si128(
        zero16,
        _mm_xor_si128(
            _mm_loadu_si128(reinterpret_cast<const __m128i *>(p1)),
            _mm_loadu_si128(reinterpret_cast<const __m128i *>(p2)))))
      return false;

    p1 += 16;
    p2 += 16;
  }

  switch (size1 % 16)
  {
    case 15 : if (p1[14] != p2[14]) return false; [[fallthrough]];
    case 14 : if (p1[13] != p2[13]) return false; [[fallthrough]];
    case 13 : if (p1[12] != p2[12]) return false; [[fallthrough]];
    case 12 : if (reinterpret_cast<const uint32_t*>(p1)[2] == reinterpret_cast<const uint32_t*>(p2)[2]) goto l8;
      else return false;
    case 11 : if (p1[10] != p2[10]) return false; [[fallthrough]];
    case 10 : if (p1[9] != p2[9]) return false; [[fallthrough]];
    case  9 : if (p1[8] != p2[8]) return false;
    l8 : [[fallthrough]];
    case  8 : return reinterpret_cast<const uint64_t*>(p1)[0] == reinterpret_cast<const uint64_t*>(p2)[0];
    case  7 : if (p1[6] != p2[6]) return false; [[fallthrough]];
    case  6 : if (p1[5] != p2[5]) return false; [[fallthrough]];
    case  5 : if (p1[4] != p2[4]) return false; [[fallthrough]];
    case  4 : return reinterpret_cast<const uint32_t*>(p1)[0] == reinterpret_cast<const uint32_t*>(p2)[0];
    case  3 : if (p1[2] != p2[2]) return false; [[fallthrough]];
    case  2 : return reinterpret_cast<const uint16_t*>(p1)[0] == reinterpret_cast<const uint16_t*>(p2)[0];
    case  1 : if (p1[0] != p2[0]) return false; [[fallthrough]];
    case  0 : break;
  }

  return true;
}


inline bool mem_equal_memcpy(const char* p1, size_t size1, const char* p2, size_t size2) {
    return (size1 == size2) && (memcmp(p1, p2, size1) == 0);
}


#endif //CPP_ETUDES_INCLUDE_MEMEQUAL_HH_
