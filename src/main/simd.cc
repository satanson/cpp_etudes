//
// Created by grakra on 23-1-6.
//

#include <immintrin.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <vector>
float calc(int8_t* p) {
    auto d = _mm256_load_si256(reinterpret_cast<__m256i*>(p));
    auto x0 = _mm256_setzero_ps();
    auto y0 = _mm256_setzero_ps();
    auto* p8 = reinterpret_cast<int8_t*>(&d);
    for (auto i = 0; i < 4; ++i) {
        p8 += 8;
        y0 = _mm256_set_ps(p8[0], p8[1], p8[2], p8[3], p8[4], p8[5], p8[6], p8[7]);
        x0 = _mm256_add_ps(x0, y0);
    }
    //x0 = _mm256_add_ps(x0, _mm256_srli_si256(x0, 16));
    //x0 = _mm256_add_ps(x0, _mm256_srli_si256(x0, 8));
    //x0 = _mm256_add_ps(x0, _mm256_srli_si256(x0, 4));
    return _mm256_cvtss_f32(x0);
}

int main(int argc, char**argv) {
    std::vector<int8_t> data;
    data.resize(256);
    for (int i=0; i <256; ++i) {
        data[i] = i;
    }
    auto r = calc(data.data());
    std::cout<<r<<std::endl;
}