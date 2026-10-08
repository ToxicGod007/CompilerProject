# Target Instruction Set Peephole Optimizer (C++17)

A modular, multi-pass peephole optimizer designed for a simple target load-store instruction set.

---

## 👥 Team Workload & Milestone Breakdown (25% Milestone)

| Team Member | Module & Responsibilities | Contribution | Status |
| :--- | :--- | :---: | :--- |
| **Person 1** | **Target Architecture IR, Lexer, Parser & Emitter**<br>• Target ISA specification (Opcodes, Operands)<br>• Instruction IR (`Instruction`, `Operand`)<br>• Assembly file & string parser (`Parser`)<br>• Assembly emitter/pretty-printer<br>• Register usage analysis (`readsRegister`, `writesRegister`)<br>• Person 1 unit tests (`tests/test_person1.cpp`) | **8.33%** | **✅ COMPLETED** |
| **Person 2** | **Core Peephole Engine & Local Pattern Transformations**<br>• Sliding-window multi-pass optimization engine (`PeepholeOptimizer`)<br>• Fixed-point convergence algorithm<br>• Optimization metrics & reporting (`OptimizationStats`)<br>• Extensible rule interface (`OptimizationRule`)<br>• 7 Optimization patterns: Redundant Load/Store, Dead Store, Redundant Moves, Algebraic Simplification, Strength Reduction, Constant Folding, NOP Elimination<br>• Label safety barrier preservation<br>• Person 2 unit tests (`tests/test_person2.cpp`) | **8.33%** | **✅ COMPLETED** |
| **Person 3** | **Control Flow & Dead Code Optimizations**<br>• Redundant jump elimination (`JMP L1; L1:`)<br>• Jump chaining / Branch-to-branch (`JMP L1` where `L1: JMP L2`)<br>• Jump-over-jump condition inversion (`BEQ L1; JMP L2; L1:` -> `BNE L2; L1:`)<br>• Unreachable code elimination after unconditional jumps/returns<br>• Unreferenced label removal<br>• CLI flags & benchmarking suite | **8.33%** | **⏳ PENDING**<br>*(See Antigravity Prompt below)* |
| **TOTAL** | **Milestone 1 Completed (When Person 3 finishes)** | **25.0%** | *(75% of project remaining)* |

---

## 🏗️ Architecture & Project Structure

```
CompilerProject/
├── Makefile                          # Build script (targets: all, test, test1, test2, demo, clean)
├── README.md                         # Project documentation and Antigravity handover prompt
├── include/
│   ├── Instruction.h                 # [Person 1] ISA opcodes, operand types, and Instruction class
│   ├── Parser.h                      # [Person 1] Assembly lexer, parser, and emitter
│   ├── OptimizationStats.h           # [Person 2] Metrics tracking and reduction statistics
│   ├── OptimizationRule.h            # [Person 2] Abstract base class for peephole patterns
│   ├── OptimizationPatterns.h        # [Person 2] Concrete local optimization pattern classes
│   ├── PeepholeOptimizer.h           # [Person 2] Multi-pass sliding-window driver
│   └── ControlFlowOptimizer.h        # [Person 3] Interface & stubs for control-flow optimizations
├── src/
│   ├── Instruction.cpp               # [Person 1] Instruction & Operand implementation
│   ├── Parser.cpp                    # [Person 1] Assembly parsing and emission logic
│   ├── OptimizationPatterns.cpp      # [Person 2] Transformation rules implementation
│   ├── PeepholeOptimizer.cpp         # [Person 2] Fixed-point optimizer loop
│   ├── ControlFlowOptimizer.cpp      # [Person 3] Stubs ready for Person 3 implementation
│   └── main.cpp                      # CLI entrypoint & demonstration driver
└── tests/
    ├── test_person1.cpp              # [Person 1] Unit tests for IR, parsing, and analysis
    ├── test_person2.cpp              # [Person 2] Unit tests for pattern matching and optimizer
    └── sample.asm                    # Sample assembly program with intentional redundancies
```

---

## ⚡ Implemented Peephole Optimizations (Person 2)

1. **Redundant Load After Store**:
   - `STORE R1, [M]; LOAD R1, [M]` $\rightarrow$ `STORE R1, [M]` (eliminated redundant memory fetch).
   - `STORE R1, [M]; LOAD R2, [M]` $\rightarrow$ `STORE R1, [M]; MOV R2, R1` (replaced slow memory read with fast register-to-register copy).
2. **Dead Store Elimination**:
   - `STORE R1, [M]; STORE R2, [M]` $\rightarrow$ `STORE R2, [M]` (first write is never read).
3. **Redundant Move Elimination**:
   - `MOV R1, R1` $\rightarrow$ eliminated (identity no-op).
   - `MOV R1, R2; MOV R2, R1` $\rightarrow$ `MOV R1, R2` (reciprocal move eliminated).
4. **Algebraic Identity Simplification**:
   - `ADD R, #0` / `SUB R, #0` / `OR R, #0` $\rightarrow$ eliminated.
   - `MUL R, #1` / `DIV R, #1` $\rightarrow$ eliminated.
   - `MUL R, #0` / `AND R, #0` $\rightarrow$ `MOV R, #0`.
   - `SUB R, R` $\rightarrow$ `MOV R, #0` (zeroing register).
5. **Strength Reduction**:
   - `MUL R, #2` $\rightarrow$ `LSL R, #1` (or `ADD R, R`).
   - `ADD R, #1` $\rightarrow$ `INC R`.
   - `SUB R, #1` $\rightarrow$ `DEC R`.
6. **Constant Folding (Window Size 2)**:
   - `MOV R, #C1; ADD R, #C2` $\rightarrow$ `MOV R, #(C1 + C2)`.
   - `MOV R, #C1; SUB R, #C2` $\rightarrow$ `MOV R, #(C1 - C2)`.
   - `MOV R, #C1; MUL R, #C2` $\rightarrow$ `MOV R, #(C1 * C2)`.
7. **NOP Elimination**:
   - `NOP` $\rightarrow$ eliminated.
8. **Label Safety Boundary**:
   - Peephole window never crosses a label target to preserve control-flow correctness.

---

## 🚀 Building & Running

### 1. Compile Everything
```bash
make all
```

### 2. Run Test Suites
```bash
make test
# Or run individually:
make test1    # Runs Person 1 tests
make test2    # Runs Person 2 tests
```

### 3. Run the Demonstration
```bash
make demo
# Or run with custom input file:
./bin/peephole_opt tests/sample.asm [output.asm]
```

---

## 🤖 Antigravity Handover Prompt for Person 3

Copy and paste the prompt below into Antigravity when Person 3 begins their work:

```markdown
You are working on the Compiler Peephole Optimizer project in C++17 located in `/Users/aritrabiswas/Desktop/CompilerProject`.

### Current Project Status:
- Person 1 and Person 2 work is 100% complete and verified with passing unit tests (`make test`).
- Person 1 implemented:
  - `include/Instruction.h`, `src/Instruction.cpp` (Opcodes, Operands, Instruction IR, register read/write analysis).
  - `include/Parser.h`, `src/Parser.cpp` (Assembly parser and emitter).
  - `tests/test_person1.cpp` (Unit tests).
- Person 2 implemented:
  - `include/OptimizationStats.h` (Statistics and reduction metrics).
  - `include/OptimizationRule.h` (Abstract rule interface).
  - `include/OptimizationPatterns.h`, `src/OptimizationPatterns.cpp` (7 core local optimizations: redundant load/store, dead store, redundant moves, algebraic identity, strength reduction, constant folding, NOP elimination).
  - `include/PeepholeOptimizer.h`, `src/PeepholeOptimizer.cpp` (Sliding window fixed-point engine).
- Together, all 3 team members are completing the 25% project milestone (~8.33% per person).
- Person 1 and Person 2 have completed their parts (16.67% total).
- Your work will finish the final 8.33% of this milestone, bringing overall project completion to exactly 25.0% (with 75.0% remaining for future milestones).

### Your Task (Person 3 Work):
Implement the Control-Flow and Dead Code Optimizations for the peephole optimizer:
1. Implement the rules declared in `include/ControlFlowOptimizer.h` inside `src/ControlFlowOptimizer.cpp`:
   - `RedundantJumpRule`: Remove unconditional jumps that jump directly to the next instruction (`JMP L1` followed immediately by `L1:`).
   - `JumpChainingRule`: If an unconditional jump targets a label that is itself an unconditional jump (`JMP L1` where `L1: JMP L2`), rewrite the original jump directly to `JMP L2`.
   - `JumpOverJumpRule`: Invert conditional branches jumping over an unconditional jump:
     `BEQ L1; JMP L2; L1:` -> `BNE L2; L1:` (or `BLT` <-> `BGE`, `BLE` <-> `BGT`).
   - `UnreachableCodeRule`: Eliminate dead code immediately following unconditional jumps (`JMP`) or returns (`RET`) up until the next reachable label definition.
2. Register these rules into `PeepholeOptimizer::loadStandardRules()` or provide a control-flow optimization pass method.
3. Write a comprehensive unit test suite in `tests/test_person3.cpp` testing all Person 3 optimizations.
4. Update `Makefile` to include `tests/test_person3.cpp` under `test` and `test3`.
5. Enhance `src/main.cpp` with command-line flags (e.g., `--passes <N>`, `--stats`, `--no-cf` to toggle control-flow optimizations).
6. Verify everything compiles cleanly with `-Wall -Wextra` and all unit tests pass (`make test`).
```