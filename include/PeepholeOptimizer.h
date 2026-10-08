#ifndef PEEPHOLE_OPTIMIZER_H
#define PEEPHOLE_OPTIMIZER_H

#include "Instruction.h"
#include "OptimizationRule.h"
#include "OptimizationStats.h"
#include <vector>
#include <memory>
#include <string>

/**
 * ============================================================================
 * PERSON 2 WORK: Peephole Optimizer Engine
 * ============================================================================
 * 
 * Manages the sliding-window peephole optimization loop, coordinating rules,
 * running iterative passes until fixed-point convergence, and collecting
 * optimization metrics.
 */

namespace compiler {

class PeepholeOptimizer {
public:
    PeepholeOptimizer();
    explicit PeepholeOptimizer(int maxPasses, bool verbose = false);

    // Rule management
    void addRule(std::unique_ptr<OptimizationRule> rule);
    void clearRules();
    void loadStandardRules(); // Loads standard Person 2 rules

    // Optimization execution
    std::vector<Instruction> optimize(const std::vector<Instruction>& instructions,
                                      OptimizationStats& stats);
    std::vector<Instruction> optimize(const std::vector<Instruction>& instructions);

    // Configuration getters & setters
    void setMaxPasses(int passes) { m_maxPasses = passes; }
    int getMaxPasses() const { return m_maxPasses; }

    void setVerbose(bool verbose) { m_verbose = verbose; }
    bool isVerbose() const { return m_verbose; }

    const OptimizationStats& getLastStats() const { return m_lastStats; }

private:
    std::vector<std::unique_ptr<OptimizationRule>> m_rules;
    int m_maxPasses{10};
    bool m_verbose{false};
    OptimizationStats m_lastStats;

    // Single pass execution across instruction stream
    bool runSinglePass(std::vector<Instruction>& stream, OptimizationStats& stats);
};

} // namespace compiler

#endif // PEEPHOLE_OPTIMIZER_H

