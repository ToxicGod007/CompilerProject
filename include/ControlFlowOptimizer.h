#ifndef CONTROL_FLOW_OPTIMIZER_H
#define CONTROL_FLOW_OPTIMIZER_H

#include "OptimizationRule.h"
#include <string>
#include <vector>

/**
 * ============================================================================
 * PERSON 3 WORK: Control Flow & Unreachable Code Optimizations (TO BE IMPLEMENTED)
 * ============================================================================
 * 
 * This file outlines the specifications for Person 3's contribution to the
 * Peephole Optimizer project.
 * 
 * Planned Responsibilities for Person 3:
 *  1. Redundant Jump Elimination (JMP L1 followed immediately by L1:)
 *  2. Branch Chaining / Jump-to-Jump (JMP L1 where L1 is JMP L2 -> JMP L2)
 *  3. Inverted Conditional Branch over Jump (BEQ L1; JMP L2; L1: -> BNE L2; L1:)
 *  4. Unreachable Dead Code Elimination (instructions following JMP/RET before next label)
 *  5. Unused Label Elimination
 *  6. CLI Runner and Benchmarking Suite
 */

namespace compiler {

// ---------------------------------------------------------------------------
// 1. Redundant Jump Elimination (JMP L1 followed immediately by label L1:)
// ---------------------------------------------------------------------------
class RedundantJumpRule : public OptimizationRule {
public:
    std::string name() const override { return "Redundant Jump Elimination"; }
    size_t windowSize() const override { return 2; }

    // TODO: Person 3 to implement in src/ControlFlowOptimizer.cpp
    bool apply(const std::vector<Instruction>& instructions,
               size_t index,
               size_t& consumedCount,
               std::vector<Instruction>& replacement) override;
};

// ---------------------------------------------------------------------------
// 2. Jump Chaining / Jump-to-Jump
// ---------------------------------------------------------------------------
class JumpChainingRule : public OptimizationRule {
public:
    std::string name() const override { return "Branch Chaining"; }
    size_t windowSize() const override { return 3; }

    // TODO: Person 3 to implement in src/ControlFlowOptimizer.cpp
    bool apply(const std::vector<Instruction>& instructions,
               size_t index,
               size_t& consumedCount,
               std::vector<Instruction>& replacement) override;
};

// ---------------------------------------------------------------------------
// 3. Jump Over Jump (Condition Inversion)
// ---------------------------------------------------------------------------
class JumpOverJumpRule : public OptimizationRule {
public:
    std::string name() const override { return "Jump Over Jump Inversion"; }
    size_t windowSize() const override { return 3; }

    // TODO: Person 3 to implement in src/ControlFlowOptimizer.cpp
    bool apply(const std::vector<Instruction>& instructions,
               size_t index,
               size_t& consumedCount,
               std::vector<Instruction>& replacement) override;
};

// ---------------------------------------------------------------------------
// 4. Unreachable Code Elimination
// ---------------------------------------------------------------------------
class UnreachableCodeRule : public OptimizationRule {
public:
    std::string name() const override { return "Unreachable Code Elimination"; }
    size_t windowSize() const override { return 2; }

    // TODO: Person 3 to implement in src/ControlFlowOptimizer.cpp
    bool apply(const std::vector<Instruction>& instructions,
               size_t index,
               size_t& consumedCount,
               std::vector<Instruction>& replacement) override;
};

} // namespace compiler

#endif // CONTROL_FLOW_OPTIMIZER_H

