// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2021/03/05
//

#pragma once
#include "guard.hh"
#include "meta_macro.hh"
namespace guard {

#define PRED_GUARD(guard_name, predicate, ...)                                 \
  template <META_MACRO_PAIR_LIST_CONCAT_WS(__VA_ARGS__)>                       \
  using guard_name =                                                           \
      std::enable_if_t<predicate<META_MACRO_PAIR_LIST_SECOND(__VA_ARGS__)>,    \
                       guard::Guard>;

#define DEF_PRED_GUARD(guard_name, pred_name, ...)                             \
  template <META_MACRO_PAIR_LIST_CONCAT_WS(__VA_ARGS__)>                       \
  struct pred_name##_struct {                                                  \
    static constexpr bool value = false;                                       \
  };                                                                           \
  template <META_MACRO_PAIR_LIST_CONCAT_WS(__VA_ARGS__)>                       \
  constexpr bool pred_name =                                                   \
      pred_name##_struct<META_MACRO_PAIR_LIST_SECOND(__VA_ARGS__)>::value;     \
                                                                               \
  PRED_GUARD(guard_name, pred_name, ##__VA_ARGS__)

#define DEF_PRED_CASE_CTOR(pred_name, ...)                                     \
  template <> struct pred_name##_struct<__VA_ARGS__> {                         \
    static constexpr bool value = true;                                        \
  };

} // namespace guard
