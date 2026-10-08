#include "Instruction.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace compiler {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static std::string toUpper(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return s;
}

// ---------------------------------------------------------------------------
// Operand Implementation
// ---------------------------------------------------------------------------
Operand Operand::makeRegister(const std::string& regName) {
    Operand op;
    op.type = OperandType::REGISTER;
    op.text = toUpper(regName);
    return op;
}

Operand Operand::makeImmediate(int value, bool useHashPrefix) {
    Operand op;
    op.type = OperandType::IMMEDIATE;
    op.immediateValue = value;
    if (useHashPrefix) {
        op.text = "#" + std::to_string(value);
    } else {
        op.text = std::to_string(value);
    }
    return op;
}

Operand Operand::makeMemory(const std::string& rawMem, const std::string& baseReg, int offset) {
    Operand op;
    op.type = OperandType::MEMORY;
    op.text = rawMem;
    op.baseRegister = toUpper(baseReg);
    op.memoryOffset = offset;
    return op;
}

Operand Operand::makeLabel(const std::string& labelName) {
    Operand op;
    op.type = OperandType::LABEL;
    op.text = labelName;
    return op;
}

Operand Operand::makeNone() {
    Operand op;
    op.type = OperandType::NONE;
    op.text = "";
    return op;
}

std::string Operand::toString() const {
    return text;
}

bool Operand::operator==(const Operand& other) const {
    if (type != other.type) return false;
    if (type == OperandType::IMMEDIATE) {
        return immediateValue == other.immediateValue;
    }
    if (type == OperandType::REGISTER) {
        return toUpper(text) == toUpper(other.text);
    }
    if (type == OperandType::MEMORY) {
        if (!baseRegister.empty() && !other.baseRegister.empty()) {
            return toUpper(baseRegister) == toUpper(other.baseRegister) && memoryOffset == other.memoryOffset;
        }
        return text == other.text;
    }
    return text == other.text;
}

// ---------------------------------------------------------------------------
// Opcode Helpers
// ---------------------------------------------------------------------------
std::string opcodeToString(Opcode op) {
    switch (op) {
        case Opcode::LABEL_DEF: return "LABEL";
        case Opcode::MOV:       return "MOV";
        case Opcode::LOAD:      return "LOAD";
        case Opcode::STORE:     return "STORE";
        case Opcode::NOP:       return "NOP";
        case Opcode::ADD:       return "ADD";
        case Opcode::SUB:       return "SUB";
        case Opcode::MUL:       return "MUL";
        case Opcode::DIV:       return "DIV";
        case Opcode::INC:       return "INC";
        case Opcode::DEC:       return "DEC";
        case Opcode::LSL:       return "LSL";
        case Opcode::LSR:       return "LSR";
        case Opcode::AND:       return "AND";
        case Opcode::OR:        return "OR";
        case Opcode::XOR:       return "XOR";
        case Opcode::CMP:       return "CMP";
        case Opcode::JMP:       return "JMP";
        case Opcode::BEQ:       return "BEQ";
        case Opcode::BNE:       return "BNE";
        case Opcode::BLT:       return "BLT";
        case Opcode::BGT:       return "BGT";
        case Opcode::BLE:       return "BLE";
        case Opcode::BGE:       return "BGE";
        case Opcode::RET:       return "RET";
        default:                return "UNKNOWN";
    }
}

Opcode stringToOpcode(const std::string& str) {
    std::string upper = toUpper(str);
    if (upper == "MOV")   return Opcode::MOV;
    if (upper == "LOAD" || upper == "LDR" || upper == "LD")  return Opcode::LOAD;
    if (upper == "STORE" || upper == "STR" || upper == "ST") return Opcode::STORE;
    if (upper == "NOP")   return Opcode::NOP;
    if (upper == "ADD")   return Opcode::ADD;
    if (upper == "SUB")   return Opcode::SUB;
    if (upper == "MUL")   return Opcode::MUL;
    if (upper == "DIV")   return Opcode::DIV;
    if (upper == "INC")   return Opcode::INC;
    if (upper == "DEC")   return Opcode::DEC;
    if (upper == "LSL")   return Opcode::LSL;
    if (upper == "LSR")   return Opcode::LSR;
    if (upper == "AND")   return Opcode::AND;
    if (upper == "OR")    return Opcode::OR;
    if (upper == "XOR")   return Opcode::XOR;
    if (upper == "CMP")   return Opcode::CMP;
    if (upper == "JMP" || upper == "B") return Opcode::JMP;
    if (upper == "BEQ" || upper == "JZ") return Opcode::BEQ;
    if (upper == "BNE" || upper == "JNZ") return Opcode::BNE;
    if (upper == "BLT")   return Opcode::BLT;
    if (upper == "BGT")   return Opcode::BGT;
    if (upper == "BLE")   return Opcode::BLE;
    if (upper == "BGE")   return Opcode::BGE;
    if (upper == "RET")   return Opcode::RET;
    return Opcode::UNKNOWN;
}

bool isBranch(Opcode op) {
    return isUnconditionalBranch(op) || isConditionalBranch(op);
}

bool isUnconditionalBranch(Opcode op) {
    return op == Opcode::JMP || op == Opcode::RET;
}

bool isConditionalBranch(Opcode op) {
    return op == Opcode::BEQ || op == Opcode::BNE ||
           op == Opcode::BLT || op == Opcode::BGT ||
           op == Opcode::BLE || op == Opcode::BGE;
}

// ---------------------------------------------------------------------------
// Instruction Implementation
// ---------------------------------------------------------------------------
Instruction Instruction::makeLabel(const std::string& lbl, int line) {
    Instruction inst;
    inst.opcode = Opcode::LABEL_DEF;
    inst.label = lbl;
    inst.lineNumber = line;
    return inst;
}

Instruction Instruction::makeNop(int line) {
    Instruction inst;
    inst.opcode = Opcode::NOP;
    inst.lineNumber = line;
    return inst;
}

Instruction Instruction::make(Opcode op, const Operand& op1, int line) {
    Instruction inst;
    inst.opcode = op;
    inst.operands.push_back(op1);
    inst.lineNumber = line;
    return inst;
}

Instruction Instruction::make(Opcode op, const Operand& op1, const Operand& op2, int line) {
    Instruction inst;
    inst.opcode = op;
    inst.operands.push_back(op1);
    inst.operands.push_back(op2);
    inst.lineNumber = line;
    return inst;
}

Instruction Instruction::make(Opcode op, const Operand& op1, const Operand& op2, const Operand& op3, int line) {
    Instruction inst;
    inst.opcode = op;
    inst.operands.push_back(op1);
    inst.operands.push_back(op2);
    inst.operands.push_back(op3);
    inst.lineNumber = line;
    return inst;
}

const Operand& Instruction::getOperand(size_t index) const {
    static const Operand emptyOp;
    if (index < operands.size()) {
        return operands[index];
    }
    return emptyOp;
}

Operand& Instruction::getOperand(size_t index) {
    static Operand emptyOp;
    if (index < operands.size()) {
        return operands[index];
    }
    emptyOp = Operand();
    return emptyOp;
}

bool Instruction::readsRegister(const std::string& reg) const {
    std::string target = toUpper(reg);
    if (opcode == Opcode::LABEL_DEF || opcode == Opcode::NOP || opcode == Opcode::UNKNOWN) {
        return false;
    }

    // CMP reads both operands
    if (opcode == Opcode::CMP) {
        for (const auto& op : operands) {
            if (op.isRegister() && toUpper(op.text) == target) return true;
            if (op.isMemory() && toUpper(op.baseRegister) == target) return true;
        }
        return false;
    }

    // STORE Rs, [Rd] reads Rs, and also reads Rd if Rd is used as base address
    if (opcode == Opcode::STORE) {
        if (!operands.empty() && operands[0].isRegister() && toUpper(operands[0].text) == target) {
            return true;
        }
        if (operands.size() > 1 && operands[1].isMemory() && toUpper(operands[1].baseRegister) == target) {
            return true;
        }
        return false;
    }

    // LOAD Rd, [Rs] reads Rs (the memory address)
    if (opcode == Opcode::LOAD) {
        if (operands.size() > 1 && operands[1].isMemory() && toUpper(operands[1].baseRegister) == target) {
            return true;
        }
        return false;
    }

    // MOV Rd, Rs reads Rs (operand 1)
    if (opcode == Opcode::MOV) {
        if (operands.size() > 1 && operands[1].isRegister() && toUpper(operands[1].text) == target) {
            return true;
        }
        return false;
    }

    // 2-operand ALU: OP Rd, Rs (e.g. ADD Rd, Rs -> Rd = Rd + Rs) reads Rd and Rs!
    // 3-operand ALU: OP Rd, Rs1, Rs2 -> reads Rs1 and Rs2
    if (opcode == Opcode::ADD || opcode == Opcode::SUB || opcode == Opcode::MUL ||
        opcode == Opcode::DIV || opcode == Opcode::AND || opcode == Opcode::OR ||
        opcode == Opcode::XOR || opcode == Opcode::LSL || opcode == Opcode::LSR) {
        if (operands.size() == 2) {
            // 2-address instruction: destination is both read and written
            if (operands[0].isRegister() && toUpper(operands[0].text) == target) return true;
            if (operands[1].isRegister() && toUpper(operands[1].text) == target) return true;
        } else if (operands.size() == 3) {
            if (operands[1].isRegister() && toUpper(operands[1].text) == target) return true;
            if (operands[2].isRegister() && toUpper(operands[2].text) == target) return true;
        }
        return false;
    }

    // INC / DEC reads and writes Rd
    if (opcode == Opcode::INC || opcode == Opcode::DEC) {
        if (!operands.empty() && operands[0].isRegister() && toUpper(operands[0].text) == target) {
            return true;
        }
        return false;
    }

    return false;
}

bool Instruction::writesRegister(const std::string& reg) const {
    std::string target = toUpper(reg);
    if (opcode == Opcode::LABEL_DEF || opcode == Opcode::NOP || opcode == Opcode::CMP ||
        isBranch() || opcode == Opcode::STORE || opcode == Opcode::UNKNOWN) {
        return false;
    }

    // Destination is first operand for MOV, LOAD, ADD, SUB, MUL, DIV, INC, DEC, LSL, LSR, AND, OR, XOR
    if (!operands.empty() && operands[0].isRegister() && toUpper(operands[0].text) == target) {
        return true;
    }
    return false;
}

bool Instruction::modifiesRegister(const std::string& reg) const {
    return writesRegister(reg);
}

std::string Instruction::toString() const {
    std::ostringstream oss;

    if (opcode == Opcode::LABEL_DEF) {
        oss << label;
        if (label.empty() || label.back() != ':') {
            oss << ":";
        }
        if (!comment.empty()) {
            oss << "  " << comment;
        }
        return oss.str();
    }

    // Normal instruction indent
    if (!label.empty()) {
        oss << label;
        if (label.back() != ':') oss << ":";
        oss << "\n";
    }

    oss << "    " << opcodeToString(opcode);

    if (!operands.empty()) {
        oss << " ";
        for (size_t i = 0; i < operands.size(); ++i) {
            oss << operands[i].toString();
            if (i + 1 < operands.size()) {
                oss << ", ";
            }
        }
    }

    if (!comment.empty()) {
        oss << "    " << comment;
    }

    return oss.str();
}

bool Instruction::operator==(const Instruction& other) const {
    if (opcode != other.opcode) return false;
    if (label != other.label) return false;
    if (operands.size() != other.operands.size()) return false;
    for (size_t i = 0; i < operands.size(); ++i) {
        if (operands[i] != other.operands[i]) return false;
    }
    return true;
}

} // namespace compiler

