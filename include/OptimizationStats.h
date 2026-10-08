#ifndef OPTIMIZATION_STATS_H
#define OPTIMIZATION_STATS_H

#include <string>
#include <map>
#include <iostream>
#include <iomanip>

/**
 * ============================================================================
 * PERSON 2 WORK: Optimization Statistics & Reporting
 * ============================================================================
 * 
 * Tracks optimization metrics across passes, including instruction counts,
 * transformation breakdown by pattern name, and overall reduction ratio.
 */

namespace compiler {

struct OptimizationStats {
    int initialInstructionCount{0};
    int finalInstructionCount{0};
    int passesExecuted{0};
    int totalTransformations{0};
    
    // Breakdown by rule category / name
    std::map<std::string, int> ruleApplications;

    void recordRule(const std::string& ruleName) {
        ruleApplications[ruleName]++;
        totalTransformations++;
    }

    void reset() {
        initialInstructionCount = 0;
        finalInstructionCount = 0;
        passesExecuted = 0;
        totalTransformations = 0;
        ruleApplications.clear();
    }

    double getReductionPercentage() const {
        if (initialInstructionCount == 0) return 0.0;
        double diff = initialInstructionCount - finalInstructionCount;
        return (diff / static_cast<double>(initialInstructionCount)) * 100.0;
    }

    void printSummary(std::ostream& out = std::cout) const {
        out << "========================================\n";
        out << "       PEEPHOLE OPTIMIZATION REPORT     \n";
        out << "========================================\n";
        out << "Passes Executed:        " << passesExecuted << "\n";
        out << "Initial Instructions:   " << initialInstructionCount << "\n";
        out << "Final Instructions:     " << finalInstructionCount << "\n";
        out << "Instructions Removed:   " << (initialInstructionCount - finalInstructionCount) << "\n";
        out << std::fixed << std::setprecision(2);
        out << "Code Size Reduction:    " << getReductionPercentage() << "%\n";
        out << "Total Transformations:  " << totalTransformations << "\n";
        out << "----------------------------------------\n";
        out << "Transformations Breakdown:\n";
        if (ruleApplications.empty()) {
            out << "  (No optimizations applied)\n";
        } else {
            for (const auto& kv : ruleApplications) {
                out << "  - " << std::left << std::setw(30) << kv.first << ": " << kv.second << "\n";
            }
        }
        out << "========================================\n";
    }
};

} // namespace compiler

#endif // OPTIMIZATION_STATS_H

