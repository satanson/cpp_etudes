//
// Created by grakra on 23-7-22.
//

#pragma once
#include <immintrin.h>
#include <mmintrin.h>

#include <random>
#include <string>
char hexdigit_ord(char ch) {
    const auto bs = _mm_set_pi8(0x7f, 'f', 'a' - 1, 'F', 'A' - 1, '9', '0' - 1, 0x00);
    auto chs = _mm_set1_pi8(ch);
    auto x = _mm_sub_pi8(bs, chs);
    auto bits = 32 - __builtin_clz(_mm_movemask_pi8(x));
    // x         legal  bits  range
    // x1111111   N  6     'z' < ch
    // 00111111   Y  5     'a' <= ch <= 'z'
    // 00011111   N  4     'a' < ch < 'Z'
    // 00001111   Y  3     'Z' <= ch <= 'A'
    // 00000111   N  2     '9' < ch < 'A'
    // 00000011   Y  1     '0' <= ch <= '9'
    // 00000001   N  0     ch < '0'

    // if bits == 0x1; then t = -1;otherwise t = 9;
    auto t = (10 & (0xff << ((bits == 0b10) << 3))) - 1;
    if (bits & 0b1001) {
        return 0xff;
    }
    // ch in 0..9; 1st byte is ('0' - 1 - ch), so obtain (ch - '0') from -1 - ('0' - 1 - ch)
    // ch in A..F; 3st byte is ('A' - 1 - ch), so obtain (ch - 'A' + 10) from -9 - ('A' - 1 - ch)
    // ch in a..f; 5st byte is ('a' - 1 - ch), so obtain (ch - 'a' + 10) from -9 - ('a' - 1 - ch)
    return static_cast<char>(t - ((_mm_cvtm64_si64(x) >> ((bits - 1) << 3) & 0xff)));
}

bool hexdigit_ord4(char ch0, char ch1, char ch2, char ch3, char* ret0, char* ret1) {
    const auto bs = _mm256_set_epi64x(0x6660'4640'392f'0000, 0x6660'4640'392f'0000, 0x6660'4640'392f'0000,
                                      0x6660'4640'392f'0000);
    auto chs = _mm256_set_epi8(ch3, ch3, ch3, ch3, ch3, ch3, '\1', '\1', ch2, ch2, ch2, ch2, ch2, ch2, '\1', '\1', ch1,
                               ch1, ch1, ch1, ch1, ch1, '\1', '\1', ch0, ch0, ch0, ch0, ch0, ch0, '\1', '\1');

    auto x = _mm256_sub_epi8(bs, chs);
    auto mask = _mm256_movemask_epi8(x);
    // x         legal  bits  range
    // 11111111   N  6    'z' < ch
    // 01111111   Y  5    'a' <= ch <= 'z'
    // 00111111   N  4    'a' < ch < 'Z'
    // 00011111   Y  3    'Z' <= ch <= 'A'
    // 00001111   N  2    '9' < ch < 'A'
    // 00000111   Y  1    '0' <= ch <= '9'
    // 00000011   N  0     ch < '0'

    // if bits == 0x1; then t = -1;otherwise t = 9;
    // ch in 0..9; 1st byte is ('0' - 1 - ch), so obtain (ch - '0') from -1 - ('0' - 1 - ch)
    // ch in A..F; 3st byte is ('A' - 1 - ch), so obtain (ch - 'A' + 10) from -9 - ('A' - 1 - ch)
    // ch in a..f; 5st byte is ('a' - 1 - ch), so obtain (ch - 'a' + 10) from -9 - ('a' - 1 - ch)
#define PROCESS_CH(i, stmt)                                                                           \
    do {                                                                                              \
        auto mask0 = mask & 0xff;                                                                     \
        auto bits = 30 - __builtin_clz(mask0);                                                        \
        if ((bits & 0x1) == 0) {                                                                      \
            return false;                                                                             \
        }                                                                                             \
        auto t = (10 & (0xff << ((bits == 0b1) << 3))) - 1;                                           \
        auto bytes32 = (__v32qi)_mm256_srli_epi64(_mm256_permute4x64_epi64(x, (i)), (bits + 1) << 3); \
        auto delta = bytes32[0];                                                                      \
        stmt;                                                                                         \
    } while (0);

    PROCESS_CH(0, (*ret0 = static_cast<char>((t - delta) << 4)));
    mask >>= 8;
    PROCESS_CH(1, (*ret0 += static_cast<char>(t - delta)));
    mask >>= 8;
    PROCESS_CH(2, (*ret1 = static_cast<char>((t - delta) << 4)));
    mask >>= 8;
    PROCESS_CH(3, (*ret1 += static_cast<char>(t - delta)));
    return true;
}

char hexdigit_ord_nonsimd(char ch) {
    char c = 0;
    if (int value = ch - '0'; value >= 0 && value <= ('9' - '0')) {
        c += value;
    } else if (int value = ch - 'A'; value >= 0 && value <= ('F' - 'A')) {
        c += value + 10;
    } else if (int value = ch - 'a'; value >= 0 && value <= ('f' - 'a')) {
        c += value + 10;
    } else {
        return 0xff;
    }
    return c;
}

std::string unhex0(const std::string& s) {
    const auto sz = s.size();
    if (sz % 2 == 1) {
        return {};
    }
    std::string ret;
    ret.reserve(sz / 2);
    for (int i = 0; i < sz; i += 2) {
        auto ch0 = hexdigit_ord_nonsimd(s[i]);
        if ((ch0 & 0xff) == 0xff) {
            return {};
        }
        auto ch1 = hexdigit_ord_nonsimd(s[i + 1]);
        if ((ch1 & 0xff) == 0xff) {
            return {};
        }
        ret.push_back((ch0 << 4) + ch1);
    }
    return ret;
}

std::string unhex1(const std::string& s) {
    const auto sz = s.size();
    if (sz % 2 == 1) {
        return {};
    }
    std::string ret;
    ret.reserve(sz / 2);
    for (int i = 0; i < sz; i += 2) {
        auto ch0 = hexdigit_ord(s[i]);
        if ((ch0 & 0xff) == 0xff) {
            return {};
        }
        auto ch1 = hexdigit_ord(s[i + 1]);
        if ((ch1 & 0xff) == 0xff) {
            return {};
        }
        ret.push_back((ch0 << 4) + ch1);
    }
    return ret;
}

std::string unhex2(const std::string& s) {
    const auto sz = s.size();
    if (sz % 2 == 1) {
        return {};
    }
    std::string ret;
    ret.reserve(sz / 2);
    int i = 0;
    for (; i < sz - 3; i += 4) {
        char ch0, ch1;
        if (hexdigit_ord4(s[i], s[i + 1], s[i + 2], s[i + 3], &ch0, &ch1)) {
            ret.push_back(ch0);
            ret.push_back(ch1);
        } else {
            return {};
        }
    }
    if (i == sz) {
        return ret;
    }
    char ch0, ch1;
    if (hexdigit_ord4(s[i], s[i + 1], '0', '0', &ch0, &ch1)) {
        ret.push_back(ch0);
        return ret;
    } else {
        return {};
    }
}

struct PrepareData {
    PrepareData(std::string& s, size_t n) {
        s.resize(n);
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int64_t> idx(0, 21);
        const char* alphabet = "0123456789abcdefABCDEF";
        for (int i = 0; i < n; ++i) {
            s[i] = alphabet[idx(gen)];
        }
    }
};