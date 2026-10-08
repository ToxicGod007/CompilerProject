#include "Instruction.h"
#include "Parser.h"
#include "PeepholeOptimizer.h"
#include "OptimizationPatterns.h"
#include <iostream>
#include <cassert>
#include <string>

using namespace compiler;

void testRedundantLoadAfterStore() {
    std::cout << "[Test Person 2] Testing Redundant Load After Store...\n";
    // Case 1A: STORE R1, [M] followed by LOAD R1, [M] -> Eliminate LOAD
    std::string code1 = 
        "    STORE R1, [R0 + 4]\n"
        "    LOAD  R1, [R0 + 4]\n";
    Parser parser;
    auto insts1 = parser.parseString(code1);
    PeepholeOptimizer opt;
    auto opt1 = opt.optimize(insts1);
    assert(opt1.size() == 1);
    assert(opt1[0].opcode == Opcode::STORE);

    // Case 1B: STORE R1, [M] followed by LOAD R2, [M] -> Replace LOAD with MOV R2, R1
    std::string code2 = 
        "    STORE R1, [R0 + 4]\n"
        "    LOAD  R2, [R0 + 4]\n";
    auto insts2 = parser.parseString(code2);
    auto opt2 = opt.optimize(insts2);
    assert(opt2.size() == 2);
    assert(opt2[0].opcode == Opcode::STORE);
    assert(opt2[1].opcode == Opcode::MOV);
    assert(opt2[1].operands[0] == Operand::makeRegister("R2"));
    assert(opt2[1].operands[1] == Operand::makeRegister("R1"));

    std::cout << "  -> Redundant load tests passed!\n";
}

void testDeadStore() {
    std::cout << "[Test Person 2] Testing Dead Store Elimination...\n";
    std::string code = 
        "    STORE R1, [R0]\n"
        "    STORE R2, [R0]\n";
    Parser parser;
    auto insts = parser.parseString(code);
    PeepholeOptimizer opt;
    auto res = opt.optimize(insts);
    assert(res.size() == 1);
    assert(res[0].opcode == Opcode::STORE);
    assert(res[0].operands[0] == Operand::makeRegister("R2"));
    std::cout << "  -> Dead store test passed!\n";
}

void testRedundantMoves() {
    std::cout << "[Test Person 2] Testing Redundant Move Elimination...\n";
    // Self-move
    std::string code1 = "    MOV R3, R3\n";
    Parser parser;
    auto insts1 = parser.parseString(code1);
    PeepholeOptimizer opt;
    auto res1 = opt.optimize(insts1);
    assert(res1.empty());

    // Reversible move: MOV R1, R2 followed by MOV R2, R1
    std::string code2 = 
        "    MOV R1, R2\n"
        "    MOV R2, R1\n";
    auto insts2 = parser.parseString(code2);
    auto res2 = opt.optimize(insts2);
    assert(res2.size() == 1);
    assert(res2[0].opcode == Opcode::MOV);
    assert(res2[0].operands[0] == Operand::makeRegister("R1"));
    assert(res2[0].operands[1] == Operand::makeRegister("R2"));

    std::cout << "  -> Redundant move tests passed!\n";
}

void testAlgebraicSimplification() {
    std::cout << "[Test Person 2] Testing Algebraic Identity Simplifications...\n";
    std::string code = 
        "    ADD R1, #0\n"
        "    SUB R2, #0\n"
        "    MUL R3, #1\n"
        "    DIV R4, #1\n"
        "    MUL R5, #0\n"
        "    SUB R6, R6\n";
    Parser parser;
    auto insts = parser.parseString(code);
    PeepholeOptimizer opt;
    auto res = opt.optimize(insts);

    assert(res.size() == 2);
    // MUL R5, #0 becomes MOV R5, #0
    assert(res[0].opcode == Opcode::MOV);
    assert(res[0].operands[0] == Operand::makeRegister("R5"));
    assert(res[0].operands[1] == Operand::makeImmediate(0));

    // SUB R6, R6 becomes MOV R6, #0
    assert(res[1].opcode == Opcode::MOV);
    assert(res[1].operands[0] == Operand::makeRegister("R6"));
    assert(res[1].operands[1] == Operand::makeImmediate(0));

    std::cout << "  -> Algebraic simplification tests passed!\n";
}

void testStrengthReduction() {
    std::cout << "[Test Person 2] Testing Strength Reduction...\n";
    std::string code = 
        "    MUL R1, #2\n"
        "    ADD R2, #1\n"
        "    SUB R3, #1\n";
    Parser parser;
    auto insts = parser.parseString(code);
    PeepholeOptimizer opt;
    auto res = opt.optimize(insts);

    assert(res.size() == 3);
    assert(res[0].opcode == Opcode::LSL);
    assert(res[0].operands[0] == Operand::makeRegister("R1"));
    assert(res[0].operands[1] == Operand::makeImmediate(1));

    assert(res[1].opcode == Opcode::INC);
    assert(res[1].operands[0] == Operand::makeRegister("R2"));

    assert(res[2].opcode == Opcode::DEC);
    assert(res[2].operands[0] == Operand::makeRegister("R3"));

    std::cout << "  -> Strength reduction tests passed!\n";
}

void testConstantFolding() {
    std::cout << "[Test Person 2] Testing Constant Folding...\n";
    std::string code = 
        "    MOV R1, #20\n"
        "    ADD R1, #15\n"
        "    MOV R2, #50\n"
        "    SUB R2, #10\n";
    Parser parser;
    auto insts = parser.parseString(code);
    PeepholeOptimizer opt;
    auto res = opt.optimize(insts);

    assert(res.size() == 2);
    assert(res[0].opcode == Opcode::MOV);
    assert(res[0].operands[0] == Operand::makeRegister("R1"));
    assert(res[0].operands[1] == Operand::makeImmediate(35));

    assert(res[1].opcode == Opcode::MOV);
    assert(res[1].operands[0] == Operand::makeRegister("R2"));
    assert(res[1].operands[1] == Operand::makeImmediate(40));

    std::cout << "  -> Constant folding tests passed!\n";
}

void testLabelPreservation() {
    std::cout << "[Test Person 2] Testing Label Safety Invariance...\n";
    // Crucial safety test: optimizer must NOT optimize across labels
    std::string code = 
        "    STORE R1, [R0]\n"
        "target_label:\n"
        "    LOAD  R1, [R0]\n";
    Parser parser;
    auto insts = parser.parseString(code);
    PeepholeOptimizer opt;
    auto res = opt.optimize(insts);

    // Must NOT eliminate LOAD, because target_label could be jumped to with R1 clobbered!
    assert(res.size() == 3);
    assert(res[0].opcode == Opcode::STORE);
    assert(res[1].isStandaloneLabel());
    assert(res[2].opcode == Opcode::LOAD);

    std::cout << "  -> Label safety invariance passed!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  RUNNING PERSON 2 UNIT TEST SUITE      \n";
    std::cout << "========================================\n";
    testRedundantLoadAfterStore();
    testDeadStore();
    testRedundantMoves();
    testAlgebraicSimplification();
    testStrengthReduction();
    testConstantFolding();
    testLabelPreservation();
    std::cout << "\nALL PERSON 2 TESTS PASSED SUCCESSFULLY!\n";
    return 0;
}

