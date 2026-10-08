#include "Instruction.h"
#include "Parser.h"
#include <iostream>
#include <cassert>
#include <string>

using namespace compiler;

void testOperands() {
    std::cout << "[Test Person 1] Testing Operand creation and comparisons...\n";
    Operand r0 = Operand::makeRegister("R0");
    Operand r0_lower = Operand::makeRegister("r0");
    assert(r0 == r0_lower);
    assert(r0.isRegister());
    assert(r0.toString() == "R0");

    Operand imm = Operand::makeImmediate(42);
    assert(imm.isImmediate());
    assert(imm.immediateValue == 42);
    assert(imm.toString() == "#42");

    Operand mem = Operand::makeMemory("[R1 + 8]", "R1", 8);
    assert(mem.isMemory());
    assert(mem.baseRegister == "R1");
    assert(mem.memoryOffset == 8);

    Operand lbl = Operand::makeLabel("loop_start");
    assert(lbl.isLabel());
    assert(lbl.toString() == "loop_start");

    std::cout << "  -> Operand tests passed!\n";
}

void testInstructionAnalysis() {
    std::cout << "[Test Person 1] Testing Instruction read/write register analysis...\n";
    Instruction mov = Instruction::make(Opcode::MOV, Operand::makeRegister("R0"), Operand::makeRegister("R1"));
    assert(mov.writesRegister("R0"));
    assert(!mov.writesRegister("R1"));
    assert(mov.readsRegister("R1"));
    assert(!mov.readsRegister("R0"));

    Instruction add2 = Instruction::make(Opcode::ADD, Operand::makeRegister("R0"), Operand::makeRegister("R2"));
    // 2-address ADD reads both R0 and R2, and writes R0
    assert(add2.writesRegister("R0"));
    assert(add2.readsRegister("R0"));
    assert(add2.readsRegister("R2"));

    Instruction st = Instruction::make(Opcode::STORE, Operand::makeRegister("R0"), Operand::makeMemory("[R1]", "R1", 0));
    assert(!st.writesRegister("R0"));
    assert(st.readsRegister("R0"));
    assert(st.readsRegister("R1"));

    std::cout << "  -> Instruction analysis tests passed!\n";
}

void testParser() {
    std::cout << "[Test Person 1] Testing Assembly Parser and Lexer...\n";
    std::string testCode =
        "; Comment line\n"
        "start:\n"
        "    MOV R0, #10\n"
        "    LOAD R1, [R2 + 4]   ; Load with offset\n"
        "    STORE R0, [R3]      // C++ style comment\n"
        "    ADD R0, R1\n"
        "    JMP exit\n"
        "exit:\n"
        "    RET\n";

    Parser parser;
    auto insts = parser.parseString(testCode);

    assert(insts.size() == 8);
    assert(insts[0].isStandaloneLabel() && insts[0].label == "start");
    assert(insts[1].opcode == Opcode::MOV);
    assert(insts[1].operands[0] == Operand::makeRegister("R0"));
    assert(insts[1].operands[1] == Operand::makeImmediate(10));

    assert(insts[2].opcode == Opcode::LOAD);
    assert(insts[2].operands[1].isMemory());
    assert(insts[2].operands[1].baseRegister == "R2");
    assert(insts[2].operands[1].memoryOffset == 4);

    assert(insts[3].opcode == Opcode::STORE);
    assert(insts[4].opcode == Opcode::ADD);
    assert(insts[5].opcode == Opcode::JMP);
    assert(insts[6].isStandaloneLabel() && insts[6].label == "exit");
    assert(insts[7].opcode == Opcode::RET);

    // Test emission
    std::string emitted = Parser::emitAssembly(insts);
    assert(emitted.find("MOV R0, #10") != std::string::npos);
    assert(emitted.find("exit:") != std::string::npos);

    std::cout << "  -> Parser and Emitter tests passed!\n";
}

int main() {
    std::cout << "========================================\n";
    std::cout << "  RUNNING PERSON 1 UNIT TEST SUITE      \n";
    std::cout << "========================================\n";
    testOperands();
    testInstructionAnalysis();
    testParser();
    std::cout << "\nALL PERSON 1 TESTS PASSED SUCCESSFULLY!\n";
    return 0;
}
