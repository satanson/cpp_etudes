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
#include <absl/container/flat_hash_map.h>
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
    std::vector<int> a{1, 2, 4};
    std::vector<int> b{1, 2, 4};
    int choose = std::rand();
    auto& c = choose ? a : b;
    std::cout << &a << std::endl;
    std::cout << &b << std::endl;
    std::cout << &c << std::endl;
}
#include<list>
TEST_F(TestUtil, testFlatHashMap) {
    absl::flat_hash_map<int, int> abc;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int8_t> r;
    std::uniform_int_distribution<size_t> rsize(1, 1000);
    for (int i = 0; i < 100000; ++i) {
        const auto size = rsize(gen);
        for (int k = 0; k < size; ++k) {
            abc[r(gen)] = r(gen);
        }
        for (const auto& [a, b] : abc) {
            std::cout << "k=" << a << ", v=" << b << std::endl;
        }
    }
}

TEST_F(TestUtil, testVectorResize){
    std::vector<int> data;
    for(int i=0;i < 100; i++){
        data.resize(i*100);
        std::cout<<"size="<<i*100 <<", cap="<<data.capacity()<<std::endl;
    }
}

TEST_F(TestUtil, TestUnsignedIntegerOverflow){
    uint32_t a = 4294967295;
    int32_t c = a;
    int64_t b =  c;
    std::cout<<b<<std::endl;
}

TEST_F(TestUtil,TestListTransform){
    std::list<int> l0{1,2,3,4,5};
    std::list<int> l1;
    l1.resize(10);
    std::transform(l0.begin(), l0.end(), l1.begin(), [](int x){return x*2;});
    std::cout<<"size="<<l1.size()<<std::endl;
    for (auto it = l1.begin(); it!=l1.end(); ++it) {
        std::cout<<*it<<std::endl;
    }
}

struct VectorWrapper{
    VectorWrapper(std::vector<std::string>&& s):_s(std::move(s)){
        std::cout<<"VectorWrapper ctor: size="<<_s.size()<<std::endl;
    }
    ~VectorWrapper(){
        std::cout<<"VectorWrapper dtor: size="<<_s.size()<<std::endl;
    }
private:
    std::vector<std::string> _s;
};
TEST_F(TestUtil, testVectorWrapper){
    {
        std::vector<std::string> s{"abc", "def"};
        auto a = std::make_tuple(std::make_shared<std::vector<std::string>>(std::move(s)), std::make_shared<VectorWrapper>(std::move(s)));
    }
    {
        std::vector<std::string> s{"abc", "def"};
        auto a = std::make_tuple( std::make_shared<VectorWrapper>(std::move(s)),std::make_shared<std::vector<std::string>>(std::move(s)));
    }
    {
        std::vector<std::string> s{"abc", "def"};
        auto a = std::make_tuple(std::make_shared<VectorWrapper>(std::move(s)), std::make_shared<VectorWrapper>(std::move(s)));
    }
}
char urlDecode(const char* a) {
    auto l = a[0];
    auto r = a[1];
    auto mask = (l - 'A') >> 8;
    auto ch = ((l - 'A' + 10) & (~mask)) + ((l - '0') & mask);
    mask = (r - 'A') >> 8;
    ch = (ch << 4) + ((r - 'A' + 10) & (~mask)) + ((r - '0') & mask);
    return ch;
}
TEST_F(TestUtil, testUrlDecode) {

    std::string s = "%21\t%23\t%24\t%26\t%27\t%28\t%29\t%2A\t%2B\t%2C\t%2F\t%3A\t%3B\t%3D\t%3F\t%40\t%5B\t%5D";
    const char* p = s.data();
    while(p < s.data()+s.size()) {
        if (*p=='%' || *p == '\t') {
            ++p;
            continue;
        }

        std::cout<<urlDecode(p)<<std::endl;
        p+=2;
    }
}



} // namespace util
} // namespace grakra
} // namespace com

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
