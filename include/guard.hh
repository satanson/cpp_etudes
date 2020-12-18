// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com

//
// Created by grakra on 2020/12/18.
//

#pragma once
#include<type_traits>
namespace guard {

using Guard = int;

template<typename Op, typename... Args>
using TypeGuard = std::enable_if_t<(std::is_same_v<Op, Args> || ...), Guard>;

template<typename T, T v, T... args>
using ValueGuard = std::enable_if_t<((v == args) || ...), Guard>;

} // namespace guard
