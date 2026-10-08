#include "ControlFlowOptimizer.h"

namespace compiler {

bool RedundantJumpRule::apply(const std::vector<Instruction>& instructions,
                              size_t index,
                              size_t& consumedCount,
                              std::vector<Instruction>& replacement) {
    if (index + 1 >= instructions.size()) return false;
    
    const auto& inst1 = instructions[index];
    const auto& inst2 = instructions[index + 1];
    
    if (inst1.opcode == Opcode::JMP && inst1.operands.size() == 1 && inst1.operands[0].isLabel()) {
        std::string target = inst1.operands[0].text;
        
        if (inst2.opcode == Opcode::LABEL_DEF && inst2.label == target) {
            consumedCount = 1;
            return true;
        }
    }
    return false;
}

bool JumpChainingRule::apply(const std::vector<Instruction>& instructions,
                             size_t index,
                             size_t& consumedCount,
                             std::vector<Instruction>& replacement) {
    const auto& inst = instructions[index];
    if (!isUnconditionalBranch(inst.opcode) && !isConditionalBranch(inst.opcode)) return false;
    
    if (inst.operands.size() != 1 || !inst.operands[0].isLabel()) return false;
    
    std::string target = inst.operands[0].text;
    
    for (size_t i = 0; i < instructions.size(); ++i) {
        if (instructions[i].opcode == Opcode::LABEL_DEF && instructions[i].label == target) {
            size_t nextIdx = i + 1;
            if (nextIdx < instructions.size()) {
                const auto& nextInst = instructions[nextIdx];
                if (nextInst.opcode == Opcode::JMP && nextInst.operands.size() == 1 && nextInst.operands[0].isLabel()) {
                    std::string newTarget = nextInst.operands[0].text;
                    if (newTarget != target) {
                        consumedCount = 1;
                        Instruction newInst = inst;
                        newInst.operands[0] = Operand::makeLabel(newTarget);
                        replacement.push_back(newInst);
                        return true;
                    }
                }
            }
            break;
        }
    }
    return false;
}

bool JumpOverJumpRule::apply(const std::vector<Instruction>& instructions,
                             size_t index,
                             size_t& consumedCount,
                             std::vector<Instruction>& replacement) {
    if (index + 2 >= instructions.size()) return false;
    
    const auto& inst1 = instructions[index];
    const auto& inst2 = instructions[index + 1];
    const auto& inst3 = instructions[index + 2];
    
    if (isConditionalBranch(inst1.opcode) && inst1.operands.size() == 1 && inst1.operands[0].isLabel()) {
        std::string l1 = inst1.operands[0].text;
        
        if (inst2.opcode == Opcode::JMP && inst2.operands.size() == 1 && inst2.operands[0].isLabel()) {
            std::string l2 = inst2.operands[0].text;
            
            if (inst3.opcode == Opcode::LABEL_DEF && inst3.label == l1) {
                Opcode inverted = Opcode::UNKNOWN;
                switch (inst1.opcode) {
                    case Opcode::BEQ: inverted = Opcode::BNE; break;
                    case Opcode::BNE: inverted = Opcode::BEQ; break;
                    case Opcode::BLT: inverted = Opcode::BGE; break;
                    case Opcode::BGE: inverted = Opcode::BLT; break;
                    case Opcode::BLE: inverted = Opcode::BGT; break;
                    case Opcode::BGT: inverted = Opcode::BLE; break;
                    default: return false;
                }
                
                consumedCount = 3;
                Instruction newInst1 = Instruction::make(inverted, Operand::makeLabel(l2), inst1.lineNumber);
                replacement.push_back(newInst1);
                replacement.push_back(inst3);
                return true;
            }
        }
    }
    return false;
}

bool UnreachableCodeRule::apply(const std::vector<Instruction>& instructions,
                                size_t index,
                                size_t& consumedCount,
                                std::vector<Instruction>& replacement) {
    const auto& inst = instructions[index];
    if (inst.opcode == Opcode::JMP || inst.opcode == Opcode::RET) {
        size_t count = 1;
        while (index + count < instructions.size()) {
            const auto& nextInst = instructions[index + count];
            if (nextInst.opcode == Opcode::LABEL_DEF) {
                break;
            }
            count++;
        }
        
        if (count > 1) {
            consumedCount = count;
            replacement.push_back(inst);
            return true;
        }
    }
    return false;
}

} // namespace compiler

