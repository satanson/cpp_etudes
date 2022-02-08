// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/19.
//

#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
namespace interpreters {
enum opcode : int8_t {
    OP_LITERAL,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_END,
};

static inline int32_t identity(int32_t a) {
    return a;
}
template <opcode OP, typename... Args>
int32_t make_instruction(Args&&... args) {
    int32_t instr = 0;
    instr |= OP << 24;
    if constexpr (OP == OP_LITERAL) {
        instr |= identity(std::forward<Args>(args)...);
    }
    return instr;
}
int32_t switch_dispatch(const int32_t* instructions, size_t n, int32_t* stack);
int32_t token_threaded(const int32_t* instructions, size_t n, int32_t* stack);
int32_t direct_threaded(const int32_t* instructions, size_t n, int32_t* stack, int32_t* operands, void**target_addresses);
} // namespace interpreters