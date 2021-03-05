// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/03/01.
//

#include <gtest/gtest.h>
#include <guard.hh>
#include "pred_guard.hh"
#include "meta_macro.hh"

namespace test {
class PredGuardTest : public ::testing::Test {};

template<typename S, typename T>
struct IsDirectlyCopyable {
    static constexpr bool value = false;
};

template<typename S, typename T>
constexpr bool is_directly_copyable = IsDirectlyCopyable<S, T>::value;

PRED_TYPE_GUARD(DirectlyCopyableGuard, is_directly_copyable, S, T);

#define IS_DIRECTLY_COPYABLE_CTOR(S, T) \
template<> \
struct IsDirectlyCopyable<S, T> {\
    static constexpr bool value = true; \
};

#define IS_DIRECTLY_COPYABLE(S, ...) \
DEF_BINARY_RELATION_ENTRY_SEP_NONE(IS_DIRECTLY_COPYABLE_CTOR, S, ##__VA_ARGS__)

template<typename S, typename T>
struct IsAssignable {
    static constexpr bool value = false;
};

template<typename S, typename T>
constexpr bool is_assignable = IsAssignable<S, T>::value;

PRED_TYPE_GUARD(AssignableGuard, is_assignable, S, T);

#define IS_ASSIGNABLE_CTOR(S, T) \
template<> \
struct IsAssignable<S, T> {\
    static constexpr bool value = true; \
};

#define IS_ASSIGNABLE(S, ...) \
DEF_BINARY_RELATION_ENTRY_SEP_NONE(IS_ASSIGNABLE_CTOR, S, ##__VA_ARGS__)

IS_DIRECTLY_COPYABLE(uint8_t, int8_t, uint8_t);
IS_DIRECTLY_COPYABLE(int8_t, int8_t, uint8_t);
IS_DIRECTLY_COPYABLE(uint16_t, int16_t, uint16_t);
IS_DIRECTLY_COPYABLE(int16_t, int16_t, uint16_t);
IS_DIRECTLY_COPYABLE(uint32_t, int32_t, uint32_t);
IS_DIRECTLY_COPYABLE(int32_t, int32_t, uint32_t);
IS_DIRECTLY_COPYABLE(uint64_t, int64_t, uint64_t);
IS_DIRECTLY_COPYABLE(int64_t, int64_t, uint64_t);

IS_ASSIGNABLE(uint8_t, uint16_t, int16_t, uint32_t, int32_t, int64_t, uint64_t)
IS_ASSIGNABLE(int8_t, uint16_t, int16_t, uint32_t, int32_t, int64_t, uint64_t)
IS_ASSIGNABLE(uint16_t, uint32_t, int32_t, int64_t, uint64_t)
IS_ASSIGNABLE(int16_t, uint32_t, int32_t, int64_t, uint64_t)
IS_ASSIGNABLE(uint32_t, int64_t, uint64_t)
IS_ASSIGNABLE(int32_t, int64_t, uint64_t)

template<typename S, typename T, typename=guard::Guard>
struct ArrayCopy {};

template<typename S, typename T>
struct ArrayCopy<S, T, DirectlyCopyableGuard<S, T>> {
    static void evaluate(const void *src, void *dst, size_t n) {
        memcpy(dst, src, sizeof(T) * n);
    }
};

template<typename S, typename T>
struct ArrayCopy<S, T, AssignableGuard<S, T>> {
    static void evaluate(const void *src, void *dst, size_t n) {
        const auto src0 = (const S *) src;
        auto dst0 = (T *) dst;
        for (auto i = 0; i < n; ++i) {
            dst0[i] = T(src0[i]);
        }
    }
};

template<typename S, typename T> constexpr int array_copy_idx =
    (sizeof(S) << 8) | (int(std::is_unsigned_v<S>) << 15) | sizeof(T) | (int(std::is_unsigned_v<T>) << 7);

#define COPY_ENTRY_CTOR(S, T) \
{array_copy_idx<S, T>, &ArrayCopy<S, T>::evaluate}

#define COPY_ENTRY(S, ...) DEF_BINARY_RELATION_ENTRY_SEP_COMMA(COPY_ENTRY_CTOR, S, ##__VA_ARGS__)
typedef void (*CopyFunc)(const void *, void *, size_t);

static const std::unordered_map<int, CopyFunc> global_copy_func_table = {
    COPY_ENTRY(int8_t, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t),
    COPY_ENTRY(uint8_t, int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t),
    COPY_ENTRY(int16_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t),
    COPY_ENTRY(uint16_t, int16_t, uint16_t, int32_t, uint32_t, int64_t, uint64_t),
    COPY_ENTRY(int32_t, int32_t, uint32_t, int64_t, uint64_t),
    COPY_ENTRY(uint32_t, int32_t, uint32_t, int64_t, uint64_t),
    COPY_ENTRY(int64_t, int64_t, uint64_t),
    COPY_ENTRY(uint64_t, int64_t, uint64_t),
};

template<typename S, typename T>
void array_copy(void *src, void *dst, size_t n) {
    const auto idx = array_copy_idx<S, T>;
    assert(global_copy_func_table.count(idx));
    global_copy_func_table.at(idx)(src, dst, n);
}

TEST_F(PredGuardTest, test_array_copy) {
    {
        std::vector<int8_t>
            a{int8_t(0), int8_t(255), int8_t(128), int8_t(127), int8_t(254), int8_t(252), int8_t(1), int8_t(2)};
        std::vector<int8_t> b(a.size(), 0);
        std::vector<uint8_t> c(a.size(), 0);
        std::vector<int8_t> a2(a.size(), 0);
        array_copy<int8_t, int8_t>(a.data(), b.data(), a.size());
        ASSERT_EQ(b[0], 0);
        ASSERT_EQ(b[1], (int8_t) 255);
        ASSERT_EQ(b[2], (int8_t) 128);
        ASSERT_EQ(b[3], (int8_t) 127);
        ASSERT_EQ(b[4], (int8_t) 254);
        ASSERT_EQ(b[5], (int8_t) 252);
        ASSERT_EQ(b[6], (int8_t) 1);
        ASSERT_EQ(b[7], (int8_t) 2);
        array_copy<int8_t, uint8_t>(a.data(), c.data(), a.size());
        array_copy<uint8_t, int8_t>(c.data(), a2.data(), c.size());
        ASSERT_EQ(a2[0], 0);
        ASSERT_EQ(a2[1], (int8_t) 255);
        ASSERT_EQ(a2[2], (int8_t) 128);
        ASSERT_EQ(a2[3], (int8_t) 127);
        ASSERT_EQ(a2[4], (int8_t) 254);
        ASSERT_EQ(a2[5], (int8_t) 252);
        ASSERT_EQ(a2[6], (int8_t) 1);
        ASSERT_EQ(a2[7], (int8_t) 2);
    }
    {
        std::vector<int8_t> a{int8_t(1), int8_t(2), int8_t(4)};
        std::vector<int32_t> b(a.size(), 0);
        array_copy<int8_t, int32_t>(a.data(), b.data(), 3);
        ASSERT_EQ(b[0], 1);
        ASSERT_EQ(b[1], 2);
        ASSERT_EQ(b[2], 4);
    }
}

}//namespace test

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}