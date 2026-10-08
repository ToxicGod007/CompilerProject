#include "OptimizationPatterns.h"

namespace compiler {

// ---------------------------------------------------------------------------
// 1. Redundant Load After Store
// ---------------------------------------------------------------------------
bool RedundantLoadAfterStoreRule::apply(const std::vector<Instruction>& instructions,
                                        size_t index,
                                        size_t& consumedCount,
                                        std::vector<Instruction>& replacement) {
    if (index + 1 >= instructions.size()) return false;

    const auto& i1 = instructions[index];
    const auto& i2 = instructions[index + 1];

    // Cannot optimize across a label boundary (control could branch to i2)
    if (i2.isLabel() || !i2.label.empty()) return false;

    // Pattern:
    // i1: STORE Rs, [Mem]
    // i2: LOAD  Rd, [Mem]
    if (i1.opcode != Opcode::STORE || i2.opcode != Opcode::LOAD) return false;
    if (i1.operands.size() < 2 || i2.operands.size() < 2) return false;

    const Operand& storeSrc = i1.operands[0];
    const Operand& storeMem = i1.operands[1];
    const Operand& loadDst  = i2.operands[0];
    const Operand& loadMem  = i2.operands[1];

    // Check if memory locations match
    if (storeMem != loadMem) return false;

    consumedCount = 2;
    replacement.clear();
    replacement.push_back(i1); // Keep the STORE

    if (storeSrc == loadDst) {
        // Pattern 1A: LOAD into same register that was just stored
        // STORE R1, [M]
        // LOAD  R1, [M]  -> Redundant, R1 already holds the value
        // Replacement is just i1.
        return true;
    } else {
        // Pattern 1B: LOAD into different register
        // STORE R1, [M]
        // LOAD  R2, [M]  -> Replace LOAD with register copy: MOV R2, R1
        Instruction movInst = Instruction::make(Opcode::MOV, loadDst, storeSrc, i2.lineNumber);
        movInst.comment = "; replaced memory load with register copy";
        replacement.push_back(movInst);
        return true;
    }
}

// ---------------------------------------------------------------------------
// 2. Dead Store Elimination
// ---------------------------------------------------------------------------
bool DeadStoreRule::apply(const std::vector<Instruction>& instructions,
                          size_t index,
                          size_t& consumedCount,
                          std::vector<Instruction>& replacement) {
    if (index + 1 >= instructions.size()) return false;

    const auto& i1 = instructions[index];
    const auto& i2 = instructions[index + 1];

    if (i2.isLabel() || !i2.label.empty()) return false;

    // Pattern:
    // STORE Rs1, [Mem]
    // STORE Rs2, [Mem]
    // First store is overwritten immediately without intervening read.
    if (i1.opcode != Opcode::STORE || i2.opcode != Opcode::STORE) return false;
    if (i1.operands.size() < 2 || i2.operands.size() < 2) return false;

    if (i1.operands[1] != i2.operands[1]) return false;

    // Keep only the second store
    consumedCount = 2;
    replacement.clear();
    replacement.push_back(i2);
    return true;
}

// ---------------------------------------------------------------------------
// 3. Redundant Move Elimination
// ---------------------------------------------------------------------------
bool RedundantMoveRule::apply(const std::vector<Instruction>& instructions,
                              size_t index,
                              size_t& consumedCount,
                              std::vector<Instruction>& replacement) {
    const auto& i1 = instructions[index];

    // Pattern 3A: Self Move (MOV R, R)
    if (i1.opcode == Opcode::MOV && i1.operands.size() >= 2) {
        if (i1.operands[0].isRegister() && i1.operands[1].isRegister() &&
            i1.operands[0] == i1.operands[1]) {
            consumedCount = 1;
            replacement.clear();
            // If i1 had a label attached, preserve label
            if (!i1.label.empty()) {
                replacement.push_back(Instruction::makeLabel(i1.label, i1.lineNumber));
            }
            return true;
        }
    }

    // Pattern 3B: Reversible Move (MOV R1, R2 followed immediately by MOV R2, R1)
    if (index + 1 < instructions.size()) {
        const auto& i2 = instructions[index + 1];
        if (!i2.isLabel() && i2.label.empty()) {
            if (i1.opcode == Opcode::MOV && i2.opcode == Opcode::MOV &&
                i1.operands.size() >= 2 && i2.operands.size() >= 2) {
                const Operand& r1_dst = i1.operands[0];
                const Operand& r1_src = i1.operands[1];
                const Operand& r2_dst = i2.operands[0];
                const Operand& r2_src = i2.operands[1];

                if (r1_dst.isRegister() && r1_src.isRegister() &&
                    r2_dst.isRegister() && r2_src.isRegister() &&
                    r1_dst == r2_src && r1_src == r2_dst) {
                    // Second MOV is redundant
                    consumedCount = 2;
                    replacement.clear();
                    replacement.push_back(i1);
                    return true;
                }
            }
        }
    }

    return false;
}

// ---------------------------------------------------------------------------
// 4. Algebraic Identity Simplification
// ---------------------------------------------------------------------------
bool AlgebraicIdentityRule::apply(const std::vector<Instruction>& instructions,
                                  size_t index,
                                  size_t& consumedCount,
                                  std::vector<Instruction>& replacement) {
    const auto& inst = instructions[index];

    if (inst.operands.size() < 2) return false;

    const Operand& dst = inst.operands[0];
    const Operand& src = inst.operands[1];

    // Identity additions and subtractions: ADD R, #0 / SUB R, #0 / OR R, #0
    if (src.isImmediate() && src.immediateValue == 0) {
        if (inst.opcode == Opcode::ADD || inst.opcode == Opcode::SUB || inst.opcode == Opcode::OR) {
            consumedCount = 1;
            replacement.clear();
            if (!inst.label.empty()) {
                replacement.push_back(Instruction::makeLabel(inst.label, inst.lineNumber));
            }
            return true;
        }

        // Multiplying or ANDing by 0 zeroes the register: MUL R, #0 -> MOV R, #0
        if (inst.opcode == Opcode::MUL || inst.opcode == Opcode::AND) {
            consumedCount = 1;
            replacement.clear();
            Instruction movZero = Instruction::make(Opcode::MOV, dst, Operand::makeImmediate(0), inst.lineNumber);
            movZero.label = inst.label;
            movZero.comment = "; simplified from " + opcodeToString(inst.opcode) + " #0";
            replacement.push_back(movZero);
            return true;
        }
    }

    // Identity multiplication and division: MUL R, #1 / DIV R, #1
    if (src.isImmediate() && src.immediateValue == 1) {
        if (inst.opcode == Opcode::MUL || inst.opcode == Opcode::DIV) {
            consumedCount = 1;
            replacement.clear();
            if (!inst.label.empty()) {
                replacement.push_back(Instruction::makeLabel(inst.label, inst.lineNumber));
            }
            return true;
        }
    }

    // Self-subtraction: SUB R, R -> MOV R, #0
    if (inst.opcode == Opcode::SUB && dst.isRegister() && src.isRegister() && dst == src) {
        consumedCount = 1;
        replacement.clear();
        Instruction movZero = Instruction::make(Opcode::MOV, dst, Operand::makeImmediate(0), inst.lineNumber);
        movZero.label = inst.label;
        movZero.comment = "; self-subtraction clears register";
        replacement.push_back(movZero);
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// 5. Strength Reduction
// ---------------------------------------------------------------------------
bool StrengthReductionRule::apply(const std::vector<Instruction>& instructions,
                                  size_t index,
                                  size_t& consumedCount,
                                  std::vector<Instruction>& replacement) {
    const auto& inst = instructions[index];
    if (inst.operands.size() < 2) return false;

    const Operand& dst = inst.operands[0];
    const Operand& src = inst.operands[1];

    // MUL R, #2 -> LSL R, #1 (or ADD R, R)
    if (inst.opcode == Opcode::MUL && src.isImmediate() && src.immediateValue == 2) {
        consumedCount = 1;
        replacement.clear();
        Instruction lsl = Instruction::make(Opcode::LSL, dst, Operand::makeImmediate(1), inst.lineNumber);
        lsl.label = inst.label;
        lsl.comment = "; strength reduction: MUL #2 to LSL #1";
        replacement.push_back(lsl);
        return true;
    }

    // ADD R, #1 -> INC R
    if (inst.opcode == Opcode::ADD && src.isImmediate() && src.immediateValue == 1) {
        consumedCount = 1;
        replacement.clear();
        Instruction inc = Instruction::make(Opcode::INC, dst, inst.lineNumber);
        inc.label = inst.label;
        replacement.push_back(inc);
        return true;
    }

    // SUB R, #1 -> DEC R
    if (inst.opcode == Opcode::SUB && src.isImmediate() && src.immediateValue == 1) {
        consumedCount = 1;
        replacement.clear();
        Instruction dec = Instruction::make(Opcode::DEC, dst, inst.lineNumber);
        dec.label = inst.label;
        replacement.push_back(dec);
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// 6. Constant Folding
// ---------------------------------------------------------------------------
bool ConstantFoldingRule::apply(const std::vector<Instruction>& instructions,
                                size_t index,
                                size_t& consumedCount,
                                std::vector<Instruction>& replacement) {
    if (index + 1 >= instructions.size()) return false;

    const auto& i1 = instructions[index];
    const auto& i2 = instructions[index + 1];

    if (i2.isLabel() || !i2.label.empty()) return false;

    // Pattern:
    // i1: MOV Rd, #C1
    // i2: OP  Rd, #C2  (where OP is ADD, SUB, or MUL)
    if (i1.opcode != Opcode::MOV || i1.operands.size() < 2) return false;
    if (!i1.operands[0].isRegister() || !i1.operands[1].isImmediate()) return false;

    if (i2.operands.size() < 2) return false;
    if (!i2.operands[0].isRegister() || !i2.operands[1].isImmediate()) return false;

    // Both instructions must target the same register
    if (i1.operands[0] != i2.operands[0]) return false;

    const Operand& reg = i1.operands[0];
    int c1 = i1.operands[1].immediateValue;
    int c2 = i2.operands[1].immediateValue;
    int foldedResult = 0;

    if (i2.opcode == Opcode::ADD) {
        foldedResult = c1 + c2;
    } else if (i2.opcode == Opcode::SUB) {
        foldedResult = c1 - c2;
    } else if (i2.opcode == Opcode::MUL) {
        foldedResult = c1 * c2;
    } else {
        return false;
    }

    consumedCount = 2;
    replacement.clear();
    Instruction folded = Instruction::make(Opcode::MOV, reg, Operand::makeImmediate(foldedResult), i1.lineNumber);
    folded.label = i1.label;
    folded.comment = "; constant folded (" + std::to_string(c1) + " " +
                     opcodeToString(i2.opcode) + " " + std::to_string(c2) + ")";
    replacement.push_back(folded);
    return true;
}

// ---------------------------------------------------------------------------
// 7. NOP Elimination
// ---------------------------------------------------------------------------
bool NopEliminationRule::apply(const std::vector<Instruction>& instructions,
                              size_t index,
                              size_t& consumedCount,
                              std::vector<Instruction>& replacement) {
    const auto& inst = instructions[index];
    if (inst.opcode == Opcode::NOP) {
        consumedCount = 1;
        replacement.clear();
        if (!inst.label.empty()) {
            replacement.push_back(Instruction::makeLabel(inst.label, inst.lineNumber));
        }
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------
std::vector<std::unique_ptr<OptimizationRule>> createPerson2StandardRules() {
    std::vector<std::unique_ptr<OptimizationRule>> rules;
    // Register in preferred priority order
    rules.push_back(std::make_unique<RedundantLoadAfterStoreRule>());
    rules.push_back(std::make_unique<DeadStoreRule>());
    rules.push_back(std::make_unique<ConstantFoldingRule>());
    rules.push_back(std::make_unique<RedundantMoveRule>());
    rules.push_back(std::make_unique<AlgebraicIdentityRule>());
    rules.push_back(std::make_unique<StrengthReductionRule>());
    rules.push_back(std::make_unique<NopEliminationRule>());
    return rules;
}

} // namespace compiler

