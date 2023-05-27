#pragma once
#include <immintrin.h>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <random>
#include <iostream>

struct DataInitializer {
    explicit DataInitializer(std::vector<int8_t>& data){
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int8_t> r(0, 64);
        for (auto i=0; i<data.size(); ++i){
            data[i] = r(gen);
        }
    }
};

std::pair<float, int> calc_harmonic_mean1(int8_t* data, size_t n) {
    float harmonic_mean = 0;
    int num_zeros = 0;

    for (int i = 0; i < n; ++i) {
        harmonic_mean += powf(2.0f, -data[i]);

        if (data[i] == 0) {
            ++num_zeros;
        }
    }
    harmonic_mean = 1.0f / harmonic_mean;
    return std::make_pair(harmonic_mean, num_zeros);
}
#if defined(__AVX2__)
__m256 exp256_ps(__m256 x) {
    /* Modified code. The original code is here: https://github.com/reyoung/avx_mathfun

   AVX implementation of exp
   Based on "sse_mathfun.h", by Julien Pommier
   http://gruntthepeon.free.fr/ssemath/
   Copyright (C) 2012 Giovanni Garberoglio
   Interdisciplinary Laboratory for Computational Science (LISC)
   Fondazione Bruno Kessler and University of Trento
   via Sommarive, 18
   I-38123 Trento (Italy)
  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.
  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:
  1. The origin of this software must not be misrepresented; you must not
           claim that you wrote the original software. If you use this software
                   in a product, an acknowledgment in the product documentation would be
                   appreciated but is not required.
           2. Altered source versions must be plainly marked as such, and must not be
                           misrepresented as being the original software.
                   3. This notice may not be removed or altered from any source distribution.
                           (this is the zlib license)
                                   */
    /*
  To increase the compatibility across different compilers the original code is
  converted to plain AVX2 intrinsics code without ingenious macro's,
  gcc style alignment attributes etc. The modified code requires AVX2
*/
    __m256 exp_hi = _mm256_set1_ps(88.3762626647949f);
    __m256 exp_lo = _mm256_set1_ps(-88.3762626647949f);

    __m256 cephes_LOG2EF = _mm256_set1_ps(1.44269504088896341);
    __m256 cephes_exp_C1 = _mm256_set1_ps(0.693359375);
    __m256 cephes_exp_C2 = _mm256_set1_ps(-2.12194440e-4);

    __m256 cephes_exp_p0 = _mm256_set1_ps(1.9875691500E-4);
    __m256 cephes_exp_p1 = _mm256_set1_ps(1.3981999507E-3);
    __m256 cephes_exp_p2 = _mm256_set1_ps(8.3334519073E-3);
    __m256 cephes_exp_p3 = _mm256_set1_ps(4.1665795894E-2);
    __m256 cephes_exp_p4 = _mm256_set1_ps(1.6666665459E-1);
    __m256 cephes_exp_p5 = _mm256_set1_ps(5.0000001201E-1);
    __m256 tmp = _mm256_setzero_ps(), fx;
    __m256i imm0;
    __m256 one = _mm256_set1_ps(1.0f);

    x = _mm256_min_ps(x, exp_hi);
    x = _mm256_max_ps(x, exp_lo);

    /* express exp(x) as exp(g + n*log(2)) */
    fx = _mm256_mul_ps(x, cephes_LOG2EF);
    fx = _mm256_add_ps(fx, _mm256_set1_ps(0.5f));
    tmp = _mm256_floor_ps(fx);
    __m256 mask = _mm256_cmp_ps(tmp, fx, _CMP_GT_OS);
    mask = _mm256_and_ps(mask, one);
    fx = _mm256_sub_ps(tmp, mask);
    tmp = _mm256_mul_ps(fx, cephes_exp_C1);
    __m256 z = _mm256_mul_ps(fx, cephes_exp_C2);
    x = _mm256_sub_ps(x, tmp);
    x = _mm256_sub_ps(x, z);
    z = _mm256_mul_ps(x, x);

    __m256 y = cephes_exp_p0;
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, cephes_exp_p1);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, cephes_exp_p2);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, cephes_exp_p3);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, cephes_exp_p4);
    y = _mm256_mul_ps(y, x);
    y = _mm256_add_ps(y, cephes_exp_p5);
    y = _mm256_mul_ps(y, z);
    y = _mm256_add_ps(y, x);
    y = _mm256_add_ps(y, one);

    /* build 2^n */
    imm0 = _mm256_cvttps_epi32(fx);
    imm0 = _mm256_add_epi32(imm0, _mm256_set1_epi32(0x7f));
    imm0 = _mm256_slli_epi32(imm0, 23);
    __m256 pow2n = _mm256_castsi256_ps(imm0);
    y = _mm256_mul_ps(y, pow2n);
    return y;
}
#endif

std::pair<float, int> calc_harmonic_mean2(int8_t* data, size_t n) {
    float harmonic_mean = 0;
    int num_zeros = 0;
    auto* p = data;
    const auto end = data + n;
#if defined(__AVX2__)
    constexpr auto BLOCK_SIZE = sizeof(__m256i);
    const auto end0 = data + (n & ~(BLOCK_SIZE - 1));
    const auto ln2 = _mm256_set1_ps(0.69314718055995f);
    const auto zerof32 = _mm256_setzero_ps();
    const auto zeroi8 = _mm256_setzero_si256();
    auto sumf32 = _mm256_setzero_ps();
    for (; p < end0; p += BLOCK_SIZE) {
        auto d = _mm256_load_si256(reinterpret_cast<__m256i*>(p));
        num_zeros += _mm_popcnt_u32(_mm256_movemask_epi8(_mm256_cmpeq_epi8(d, zeroi8)));

        auto pp = p;
        for (int i = 0; i < 4; ++i) {
            auto x = _mm256_set_ps(pp[0], pp[1], pp[2], pp[3], pp[4], pp[5], pp[6], pp[7]);
            sumf32 = _mm256_add_ps(exp256_ps((_mm256_mul_ps(_mm256_sub_ps(zerof32, x), ln2))), sumf32);
            pp += 8;
        }
    }
    for (int i = 0; i < sizeof(sumf32) / sizeof(float); ++i) {
        harmonic_mean += (reinterpret_cast<float*>(&sumf32))[i];
    }
#endif
    for (; p < end; ++p) {
        harmonic_mean += powf(2.0f, p[0]);
        if (p[0] == 0) {
            ++num_zeros;
        }
    }

    harmonic_mean = 1.0f / harmonic_mean;
    return std::make_pair(harmonic_mean, num_zeros);
}

std::pair<float, int> calc_harmonic_mean3(int8_t* data, size_t n) {
    float harmonic_mean = 0;
    int num_zeros = 0;

    for (int i = 0; i < n; ++i) {
        harmonic_mean += 1.0f / static_cast<float>((1L << data[i]));
        if (data[i] == 0) {
            ++num_zeros;
        }
    }
    harmonic_mean = 1.0f / harmonic_mean;
    return std::make_pair(harmonic_mean, num_zeros);
}

static float tables[65] = {
        1.0f / static_cast<float>(1L << 0),  1.0f / static_cast<float>(1L << 1),  1.0f / static_cast<float>(1L << 2),
        1.0f / static_cast<float>(1L << 3),  1.0f / static_cast<float>(1L << 4),  1.0f / static_cast<float>(1L << 5),
        1.0f / static_cast<float>(1L << 6),  1.0f / static_cast<float>(1L << 7),  1.0f / static_cast<float>(1L << 8),
        1.0f / static_cast<float>(1L << 9),  1.0f / static_cast<float>(1L << 10), 1.0f / static_cast<float>(1L << 11),
        1.0f / static_cast<float>(1L << 12), 1.0f / static_cast<float>(1L << 13), 1.0f / static_cast<float>(1L << 14),
        1.0f / static_cast<float>(1L << 15), 1.0f / static_cast<float>(1L << 16), 1.0f / static_cast<float>(1L << 17),
        1.0f / static_cast<float>(1L << 18), 1.0f / static_cast<float>(1L << 19), 1.0f / static_cast<float>(1L << 20),
        1.0f / static_cast<float>(1L << 21), 1.0f / static_cast<float>(1L << 22), 1.0f / static_cast<float>(1L << 23),
        1.0f / static_cast<float>(1L << 24), 1.0f / static_cast<float>(1L << 25), 1.0f / static_cast<float>(1L << 26),
        1.0f / static_cast<float>(1L << 27), 1.0f / static_cast<float>(1L << 28), 1.0f / static_cast<float>(1L << 29),
        1.0f / static_cast<float>(1L << 30), 1.0f / static_cast<float>(1L << 31), 1.0f / static_cast<float>(1L << 32),
        1.0f / static_cast<float>(1L << 33), 1.0f / static_cast<float>(1L << 34), 1.0f / static_cast<float>(1L << 35),
        1.0f / static_cast<float>(1L << 36), 1.0f / static_cast<float>(1L << 37), 1.0f / static_cast<float>(1L << 38),
        1.0f / static_cast<float>(1L << 39), 1.0f / static_cast<float>(1L << 40), 1.0f / static_cast<float>(1L << 41),
        1.0f / static_cast<float>(1L << 42), 1.0f / static_cast<float>(1L << 43), 1.0f / static_cast<float>(1L << 44),
        1.0f / static_cast<float>(1L << 45), 1.0f / static_cast<float>(1L << 46), 1.0f / static_cast<float>(1L << 47),
        1.0f / static_cast<float>(1L << 48), 1.0f / static_cast<float>(1L << 49), 1.0f / static_cast<float>(1L << 50),
        1.0f / static_cast<float>(1L << 51), 1.0f / static_cast<float>(1L << 52), 1.0f / static_cast<float>(1L << 53),
        1.0f / static_cast<float>(1L << 54), 1.0f / static_cast<float>(1L << 55), 1.0f / static_cast<float>(1L << 56),
        1.0f / static_cast<float>(1L << 57), 1.0f / static_cast<float>(1L << 58), 1.0f / static_cast<float>(1L << 59),
        1.0f / static_cast<float>(1L << 60), 1.0f / static_cast<float>(1L << 61), 1.0f / static_cast<float>(1L << 62),
        1.0f / static_cast<float>(1L << 63), 1.0f / static_cast<float>(1L << 64),
};
std::pair<float, int> calc_harmonic_mean4(int8_t* data, size_t n) {
    float harmonic_mean = 0;
    int num_zeros = 0;

    for (int i = 0; i < n; ++i) {
        harmonic_mean += tables[data[i]];
        if (data[i] == 0) {
            ++num_zeros;
        }
    }
    harmonic_mean = 1.0f / harmonic_mean;
    return std::make_pair(harmonic_mean, num_zeros);
}
