#include "Parser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <regex>

namespace compiler {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static std::string trimCopy(std::string s) {
    auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char ch) {
        return std::isspace(ch);
    });
    auto end = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char ch) {
        return std::isspace(ch);
    }).base();
    return (start < end ? std::string(start, end) : std::string());
}

static std::string toUpperStr(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });
    return s;
}

void Parser::trim(std::string& s) {
    s = trimCopy(s);
}

std::string Parser::stripComments(const std::string& line, std::string& commentOut) {
    commentOut.clear();
    // Look for ; or // as comment markers
    // Note: avoid treating '#' as comment if it's an immediate (like #10 or #-5)
    size_t commentPos = std::string::npos;
    
    // Check for ';'
    size_t scPos = line.find(';');
    if (scPos != std::string::npos) {
        commentPos = scPos;
    }

    // Check for '//'
    size_t slashPos = line.find("//");
    if (slashPos != std::string::npos && (commentPos == std::string::npos || slashPos < commentPos)) {
        commentPos = slashPos;
    }

    // Check for '#' at start of trimmed line
    std::string trimmedLine = trimCopy(line);
    if (!trimmedLine.empty() && trimmedLine[0] == '#') {
        // If it's something like "# this is comment" (not followed immediately by a digit or '-' then digit)
        if (trimmedLine.size() > 1 && !std::isdigit(trimmedLine[1]) && trimmedLine[1] != '-') {
            commentPos = line.find('#');
        }
    }

    if (commentPos != std::string::npos) {
        commentOut = trimCopy(line.substr(commentPos));
        return line.substr(0, commentPos);
    }
    return line;
}

std::vector<std::string> Parser::splitOperands(const std::string& operandPart) {
    std::vector<std::string> result;
    std::string current;
    bool insideBracket = false;

    for (char c : operandPart) {
        if (c == '[') {
            insideBracket = true;
            current += c;
        } else if (c == ']') {
            insideBracket = false;
            current += c;
        } else if (c == ',' && !insideBracket) {
            std::string trimmed = trimCopy(current);
            if (!trimmed.empty()) {
                result.push_back(trimmed);
            }
            current.clear();
        } else {
            current += c;
        }
    }

    std::string finalTrimmed = trimCopy(current);
    if (!finalTrimmed.empty()) {
        result.push_back(finalTrimmed);
    }

    return result;
}

Operand Parser::parseOperand(const std::string& rawToken) {
    std::string token = trimCopy(rawToken);
    if (token.empty()) {
        return Operand::makeNone();
    }

    // 1. Memory operand: [R1], [R1 + 4], [var]
    if (token.front() == '[' && token.back() == ']') {
        std::string inner = trimCopy(token.substr(1, token.size() - 2));
        
        // Parse base register and offset if present: [R1 + 4] or [R1 - 4]
        size_t plusPos = inner.find('+');
        size_t minusPos = inner.find('-');
        if (plusPos != std::string::npos) {
            std::string base = trimCopy(inner.substr(0, plusPos));
            std::string offStr = trimCopy(inner.substr(plusPos + 1));
            int offVal = 0;
            try { offVal = std::stoi(offStr); } catch (...) {}
            return Operand::makeMemory(token, base, offVal);
        } else if (minusPos != std::string::npos && minusPos > 0) {
            std::string base = trimCopy(inner.substr(0, minusPos));
            std::string offStr = trimCopy(inner.substr(minusPos + 1));
            int offVal = 0;
            try { offVal = -std::stoi(offStr); } catch (...) {}
            return Operand::makeMemory(token, base, offVal);
        } else {
            // e.g. [R1] or [var]
            std::string upperInner = toUpperStr(inner);
            if (upperInner.size() >= 2 && upperInner[0] == 'R' && std::isdigit(upperInner[1])) {
                return Operand::makeMemory(token, upperInner, 0);
            }
            if (upperInner == "SP" || upperInner == "FP" || upperInner == "LR" || upperInner == "PC") {
                return Operand::makeMemory(token, upperInner, 0);
            }
            return Operand::makeMemory(token, "", 0);
        }
    }

    // 2. Immediate with '#' prefix: #10, #-5, #0
    if (token.front() == '#') {
        int val = 0;
        try {
            val = std::stoi(token.substr(1));
        } catch (...) {
            val = 0;
        }
        return Operand::makeImmediate(val, true);
    }

    // 3. Raw numeric immediate: 10, -5, 0
    try {
        size_t idx = 0;
        int val = std::stoi(token, &idx);
        if (idx == token.size()) {
            return Operand::makeImmediate(val, false);
        }
    } catch (...) {
    }

    // 4. Register operand: R0, R1, SP, FP, LR, PC
    std::string upper = toUpperStr(token);
    if ((upper.size() >= 2 && upper[0] == 'R' && std::isdigit(upper[1])) ||
        upper == "SP" || upper == "FP" || upper == "LR" || upper == "PC") {
        return Operand::makeRegister(upper);
    }

    // 5. Default: Label or symbol name
    return Operand::makeLabel(token);
}

bool Parser::parseLine(const std::string& rawLine, int lineNumber, std::vector<Instruction>& outInstructions) {
    std::string comment;
    std::string line = stripComments(rawLine, comment);
    trim(line);

    if (line.empty()) {
        return false;
    }

    // Check if line contains a label definition (e.g. "loop:" or "L1: MOV R0, #1")
    size_t colonPos = line.find(':');
    std::string labelName;
    if (colonPos != std::string::npos) {
        labelName = trimCopy(line.substr(0, colonPos));
        line = trimCopy(line.substr(colonPos + 1));

        if (line.empty()) {
            // Standalone label line
            Instruction labelInst = Instruction::makeLabel(labelName, lineNumber);
            labelInst.comment = comment;
            outInstructions.push_back(labelInst);
            return true;
        }
        // Otherwise, labelName precedes an instruction on the same line
        Instruction labelInst = Instruction::makeLabel(labelName, lineNumber);
        outInstructions.push_back(labelInst);
    }

    // Now parse the instruction mnemonic and operands
    // Extract first word (mnemonic)
    std::istringstream iss(line);
    std::string mnemonic;
    iss >> mnemonic;

    Opcode op = stringToOpcode(mnemonic);
    if (op == Opcode::UNKNOWN) {
        // Unknown opcode, ignore or handle gracefully
        return false;
    }

    // The rest of the line contains operands
    std::string restOfLine;
    std::getline(iss, restOfLine);
    trim(restOfLine);

    std::vector<Operand> operands;
    if (!restOfLine.empty()) {
        std::vector<std::string> rawOps = splitOperands(restOfLine);
        for (const auto& rop : rawOps) {
            operands.push_back(parseOperand(rop));
        }
    }

    Instruction inst(op, operands, "", comment, lineNumber);
    outInstructions.push_back(inst);
    return true;
}

std::vector<Instruction> Parser::parseString(const std::string& asmSource) {
    std::istringstream stream(asmSource);
    return parseStream(stream);
}

std::vector<Instruction> Parser::parseStream(std::istream& in) {
    std::vector<Instruction> result;
    std::string line;
    int lineNum = 1;
    while (std::getline(in, line)) {
        parseLine(line, lineNum++, result);
    }
    return result;
}

std::vector<Instruction> Parser::parseFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for reading: " + filepath);
    }
    return parseStream(file);
}

std::string Parser::emitAssembly(const std::vector<Instruction>& instructions) {
    std::ostringstream oss;
    for (const auto& inst : instructions) {
        oss << inst.toString() << "\n";
    }
    return oss.str();
}

void Parser::emitToFile(const std::vector<Instruction>& instructions, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file for writing: " + filepath);
    }
    file << emitAssembly(instructions);
}

} // namespace compiler
