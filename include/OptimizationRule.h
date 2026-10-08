#ifndef OPTIMIZATION_RULE_H
#define OPTIMIZATION_RULE_H

#include "Instruction.h"
#include "OptimizationStats.h"
#include <string>
#include <vector>
#include <memory>

/**
 * ============================================================================
 * PERSON 2 WORK: Modular Optimization Rule Interface
 * ============================================================================
 * 
 * Abstract base class for peephole optimization rules. Each rule inspects
 * a sliding window of instructions and, if matched, supplies replacement
 * instructions.
 */

namespace compiler {

class OptimizationRule {
public:
    virtual ~OptimizationRule() = default;

    // Human-readable name of the optimization rule
    virtual std::string name() const = 0;

    // Maximum number of instructions this rule looks at
    virtual size_t windowSize() const = 0;

    /**
     * Attempts to match and apply the rule at the specified position.
     * 
     * @param instructions  Full instruction stream
     * @param index         Current starting position in instruction stream
     * @param consumedCount Out: number of instructions to be replaced
     * @param replacement   Out: new instructions to insert (can be empty if eliminated)
     * @return true if pattern matched and replacement was generated, false otherwise
     */
    virtual bool apply(const std::vector<Instruction>& instructions,
                       size_t index,
                       size_t& consumedCount,
                       std::vector<Instruction>& replacement) = 0;
};

} // namespace compiler

#endif // OPTIMIZATION_RULE_H

