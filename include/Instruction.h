#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

/**
 * ============================================================================
 * PERSON 1 WORK: Target Instruction Set Representation & Data Structures
 * ============================================================================
 * 
 * Defines the Target Architecture Instruction Set, Operand types, and IR
 * data structures used by the peephole optimizer.
 */

namespace compiler {

// ---------------------------------------------------------------------------
// Operand Types & Representation
// ---------------------------------------------------------------------------
enum class OperandType {
    NONE,
    REGISTER,       // e.g., R0, R1, SP, FP
    IMMEDIATE,      // e.g., #10, 0, -5
    MEMORY,         // e.g., [R1], [R1 + 4], [var]
    LABEL           // e.g., L1, loop, target
};

struct Operand {
    OperandType type{OperandType::NONE};
    std::string text;           // Raw string representation (e.g., "R0", "#0", "[R1+4]", "L1")
    int immediateValue{0};      // Value if IMMEDIATE
    std::string baseRegister;   // Base register if MEMORY (e.g., "R1" in "[R1+4]")
    int memoryOffset{0};        // Offset if MEMORY (e.g., 4 in "[R1+4]")

    // Constructors
    Operand() = default;

    // Factory methods
    static Operand makeRegister(const std::string& regName);
    static Operand makeImmediate(int value, bool useHashPrefix = true);
    static Operand makeMemory(const std::string& rawMem, const std::string& baseReg = "", int offset = 0);
    static Operand makeLabel(const std::string& labelName);
    static Operand makeNone();

    // Queries
    bool isRegister() const { return type == OperandType::REGISTER; }
    bool isImmediate() const { return type == OperandType::IMMEDIATE; }
    bool isMemory() const { return type == OperandType::MEMORY; }
    bool isLabel() const { return type == OperandType::LABEL; }
    bool isNone() const { return type == OperandType::NONE; }

    std::string toString() const;

    bool operator==(const Operand& other) const;
    bool operator!=(const Operand& other) const { return !(*this == other); }
};

// ---------------------------------------------------------------------------
// Opcode Enumeration
// ---------------------------------------------------------------------------
enum class Opcode {
    // Label Pseudo-instruction
    LABEL_DEF,  // e.g., "L1:"

    // Data Movement
    MOV,        // MOV Rd, Rs / MOV Rd, #imm
    LOAD,       // LOAD Rd, [Rs + offset] / LOAD Rd, [mem]
    STORE,      // STORE Rs, [Rd + offset] / STORE Rs, [mem]
    NOP,        // No operation

    // Arithmetic
    ADD,        // ADD Rd, Rs / ADD Rd, #imm
    SUB,        // SUB Rd, Rs / SUB Rd, #imm
    MUL,        // MUL Rd, Rs / MUL Rd, #imm
    DIV,        // DIV Rd, Rs / DIV Rd, #imm
    INC,        // INC Rd
    DEC,        // DEC Rd

    // Bitwise / Shift
    LSL,        // LSL Rd, #imm (Logical Shift Left)
    LSR,        // LSR Rd, #imm (Logical Shift Right)
    AND,        // AND Rd, Rs / AND Rd, #imm
    OR,         // OR Rd, Rs / OR Rd, #imm
    XOR,        // XOR Rd, Rs / XOR Rd, #imm

    // Control Flow
    CMP,        // CMP Rs1, Rs2 / CMP Rs1, #imm
    JMP,        // Unconditional branch: JMP label
    BEQ,        // Branch if equal: BEQ label
    BNE,        // Branch if not equal: BNE label
    BLT,        // Branch if less than: BLT label
    BGT,        // Branch if greater than: BGT label
    BLE,        // Branch if less or equal: BLE label
    BGE,        // Branch if greater or equal: BGE label
    RET,        // Return from subroutine

    // Unknown/unsupported
    UNKNOWN
};

std::string opcodeToString(Opcode op);
Opcode stringToOpcode(const std::string& str);
bool isBranch(Opcode op);
bool isUnconditionalBranch(Opcode op);
bool isConditionalBranch(Opcode op);

// ---------------------------------------------------------------------------
// Instruction Class
// ---------------------------------------------------------------------------
class Instruction {
public:
    Opcode opcode{Opcode::UNKNOWN};
    std::vector<Operand> operands;
    std::string label;      // Standalone label or prefix label, e.g., "L1:"
    std::string comment;    // Optional assembly comment (e.g. "; counter init")
    int lineNumber{0};      // Source line number for debugging and diagnostics

    Instruction() = default;
    Instruction(Opcode op, std::vector<Operand> ops, std::string lbl = "", std::string cmt = "", int line = 0)
        : opcode(op), operands(std::move(ops)), label(std::move(lbl)), comment(std::move(cmt)), lineNumber(line) {}

    // Convenience constructors
    static Instruction makeLabel(const std::string& lbl, int line = 0);
    static Instruction makeNop(int line = 0);
    static Instruction make(Opcode op, const Operand& op1, int line = 0);
    static Instruction make(Opcode op, const Operand& op1, const Operand& op2, int line = 0);
    static Instruction make(Opcode op, const Operand& op1, const Operand& op2, const Operand& op3, int line = 0);

    // Queries
    bool isLabel() const { return opcode == Opcode::LABEL_DEF || !label.empty(); }
    bool isStandaloneLabel() const { return opcode == Opcode::LABEL_DEF; }
    bool isNop() const { return opcode == Opcode::NOP; }
    bool isBranch() const { return compiler::isBranch(opcode); }
    bool isUnconditionalBranch() const { return compiler::isUnconditionalBranch(opcode); }
    bool isConditionalBranch() const { return compiler::isConditionalBranch(opcode); }

    // Register access analysis
    bool readsRegister(const std::string& reg) const;
    bool writesRegister(const std::string& reg) const;
    bool modifiesRegister(const std::string& reg) const;

    // Operand accessors
    bool hasOperands() const { return !operands.empty(); }
    size_t operandCount() const { return operands.size(); }
    const Operand& getOperand(size_t index) const;
    Operand& getOperand(size_t index);

    // Assembly formatting
    std::string toString() const;

    // Equality
    bool operator==(const Instruction& other) const;
    bool operator!=(const Instruction& other) const { return !(*this == other); }
};

} // namespace compiler

#endif // INSTRUCTION_H

