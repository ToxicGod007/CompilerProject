# Target Instruction Set Peephole Optimizer (C++17)

A modular, multi-pass peephole optimizer designed for a simple target load-store instruction set.

---

## 🏗️ Architecture & Project Structure

```
CompilerProject/
├── Makefile                          # Build script (targets: all, test, test1, test2, test3, demo, clean)
├── README.md                         # Project documentation
├── include/
│   ├── Instruction.h                 # ISA opcodes, operand types, and Instruction class
│   ├── Parser.h                      # Assembly lexer, parser, and emitter
│   ├── OptimizationStats.h           # Metrics tracking and reduction statistics
│   ├── OptimizationRule.h            # Abstract base class for peephole patterns
│   ├── OptimizationPatterns.h        # Concrete local optimization pattern classes
│   ├── PeepholeOptimizer.h           # Multi-pass sliding-window driver
│   └── ControlFlowOptimizer.h        # Interface for control-flow optimizations
├── src/
│   ├── Instruction.cpp               # Instruction & Operand implementation
│   ├── Parser.cpp                    # Assembly parsing and emission logic
│   ├── OptimizationPatterns.cpp      # Transformation rules implementation
│   ├── PeepholeOptimizer.cpp         # Fixed-point optimizer loop
│   ├── ControlFlowOptimizer.cpp      # Control flow optimization implementation
│   └── main.cpp                      # CLI entrypoint & demonstration driver
└── tests/
    ├── test_person1.cpp              # Unit tests for IR, parsing, and analysis
    ├── test_person2.cpp              # Unit tests for pattern matching and optimizer
    ├── test_person3.cpp              # Unit tests for control flow rules
    └── sample.asm                    # Sample assembly program with intentional redundancies
```

---

## ⚡ Implemented Optimizations

### Local Peephole Patterns
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
6. **Constant Folding**:
   - `MOV R, #C1; ADD R, #C2` $\rightarrow$ `MOV R, #(C1 + C2)`.
   - `MOV R, #C1; SUB R, #C2` $\rightarrow$ `MOV R, #(C1 - C2)`.
   - `MOV R, #C1; MUL R, #C2` $\rightarrow$ `MOV R, #(C1 * C2)`.
7. **NOP Elimination**:
   - `NOP` $\rightarrow$ eliminated.

### Control-Flow & Dead Code Optimizations
1. **Redundant Jump Elimination**:
   - `JMP L1; L1:` $\rightarrow$ `L1:` (removes jumps immediately proceeding to their target label).
2. **Jump Chaining / Branch-to-branch**:
   - `JMP L1` where `L1: JMP L2` $\rightarrow$ `JMP L2`.
3. **Jump-over-jump Condition Inversion**:
   - `BEQ L1; JMP L2; L1:` $\rightarrow$ `BNE L2; L1:`.
4. **Unreachable Code Elimination**:
   - Removes any dead instructions following a `JMP` or `RET` that occur before the next label.

### Label Safety Boundary
- Peephole window never crosses a label target to preserve control-flow correctness.

---

## 🚀 Building & Running

### 1. Compile Everything
```bash
make all
```

*(Note for Windows users: The `Makefile` assumes a Unix-like environment with `clang++`, `mkdir`, and `rm`. You may need to use WSL, MSYS2, or MinGW to run `make`, or otherwise compile the `.cpp` files manually.)*

### Design Assumptions
* **Cost Model**: Optimizations like `MUL #2 -> LSL #1` assume that shifts and increments are faster or use fewer bytes than multiplication and addition on the target processor.
* **Side Effects**: The rules assume arithmetic instructions and redundant memory operations do not have essential side effects (e.g., setting un-modeled condition flags or interacting with memory-mapped I/O).

### 2. Run Test Suites
```bash
make test
# Or run individually:
make test1
make test2
make test3
```

### 3. Run the Demonstration
```bash
make demo
# Or run with custom input file:
./bin/peephole_opt tests/sample.asm [output.asm]
```