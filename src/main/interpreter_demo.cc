//
// Created by grakra on 2022/2/4.
//

#include "interpreters/interpreters.hh"
using interpreters::make_instruction;
using interpreters::opcode;
using interpreters::switch_dispatch;
int main(int argc, char** argv) {
    std::vector<int32_t> instructions{
            make_instruction<opcode::OP_LITERAL>(10), make_instruction<opcode::OP_LITERAL>(11),
            make_instruction<opcode::OP_ADD>(),       make_instruction<opcode::OP_LITERAL>(12),
            make_instruction<opcode::OP_MUL>(),       make_instruction<opcode::OP_END>(),
    };
    std::vector<int32_t> stack(instructions.size());
    auto result = switch_dispatch(&instructions.front(), instructions.size(), &stack.front());
    return result;
}
