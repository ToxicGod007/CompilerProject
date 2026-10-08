#include "PeepholeOptimizer.h"
#include "OptimizationPatterns.h"
#include "ControlFlowOptimizer.h"
#include <iostream>

namespace compiler {

PeepholeOptimizer::PeepholeOptimizer()
    : m_maxPasses(10), m_verbose(false) {
    loadStandardRules();
}

PeepholeOptimizer::PeepholeOptimizer(int maxPasses, bool verbose)
    : m_maxPasses(maxPasses), m_verbose(verbose) {
    loadStandardRules();
}

void PeepholeOptimizer::addRule(std::unique_ptr<OptimizationRule> rule) {
    m_rules.push_back(std::move(rule));
}

void PeepholeOptimizer::clearRules() {
    m_rules.clear();
}

void PeepholeOptimizer::loadStandardRules() {
    m_rules = createPerson2StandardRules();
    m_rules.push_back(std::make_unique<RedundantJumpRule>());
    m_rules.push_back(std::make_unique<JumpChainingRule>());
    m_rules.push_back(std::make_unique<JumpOverJumpRule>());
    m_rules.push_back(std::make_unique<UnreachableCodeRule>());
}

bool PeepholeOptimizer::runSinglePass(std::vector<Instruction>& stream, OptimizationStats& stats) {
    bool modified = false;
    size_t i = 0;

    while (i < stream.size()) {
        bool ruleMatched = false;

        for (const auto& rule : m_rules) {
            size_t consumed = 0;
            std::vector<Instruction> replacement;

            if (rule->apply(stream, i, consumed, replacement)) {
                if (m_verbose) {
                    std::cout << "[Pass " << (stats.passesExecuted + 1) << "] Rule Applied: '"
                              << rule->name() << "' at instruction " << i
                              << " (consumed " << consumed << ", inserted " << replacement.size() << ")\n";
                }

                stats.recordRule(rule->name());

                // Replace consumed instructions with replacement
                stream.erase(stream.begin() + i, stream.begin() + i + consumed);
                stream.insert(stream.begin() + i, replacement.begin(), replacement.end());

                modified = true;
                ruleMatched = true;

                // Advance past replaced elements to prevent cyclical infinite loops within one pass.
                // Cascading optimizations across replaced code will be caught on the next pass.
                if (replacement.empty()) {
                    // Current instruction was deleted; next instruction shifted to index i, don't increment i
                } else {
                    i += replacement.size();
                }
                break;
            }
        }

        if (!ruleMatched) {
            ++i;
        }
    }

    return modified;
}

std::vector<Instruction> PeepholeOptimizer::optimize(const std::vector<Instruction>& instructions,
                                                     OptimizationStats& stats) {
    stats.reset();
    stats.initialInstructionCount = static_cast<int>(instructions.size());

    std::vector<Instruction> current = instructions;

    for (int pass = 0; pass < m_maxPasses; ++pass) {
        if (m_verbose) {
            std::cout << "--- Starting Peephole Pass " << (pass + 1) << " ---\n";
        }

        bool passModified = runSinglePass(current, stats);
        stats.passesExecuted++;

        if (!passModified) {
            if (m_verbose) {
                std::cout << "Fixed-point reached after " << stats.passesExecuted << " pass(es).\n";
            }
            break;
        }
    }

    stats.finalInstructionCount = static_cast<int>(current.size());
    m_lastStats = stats;
    return current;
}

std::vector<Instruction> PeepholeOptimizer::optimize(const std::vector<Instruction>& instructions) {
    OptimizationStats stats;
    return optimize(instructions, stats);
}

} // namespace compiler

