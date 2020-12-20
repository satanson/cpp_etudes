// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com

//
// Created by grakra on 2020/12/18.
//

#pragma once
#include <type_traits>
namespace guard {

using Guard = int;

template <typename T, typename... Args>
constexpr bool type_in = (std::is_same_v<T, Args> || ...);

template <typename T, T v, T... args>
constexpr bool value_in = ((v == args) || ...);

template <typename T, typename... Args>
using TypeGuard =
    std::enable_if_t<((std::is_same_v<T, Args>) || ...), guard::Guard>;

#define TYPE_GUARD(guard_name, pred_name, ...)                                 \
  template <typename T> struct pred_name##_struct {                            \
    static constexpr bool value = guard::type_in<T, ##__VA_ARGS__>;            \
  };                                                                           \
  template <typename T>                                                        \
  constexpr bool pred_name = pred_name##_struct<T>::value;                     \
  template <typename T> using guard_name = guard::TypeGuard<T, ##__VA_ARGS__>;

template <typename T, T v, T... args>
using ValueGuard = std::enable_if_t<((v == args) || ...), guard::Guard>;

#define VALUE_GUARD(type, guard_name, pred_name, ...)                          \
  template <type v> struct pred_name##_struct {                                \
    static constexpr bool value = guard::value_in<type, v, ##__VA_ARGS__>;     \
  };                                                                           \
  template <type v> constexpr bool pred_name = pred_name##_struct<v>::value;   \
  template <type v>                                                            \
  using guard_name = guard::ValueGuard<type, v, ##__VA_ARGS__>;

template <typename T, template <typename> typename... TypePredicates>
struct OrTypePredicates {
  static constexpr bool value = ((TypePredicates<T>::value) || ...);
};

template <typename T, T v, template <T> typename... ValuePredicates>
struct OrValuePredicates {
  static constexpr bool value = ((ValuePredicates<v>::value) || ...);
};

template <typename T, template <typename> typename... TypePredicates>
constexpr bool type_union = OrTypePredicates<T, TypePredicates...>::value;
template <typename T, template <typename> typename... TypePredicates>
using TypeGuardUnion =
    std::enable_if_t<type_union<T, TypePredicates...>, Guard>;

template <typename T, T v, template <T> typename... ValuePredicates>
constexpr bool value_union = OrValuePredicates<T, v, ValuePredicates...>::value;
template <typename T, T v, template <T> typename... ValuePredicates>
using ValueGuardUnion =
    std::enable_if_t<value_union<T, v, ValuePredicates...>, Guard>;

#define UNION_TYPE_GUARD(guard_name, pred_name, ...)                           \
  template <typename T> struct pred_name##_struct {                            \
    static constexpr bool value = guard::type_union<T, ##__VA_ARGS__>;         \
  };                                                                           \
  template <typename T>                                                        \
  constexpr bool pred_name = pred_name##_struct<T>::value;                     \
  template <typename T>                                                        \
  using guard_name = std::enable_if_t<pred_name<T>, guard::Guard>;

#define UNION_VALUE_GUARD(type, guard_name, pred_name, ...)                    \
  template <type v> struct pred_name##_struct {                                \
    static constexpr bool value = guard::value_union<type, v, ##__VA_ARGS__>;  \
  };                                                                           \
  template <type v> constexpr bool pred_name = pred_name##_struct<v>::value;   \
  template <type v>                                                            \
  using guard_name = guard::ValueGuardUnion<type, v, ##__VA_ARGS__>;

} // namespace guard
