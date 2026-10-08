#include "Instruction.h"
#include "Parser.h"
#include "PeepholeOptimizer.h"
#include <iostream>
#include <fstream>
#include <string>

using namespace compiler;

void printBanner() {
    std::cout << "=========================================================\n";
    std::cout << "          TARGET ISA PEEPHOLE OPTIMIZER (C++17)          \n";
    std::cout << "  Milestone: 25% Completion (Person 1 & Person 2 done)   \n";
    std::cout << "=========================================================\n\n";
}

int runDemo() {
    std::string sampleAsm = 
        "; Sample Target Assembly Program demonstrating Redundant Instruction Sequences\n"
        "main:\n"
        "    MOV R0, #10           ; Load 10\n"
        "    ADD R0, #5            ; Add 5 (Eligible for Constant Folding)\n"
        "    STORE R0, [R1 + 4]    ; Store result into memory\n"
        "    LOAD  R0, [R1 + 4]    ; Redundant load after store (Eligible for Elimination)\n"
        "    MOV R2, R2            ; Self-assignment move (Eligible for Elimination)\n"
        "    MOV R3, R4            ; Register copy\n"
        "    MOV R4, R3            ; Redundant reciprocal move (Eligible for Elimination)\n"
        "    ADD R5, #0            ; Algebraic identity +0 (Eligible for Elimination)\n"
        "    MUL R6, #1            ; Algebraic identity *1 (Eligible for Elimination)\n"
        "    MUL R7, #2            ; Strength reduction: MUL #2 -> LSL #1\n"
        "    ADD R8, #1            ; Strength reduction: ADD #1 -> INC\n"
        "    SUB R9, #1            ; Strength reduction: SUB #1 -> DEC\n"
        "    STORE R10, [R2]       ; Store value to [R2]\n"
        "    STORE R11, [R2]       ; Dead store: immediate overwrite (Eligible for Elimination)\n"
        "    STORE R12, [R3]       ; Store to [R3]\n"
        "    LOAD  R13, [R3]       ; Load from [R3] -> Optimize to MOV R13, R12\n"
        "    SUB R14, R14          ; Self-subtraction -> Optimize to MOV R14, #0\n"
        "    NOP                   ; No-operation (Eligible for Elimination)\n"
        "    RET\n";

    std::cout << ">>> 1. ORIGINAL INPUT ASSEMBLY (Parsed by Person 1):\n";
    std::cout << "---------------------------------------------------------\n";
    std::cout << sampleAsm << "\n";

    Parser parser;
    std::vector<Instruction> instructions = parser.parseString(sampleAsm);
    std::cout << "Successfully parsed " << instructions.size() << " instructions.\n\n";

    std::cout << ">>> 2. RUNNING PEEPHOLE OPTIMIZATION ENGINE (Person 2):\n";
    std::cout << "---------------------------------------------------------\n";
    PeepholeOptimizer optimizer(10, /*verbose=*/true);
    OptimizationStats stats;
    std::vector<Instruction> optimized = optimizer.optimize(instructions, stats);

    std::cout << "\n>>> 3. OPTIMIZATION METRICS:\n";
    stats.printSummary(std::cout);

    std::cout << "\n>>> 4. OPTIMIZED ASSEMBLY OUTPUT:\n";
    std::cout << "---------------------------------------------------------\n";
    std::cout << Parser::emitAssembly(optimized) << "\n";

    return 0;
}

int main(int argc, char* argv[]) {
    printBanner();

    if (argc < 2 || std::string(argv[1]) == "--demo") {
        std::cout << "Running built-in demonstration test case...\n";
        std::cout << "(Usage: " << argv[0] << " <input.asm> [output.asm])\n\n";
        return runDemo();
    }

    std::string inputFile = argv[1];
    std::string outputFile = (argc >= 3) ? argv[2] : "";

    try {
        Parser parser;
        std::cout << "Reading input from: " << inputFile << " ...\n";
        std::vector<Instruction> instructions = parser.parseFile(inputFile);
        std::cout << "Parsed " << instructions.size() << " instructions.\n\n";

        PeepholeOptimizer optimizer(10, /*verbose=*/true);
        OptimizationStats stats;
        std::vector<Instruction> optimized = optimizer.optimize(instructions, stats);

        stats.printSummary(std::cout);

        if (!outputFile.empty()) {
            std::cout << "Writing optimized assembly to: " << outputFile << " ...\n";
            Parser::emitToFile(optimized, outputFile);
            std::cout << "Done.\n";
        } else {
            std::cout << "\n>>> OPTIMIZED ASSEMBLY OUTPUT:\n";
            std::cout << "---------------------------------------------------------\n";
            std::cout << Parser::emitAssembly(optimized) << "\n";
        }
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}

