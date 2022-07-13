// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/7/2.
//
#include <gtest/gtest.h>

#include <random>
#include <util/bits_op.hh>
namespace com {
namespace grakra {
namespace util {
class TestUtil : public ::testing::Test {};
TEST_F(TestUtil, testReverseBits) {
    GTEST_LOG_(INFO) << reverse_bits(0x1);
    GTEST_LOG_(INFO) << reverse_bits(0x80000000);
    GTEST_LOG_(INFO) << reverse_bits(0xf0000000);
    std::vector<uint32_t> nums{0,          0x1,        0x9,        0xff,       0x11,       0x99,
                               0xff,       0x1111,     0x9999,     0xffff,     0x80000000, 0x90000000,
                               0xf0000000, 0x88000000, 0x99000000, 0xff000000, 0xffff0000, 0xffffffff,
                               0x99999999, 0x11111111, 0x88888888, 0x55555555, 0x59595959};
    for (auto n : nums) {
        ASSERT_EQ(n, reverse_bits(reverse_bits(n)));
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> rand(0, UINT32_MAX);
    for (auto i = 0; i < 10000; ++i) {
        auto n = rand(gen);
        ASSERT_EQ(n, reverse_bits(reverse_bits(n)));
    }
}
typedef int32_t JulianDate;
JulianDate from_date(int year, int month, int day) {
    JulianDate century;
    JulianDate julian;

    if (month > 2) {
        month += 1;
        year += 4800;
    } else {
        month += 13;
        year += 4799;
    }

    century = year / 100;
    julian = year * 365 - 32167;
    julian += year / 4 - century + century / 4;
    julian += 7834 * month / 256 + day;
    return julian;
}

inline void to_date(JulianDate julian, int* year, int* month, int* day) {
    static constexpr JulianDate ZERO_EPOCH_JULIAN = 1721028;
    if (julian == ZERO_EPOCH_JULIAN) {
        *year = 0;
        *month = 0;
        *day = 0;
        return;
    }

    int quad;
    int extra;
    int y;

    julian += 32044;
    quad = julian / 146097;
    extra = (julian - quad * 146097) * 4 + 3;
    julian += 60 + quad * 3 + extra / 146097;
    quad = julian / 1461;
    julian -= quad * 1461;
    y = julian * 4 / 1461;
    julian = ((y != 0) ? ((julian + 305) % 365) : ((julian + 306) % 366)) + 123;
    y += quad * 4;
    quad = julian * 2141 / 65536;

    *year = y - 4800;
    *day = julian - 7834 * quad / 256;
    *month = (quad + 10) % 12 + 1;
}

TEST_F(TestUtil, testDateConversion) {
    std::vector<std::tuple<int, int, int>> cases = {
            {0, 0, 0}, {1, 1, 1}, {0, 1, 1}, {1, 0, 1}, {1988, 12, 18},
    };
    for (auto& tc : cases) {
        JulianDate d = from_date(std::get<0>(tc), std::get<1>(tc), std::get<2>(tc));
        printf("%d\n", d);
        int year, mon, day;
        to_date(d, &year, &mon, &day);
        printf("%d-%d-%d\n", year, mon, day);
    }
}

TEST_F(TestUtil, testTerityExprReferenceAssignment) {
    std::vector<int> a{1,2,4};
    std::vector<int> b{1,2,4};
    int choose = std::rand();
    auto& c = choose? a:b;
    std::cout<<&a<<std::endl;
    std::cout<<&b<<std::endl;
    std::cout<<&c<<std::endl;
}

} // namespace util
} // namespace grakra
} // namespace com

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
