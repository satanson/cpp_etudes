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

// type guard based on typed predicate
#define EXPAND_ONE_TYPENAME(dummy, t) typename t

#define EXPAND_TYPENAME_LIST(dummy, ...) \
DEF_BINARY_RELATION_ENTRY_SEP_COMMA(EXPAND_ONE_TYPENAME, dummy, ##__VA_ARGS__)

#define PRED_TYPE_GUARD(guard_name, predicate, ...) \
template<EXPAND_TYPENAME_LIST(guard_name, ##__VA_ARGS__)> \
using guard_name = std::enable_if_t<predicate<__VA_ARGS__>, guard::Guard>;

// Value guard based on non-typed predicate
#define EXPAND_ONE_NON_TYPED(dummy, t, v) t v

#define EXPAND_ONE_NON_TYPED_LIST(dummy, t, ...) \
DEF_TERNARY_RELATION_ENTRY_SEP_COMMA_012(EXPAND_ONE_NON_TYPED, dummy, t, ##__VA_ARGS__)

#define PRED_VALUE_GUARD(t, guard_name, predicate, ...) \
template<EXPAND_NON_TYPED_LIST(t, t, ##__VA_ARGS__)> \
using guard_name = std::enable_if_t<predicate<__VA_ARGS__>, guard::Guard>;

}//namespace guard

