#ifndef PARSER_H
#define PARSER_H

#include "Instruction.h"
#include <string>
#include <vector>
#include <istream>
#include <ostream>

/**
 * ============================================================================
 * PERSON 1 WORK: Assembly Parser and Formatter / Emitter
 * ============================================================================
 * 
 * Provides lexing and parsing capabilities for reading assembly source code
 * into an internal vector of Instructions, and serializing Instructions back
 * to cleanly formatted assembly.
 */

namespace compiler {

class Parser {
public:
    Parser() = default;

    // High-level parsing APIs
    std::vector<Instruction> parseString(const std::string& asmSource);
    std::vector<Instruction> parseStream(std::istream& in);
    std::vector<Instruction> parseFile(const std::string& filepath);

    // Parsing a single line
    bool parseLine(const std::string& line, int lineNumber, std::vector<Instruction>& outInstructions);

    // Emitting formatted assembly
    static std::string emitAssembly(const std::vector<Instruction>& instructions);
    static void emitToFile(const std::vector<Instruction>& instructions, const std::string& filepath);

private:
    // Helper parsers
    Operand parseOperand(const std::string& token);
    void trim(std::string& s);
    std::string stripComments(const std::string& line, std::string& commentOut);
    std::vector<std::string> splitOperands(const std::string& operandPart);
};

} // namespace compiler

#endif // PARSER_H

