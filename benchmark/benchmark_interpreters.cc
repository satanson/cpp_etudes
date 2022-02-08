// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/19.
//

#include "benchmark/benchmark.h"
#include "interpreters/interpreters.hh"
namespace interpreters {
void BM_switch_dispatch(benchmark::State& state) {
    std::vector<int32_t> instructions{
            make_instruction<OP_LITERAL>(10), make_instruction<OP_LITERAL>(11), make_instruction<OP_ADD>(),
            make_instruction<OP_LITERAL>(12), make_instruction<OP_MUL>(),
    };
    std::vector<int32_t> stack(instructions.size());
    for (auto _ : state) {
        switch_dispatch(&instructions.front(), instructions.size(), &stack.front());
    }
}

void BM_token_threaded(benchmark::State& state) {
    std::vector<int32_t> instructions{
            make_instruction<OP_LITERAL>(10), make_instruction<OP_LITERAL>(11), make_instruction<OP_ADD>(),
            make_instruction<OP_LITERAL>(12), make_instruction<OP_MUL>(),       make_instruction<OP_END>(),
    };
    std::vector<int32_t> stack(instructions.size());
    for (auto _ : state) {
        token_threaded(&instructions.front(), instructions.size(), &stack.front());
    }
}

void BM_direct_threaded(benchmark::State& state) {
    std::vector<int32_t> instructions{
        make_instruction<OP_LITERAL>(10), make_instruction<OP_LITERAL>(11), make_instruction<OP_ADD>(),
        make_instruction<OP_LITERAL>(12), make_instruction<OP_MUL>(),       make_instruction<OP_END>(),
        };
    std::vector<int32_t> stack(instructions.size());
    std::vector<int32_t> operands(instructions.size());
    std::vector<void*> target_addresses(instructions.size());
    for (auto _ : state) {
        direct_threaded(&instructions.front(), instructions.size(), &stack.front(), &operands.front(), &target_addresses.front());
    }
}
BENCHMARK(BM_switch_dispatch);
BENCHMARK(BM_token_threaded);
BENCHMARK(BM_direct_threaded);
} // namespace interpreters
BENCHMARK_MAIN();