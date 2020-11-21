// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com

//
// Created by grakra on 2020/7/2.
//

#ifndef CPP_ETUDES_BITS_OP_HH
#define CPP_ETUDES_BITS_OP_HH
namespace com {
namespace grakra {
namespace util {
static inline uint32_t reverse_bits(uint32_t n) {
  uint32_t j = 0x80000000;
  uint32_t m = 0;
  for (uint32_t i = 1; i != 0; i <<= 1) {
    if (n & i) {
      m |= j;
    }
    j >>= 1;
  }
  return m;
}

} // namespace util
} // namespace grakra
} // namespace com
#endif //CPP_ETUDES_BITS_OP_HH
