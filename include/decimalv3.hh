//
// Created by grakra on 2020/12/7.
//

#ifndef CPP_ETUDES_INCLUDE_DECIMALV3_HH_
#define CPP_ETUDES_INCLUDE_DECIMALV3_HH_
#include <cstdint>
#include <type_traits>
typedef __int128 int128_t;

template <typename T>
constexpr bool is_underlying_type_of_decimal = false;
template <>
constexpr bool is_underlying_type_of_decimal<int32_t> = true;
template <>
constexpr bool is_underlying_type_of_decimal<int64_t> = true;
template <>
constexpr bool is_underlying_type_of_decimal<int128_t> = true;

template <typename T, int n>
struct EXP10 {
    using type = std::enable_if_t<is_underlying_type_of_decimal<T>, T>;
    static constexpr type value = EXP10<T, n - 1>::value * static_cast<type>(10);
};

template <typename T>
struct EXP10<T, 0> {
    using type = std::enable_if_t<is_underlying_type_of_decimal<T>, T>;
    static constexpr type value = static_cast<type>(1);
};

#endif // CPP_ETUDES_INCLUDE_DECIMALV3_HH_
