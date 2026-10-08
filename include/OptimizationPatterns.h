#ifndef OPTIMIZATION_PATTERNS_H
#define OPTIMIZATION_PATTERNS_H

#include "OptimizationRule.h"
#include <memory>
#include <vector>

/**
 * ============================================================================
 * ============================================================================
 * 
 * Implements standard compiler peephole optimizations:
 *  1. Redundant Load After Store (STORE M, LOAD M)
 *  2. Dead Store Elimination (STORE M, STORE M)
 *  3. Redundant / Self Move Elimination (MOV R, R; MOV R1, R2 followed by MOV R2, R1)
 *  4. Algebraic Identity Simplification (ADD 0, SUB 0, MUL 1, MUL 0, SUB R, R)
 *  5. Strength Reduction (MUL #2 -> LSL #1 / ADD R, R; ADD #1 -> INC; SUB #1 -> DEC)
 *  6. Constant Folding (MOV #C1 followed by ADD/SUB/MUL #C2)
 *  7. NOP Instruction Elimination
 */

namespace compiler {

// ---------------------------------------------------------------------------
// 1. Redundant Load After Store Elimination
// ---------------------------------------------------------------------------
class RedundantLoadAfterStoreRule : public OptimizationRule {
public:
    std::string name() const override { return "Redundant Load After Store"; }
    bool apply(const std::vector<Instruction>& instructions,
               size_t index,
               size_t& consumedCount,
               std::vector<Instruction>& replacement) override;
};

// ---------------------------------------------------------------------------
// 2. Dead Store Elimination (Consecutive Stores to Same Address)
// ---------------------------------------------------------------------------
class DeadStoreRule : public OptimizationRule {
public:
    std::string name() const override { return "Dead Store Elimination"; }
    bool apply(const std::vector<Instruction>& instructions,
               size_t index,
               size_t& consumedCount,
               std::vector<Instruction>& replacement) override;
};

// ---------------------------------------------------------------------------
// 3. Redundant Move Elimination (Self-move & Reversible moves)
// ---------------------------------------------------------------------------
class RedundantMoveRule : public OptimizationRule {
public:
    std::string name() const override { return "Redundant Move Elimination"; }
    bool apply(const std::vector<Instruction>& instructions,
               size_t index,
               size_t& consumedCount,
               std::vector<Instruction>& replacement) override;
};

// ---------------------------------------------------------------------------
// 4. Algebraic Identity Simplification
// ---------------------------------------------------------------------------
class AlgebraicIdentityRule : public OptimizationRule {
public:
    std::string name() const override { return "Algebraic Identity Simplification"; }
    bool apply(const std::vector<Instruction>& instructions,
               size_t index,
               size_t& consumedCount,
               std::vector<Instruction>& replacement) override;
};

// ---------------------------------------------------------------------------
// 5. Strength Reduction
// ---------------------------------------------------------------------------
class StrengthReductionRule : public OptimizationRule {
public:
    std::string name() const override { return "Strength Reduction"; }
    bool apply(const std::vector<Instruction>& instructions,
               size_t index,
               size_t& consumedCount,
               std::vector<Instruction>& replacement) override;
};

// ---------------------------------------------------------------------------
// 6. Constant Folding (Window of 2 instructions)
// ---------------------------------------------------------------------------
class ConstantFoldingRule : public OptimizationRule {
public:
    std::string name() const override { return "Constant Folding"; }
    bool apply(const std::vector<Instruction>& instructions,
               size_t index,
               size_t& consumedCount,
               std::vector<Instruction>& replacement) override;
};

// ---------------------------------------------------------------------------
// 7. NOP Elimination
// ---------------------------------------------------------------------------
class NopEliminationRule : public OptimizationRule {
public:
    std::string name() const override { return "NOP Elimination"; }
    bool apply(const std::vector<Instruction>& instructions,
               size_t index,
               size_t& consumedCount,
               std::vector<Instruction>& replacement) override;
};

// Factory helper to instantiate standard Person 2 rules
std::vector<std::unique_ptr<OptimizationRule>> createStandardRules();

} // namespace compiler

#endif // OPTIMIZATION_PATTERNS_H

