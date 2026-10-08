#include "Parser.h"
#include "PeepholeOptimizer.h"
#include "ControlFlowOptimizer.h"
#include <iostream>
#include <cassert>

using namespace compiler;

void testRedundantJump() {
    std::string code = 
        "JMP L1\n"
        "L1:\n"
        "MOV R0, #1\n";
    Parser parser;
    auto instructions = parser.parseString(code);
    PeepholeOptimizer opt;
    auto optimized = opt.optimize(instructions);
    assert(optimized.size() == 2);
    assert(optimized[0].opcode == Opcode::LABEL_DEF);
    assert(optimized[0].label == "L1");
}

void testJumpChaining() {
    std::string code = 
        "JMP L1\n"
        "MOV R0, #1\n"
        "L1:\n"
        "JMP L2\n"
        "L2:\n"
        "MOV R1, #2\n";
    Parser parser;
    auto instructions = parser.parseString(code);
    PeepholeOptimizer opt;
    auto optimized = opt.optimize(instructions);
    
    assert(optimized[0].opcode == Opcode::JMP);
    assert(optimized[0].operands[0].text == "L2");
}

void testJumpOverJump() {
    std::string code = 
        "BEQ L1\n"
        "JMP L2\n"
        "L1:\n"
        "MOV R0, #1\n";
    Parser parser;
    auto instructions = parser.parseString(code);
    PeepholeOptimizer opt;
    auto optimized = opt.optimize(instructions);
    
    assert(optimized[0].opcode == Opcode::BNE);
    assert(optimized[0].operands[0].text == "L2");
    assert(optimized[1].opcode == Opcode::LABEL_DEF);
    assert(optimized[1].label == "L1");
}

void testUnreachableCode() {
    std::string code = 
        "JMP L1\n"
        "MOV R0, #1\n"
        "ADD R0, R0\n"
        "L1:\n"
        "MOV R1, #2\n";
    Parser parser;
    auto instructions = parser.parseString(code);
    PeepholeOptimizer opt;
    auto optimized = opt.optimize(instructions);
    
    assert(optimized.size() == 2);
    assert(optimized[0].opcode == Opcode::LABEL_DEF);
    assert(optimized[1].opcode == Opcode::MOV);
}

int main() {
    testRedundantJump();
    testJumpChaining();
    testJumpOverJump();
    testUnreachableCode();
    std::cout << "Control Flow tests passed!\n";
    return 0;
}
