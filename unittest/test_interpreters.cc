// Copyright (c) 2020 Ran Panfeng.  All rights reserved.
// Author: satanson
// Email: ranpanf@gmail.com
// Github repository: https://github.com/satanson/cpp_etudes.git

//
// Created by grakra on 2020/11/19.
//

#include <gtest/gtest.h>

#include "interpreters/interpreters.hh"
namespace interpreters {
struct TestInterpreters : public testing::Test {};
TEST_F(TestInterpreters, TestBasic) {
    std::vector<int32_t> instructions{
            make_instruction<OP_LITERAL>(10), make_instruction<OP_LITERAL>(11), make_instruction<OP_ADD>(),
            make_instruction<OP_LITERAL>(12), make_instruction<OP_MUL>(),
    };
    std::vector<int32_t> stack(instructions.size());
    auto result = switch_dispatch(&instructions.front(), instructions.size(), &stack.front());
    ASSERT_EQ(result, 252);
}

TEST_F(TestInterpreters, TestThreaded) {
    std::vector<int32_t> instructions{
            make_instruction<OP_LITERAL>(10), make_instruction<OP_LITERAL>(11), make_instruction<OP_ADD>(),
            make_instruction<OP_LITERAL>(12), make_instruction<OP_MUL>(),       make_instruction<OP_END>(),
    };
    std::vector<int32_t> stack(instructions.size());
    auto result = direct_threading(&instructions.front(), instructions.size(), &stack.front());
    ASSERT_EQ(result, 252);
}
} // namespace interpreters
int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
