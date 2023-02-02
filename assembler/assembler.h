#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <fstream>
#include <map>
#include <sstream>
#include <string>

class Assembler {
    public:
        Assembler(const std::string & input);
        ~Assembler() {}

        Assembler(const Assembler &) = delete;
        Assembler & operator=(const Assembler &) = delete;
        Assembler(const Assembler &&) = delete;
        Assembler & operator=(const Assembler &&) = delete;

        void parseCode();
        void writeOutput();

    private:
        const std::string initialCommandChars = "@AMD";
        const char openingLabelChar = '(';
        const char closingLabelChar = ')';
        const char jmpSeparator = ';';
        const std::string commentPrefix = "//";
        const std::string compPrefix = "111";
        const std::string loadPrefix = "0";

        const std::map<std::string, std::string> opCodes = {
            {"0", "0101010"}, {"1", "0111111"}, {"-1", "0111010"}, {"D", "0001100"},
            {"A", "0110000"}, {"!D", "0001101"}, {"!A", "0110001"}, {"-D", "0001111"},
            {"-A", "0110011"}, {"D+1", "0011111"}, {"A+1", "0110111"}, {"D-1", "0001110"},
            {"A-1", "0110010"}, {"D+A", "0000010"}, {"D-A", "0010011"}, {"A-D", "0000111"},
            {"D&A", "0000000"}, {"D|A", "0010101"}, {"M", "1110000"}, {"!M", "1110001"},
            {"-M", "1110011"}, {"M+1", "1110111"}, {"M-1", "1110010"}, {"D+M", "1000010"},
            {"D-M", "1010011"}, {"M-D", "1000111"}, {"D&M", "1000000"}, {"D|M", "1010101"}
        };

        const std::map<std::string, std::string> jumpCodes = {
            {"JGT", "001"}, {"JEQ", "010"}, {"JGE", "011"}, {"JLT", "100"},
            {"JNE", "101"}, {"JLE", "110"}, {"JMP", "111"}
        };

        std::string basename;
        std::string outName;
        std::ifstream inFile;
        std::stringstream instructionStream;
        int binaryLineCount = -1;
        int freeMemoryIndex = 16;
        std::map<std::string, int> symbolTable = {
            {"SP", 0}, {"LCL", 1}, {"ARG", 2}, {"THIS", 3}, {"THAT", 4},
            {"R0", 0}, {"R1", 1}, {"R2", 2}, {"R3", 3},
            {"R4", 4}, {"R5", 5}, {"R6", 6}, {"R7", 7},
            {"R8", 8}, {"R9", 9}, {"R10", 10}, {"R11", 11},
            {"R12", 12}, {"R13", 13}, {"R14", 14}, {"R15", 15},
            {"SCREEN", 16384}, {"KBD", 24576}
        };

        // methods
        void assignLabelCodes();
        void translateCommands();
        std::string binaryRep(const int val) const;
        std::string binaryCompCode(const std::string & command) const;
        std::string binaryDestCode(const std::string & command) const;
        std::string binaryJumpCode(const std::string & command) const;
};

#endif /* ASSEMBLER_H */
