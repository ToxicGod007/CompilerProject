; Peephole Optimizer Demonstration Input
; Architecture: Simple RISC/Load-Store Target Assembly

_start:
    MOV R0, #100
    ADD R0, #50
    STORE R0, [R1 + 4]
    LOAD R0, [R1 + 4]
    MOV R2, R2
    MOV R3, R4
    MOV R4, R3
    ADD R5, #0
    SUB R6, #0
    MUL R7, #1
    DIV R8, #1
    MUL R9, #0
    SUB R10, R10
    MUL R11, #2
    ADD R12, #1
    SUB R13, #1
    STORE R14, [R1]
    STORE R15, [R1]
    STORE R14, [R2]
    LOAD R15, [R2]
    NOP
    RET

