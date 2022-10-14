// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/19.
//

#include "interpreters/interpreters.hh"

#include <algorithm>
#include <cassert>
#include <cstddef>
namespace interpreters {
int32_t switch_dispatch(const int32_t* instructions, size_t n, int32_t* stack) {
    size_t sp = 0;
    for (auto i = 0; i < n; ++i) {
        auto instr = instructions[i];
        auto op = opcode(instr >> 24);
        switch (op) {
        case OP_LITERAL: {
            stack[sp++] = (instr & (1 << 24) - 1);
            break;
        }
        case OP_ADD: {
            auto result = stack[--sp] + stack[--sp];
            stack[sp++] = result;
            break;
        }
        case OP_SUB: {
            auto result = stack[--sp] - stack[--sp];
            stack[sp++] = result;
            break;
        }
        case OP_DIV: {
            auto result = stack[--sp] / stack[--sp];
            stack[sp++] = result;
            break;
        }
        case OP_MUL: {
            auto result = stack[--sp] * stack[--sp];
            stack[sp++] = result;
            break;
        }
        case OP_MOD: {
            auto result = stack[--sp] % stack[--sp];
            stack[sp++] = result;
            break;
        }
        default:
            assert(false);
        }
    }
    return stack[0];
}

int32_t token_threaded(const int32_t* instructions, size_t n, int32_t* stack) {
    static void* dispatch_table[] = {&&op_literal, &&op_add, &&op_sub, &&op_mul, &&op_div, &&op_mod, &&op_end};
    auto* sp = stack + n - 1;
    const auto* ip = instructions;
    goto* dispatch_table[(*ip) >> 24];
op_literal:
    sp[0] = (*ip) & ((1 << 24) - 1);
    --sp;
    goto* dispatch_table[(*++ip) >> 24];
op_add : {
    sp[2] = sp[1] + sp[2];
    ++sp;
    goto* dispatch_table[(*++ip) >> 24];
}
op_sub : {
    sp[2] = sp[1] - sp[2];
    ++sp;
    goto* dispatch_table[(*++ip) >> 24];
}
op_div : {
    sp[2] = sp[1] / sp[2];
    ++sp;
    goto* dispatch_table[(*++ip) >> 24];
}
op_mul : {
    sp[2] = sp[1] * sp[2];
    ++sp;
    goto* dispatch_table[(*++ip) >> 24];
}
op_mod : {
    sp[2] = sp[1] % sp[2];
    ++sp;
    goto* dispatch_table[(*++ip) >> 24];
op_end:
    return sp[1];
}
}
int32_t direct_threaded(const int32_t* instructions, size_t n, int32_t* stack, int32_t* operands,
                        void** target_addresses) {
    static void* dispatch_table[] = {&&op_literal, &&op_add, &&op_sub, &&op_mul, &&op_div, &&op_mod, &&op_end};
    size_t k = 0;
    for (auto i = 0; i < n; ++i) {
        auto instr = instructions[i];
        target_addresses[i] = dispatch_table[instr >> 24];
        if (instr >> 24 == OP_LITERAL) {
            operands[k++] = instr & ((1 << 24) - 1);
        }
    }
    k = 0;
    auto* sp = stack + n - 1;
    const auto* ip = target_addresses;
    goto*(*ip++);
op_literal:
    sp[0] = operands[k++];
    --sp;
    goto*(*ip++);
op_add : {
    sp[2] = sp[1] + sp[2];
    ++sp;
    goto*(*ip++);
}
op_sub : {
    sp[2] = sp[1] - sp[2];
    ++sp;
    goto*(*ip++);
}
op_div : {
    sp[2] = sp[1] / sp[2];
    ++sp;
    goto*(*ip++);
}
op_mul : {
    sp[2] = sp[1] * sp[2];
    ++sp;
    goto*(*ip++);
}
op_mod : {
    sp[2] = sp[1] % sp[2];
    ++sp;
    goto*(*ip++);
op_end:
    return sp[1];
}
}
} // namespace interpreters