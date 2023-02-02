#include "assembler.h"

#include <iostream>
#include <algorithm>
#include <cctype>

// NOTE: Add debug output for help in checking!

/* ---------------------------------------------------------------------------------------------- */

Assembler::Assembler(const std::string & input) :
    inFile(input)
{
    if (!inFile.is_open()) {
        std::cerr << "Could not open file " << input << '\n';
        exit(EXIT_FAILURE);
    }

    size_t dotPos = input.find('.');
    basename = input.substr(0, dotPos);

    // !! BEGIN DEBUG !!
    std::cout << "Basename is: " << basename << '\n';
    // !! END DEBUG !!

    outName = basename + ".hack";
}

/* ---------------------------------------------------------------------------------------------- */

void Assembler::parseCode() {

    // only pseudo commands on first pass
    assignLabelCodes();

    // !! BEGIN DEBUG !!
    std::cout << "Finished assigning label codes\n";
    // !! END DEBUG !!

    // second pass interprets commands
    inFile.clear();
    inFile.seekg(0);
    translateCommands();

    // !! BEGIN DEBUG !!
    std::cout << "Finished translating commands\n";
    // !! END DEBUG !!
}

/* ---------------------------------------------------------------------------------------------- */

void Assembler::writeOutput() {

    std::ofstream outFile(outName);

    if (!outFile.is_open()) {
        std::cerr << "Could not open output file " << outName << '\n';
        exit(EXIT_FAILURE);
    }

    outFile << instructionStream.str();
}

/* ---------------------------------------------------------------------------------------------- */

void Assembler::assignLabelCodes() {

    // !! BEGIN DEBUG !!
    std::cout << "In assignLabelCodes()...\n";
    // !! END DEBUG !!

    std::string line;
    while (std::getline(inFile, line)) {

        // !! BEGIN DEBUG !!
        std::cout << "Read line: " << line << '\n';
        // !! END DEBUG !!

        // remove spaces
        line.erase(std::remove_if(line.begin(),
                                line.end(),
                                [](unsigned char c) {
                                    return std::isspace(c);
                                }),
                    line.end());

        // !! BEGIN DEBUG !!
        std::cout << "Line after stripping spaces: " << line << '\n';
        // !! END DEBUG !!

        if (line.empty()) {

            // blank line
            continue;

        } else if (line[0] == '/') {

            // comment line
            continue;

        } else if (line[0] == openingLabelChar) {

            // instruction label pseudocommand
            std::string temp = line.substr(1);

            auto endLabel = temp.find(closingLabelChar);

            std::string labelText = temp.substr(0, endLabel);

        // !! BEGIN DEBUG !!
        std::cout << "Found label: " << labelText << '\n';
        // !! END DEBUG !!

            symbolTable.insert({labelText, binaryLineCount + 1});

        } else {

            // regular command
            ++binaryLineCount;

        }
    }
}

/* ---------------------------------------------------------------------------------------------- */

void Assembler::translateCommands() {

    // !! BEGIN DEBUG !!
    std::cout << "In translateCommands()...\n";
    // !! END DEBUG !!

    std::string line;
    while (std::getline(inFile, line)) {

    // !! BEGIN DEBUG !!
    std::cout << "Read line: " << line << '\n';
    // !! END DEBUG !!

        // remove spaces
        line.erase(std::remove_if(line.begin(),
                                line.end(),
                                [](unsigned char c) {
                                    return std::isspace(c);
                                }),
                    line.end());

    // !! BEGIN DEBUG !!
    std::cout << "Line after stripping spaces: " << line << '\n';
    // !! END DEBUG !!

        // skip blank lines and comments and ignore labels
        if (line.empty())
            continue;

        else if (line[0] == '/')
            continue;

        else if (line[0] == openingLabelChar)
            continue;

        // strip end of line comments

        size_t firstCommentPos = line.find(commentPrefix);
        line = line.substr(0, firstCommentPos);

    // !! BEGIN DEBUG !!
    std::cout << "Line after stripping trailing comments: " << line << '\n';
    // !! END DEBUG !!

        // interpret command

        size_t iNonBlank = line.find_first_of(initialCommandChars);

    // !! BEGIN DEBUG !!
    std::cout << "iNonBlank is: " << iNonBlank << '\n';
    // !! END DEBUG !!

        std::string currInstruction;

        if (line[iNonBlank] == '@') {

    // !! BEGIN DEBUG !!
    std::cout << "Register load command found\n";
    // !! END DEBUG !!

            std::string binAddress;
            std::string memValString = line.substr(iNonBlank + 1);

    // !! BEGIN DEBUG !!
    std::cout << "Memory address is: " << memValString << '\n';
    // !! END DEBUG !!

            // check if we have symbol or numeric literal
            if (std::isdigit(line[iNonBlank + 1])) {

                binAddress = binaryRep(std::stoi(memValString));

            } else {

                if (symbolTable.find(memValString) != symbolTable.end()) {
    // !! BEGIN DEBUG !!
    std::cout << "Found symbol: " << memValString << '\n';
    // !! END DEBUG !!

                    binAddress = binaryRep(symbolTable.at(memValString));
                } else {
    // !! BEGIN DEBUG !!
    std::cout << "New symbol: " << memValString << ", inserting...\n";
    // !! END DEBUG !!

                    binAddress = binaryRep(freeMemoryIndex);
                    symbolTable.insert({memValString, freeMemoryIndex});
                    ++freeMemoryIndex;
                }

            }

            currInstruction = loadPrefix + binAddress;
            instructionStream << currInstruction << '\n';

        } else if (line[iNonBlank] == 'A' || line[iNonBlank] == 'M' ||
                line[iNonBlank] == 'D') {

    // !! BEGIN DEBUG !!
    std::cout << "ALU command found: " << line << '\n';
    // !! END DEBUG !!

            // ALU command invocation
            line = line.substr(iNonBlank);
            currInstruction = compPrefix + binaryCompCode(line) +
                binaryDestCode(line) + binaryJumpCode(line);

            instructionStream << currInstruction << '\n';

        } else {

            std::cerr << "WARNING: Unrecognized line format \"" << line << "\"\n";

        }
    }
}

/* ---------------------------------------------------------------------------------------------- */

// only for 15-bit numbers

std::string Assembler::binaryRep(const int val) const {

    int mask = 1;
    int remainingVal = val;
    std::string binString;
    const unsigned bitCount = 15;

    // this loop flips endian-ness
    for (unsigned i = 0; i < bitCount; ++i){
        binString += std::to_string(remainingVal & mask);
        remainingVal = remainingVal >> 1;
    }

    // restore endian-ness
    std::reverse(binString.begin(), binString.end());

    return binString;
}

/* ---------------------------------------------------------------------------------------------- */

std::string Assembler::binaryCompCode(const std::string & command) const {

    auto equalPos = command.find("=");

    if (equalPos == std::string::npos) {
        std::cerr << "ERROR: Malformed command instruction \"" << command << "\"\n";
        exit(EXIT_FAILURE);
    }

    std::string fullCmd = command.substr(equalPos + 1);

    // !! BEGIN DEBUG !!
    std::cout << "In binaryCompCode(), fullCmd is: " << fullCmd << '\n';
    // !! END DEBUG !!

    size_t semiPos = fullCmd.find(jmpSeparator);

    // !! BEGIN DEBUG !!
    std::cout << "In binaryCompCode(), semiPos is: " << semiPos << '\n';
    // !! END DEBUG !!

    std::string aluCmd = fullCmd.substr(0, semiPos);

    // !! BEGIN DEBUG !!
    std::cout << "In binaryCompCode(), aluCmd is: " << aluCmd << '\n';
    // !! END DEBUG !!

    std::string cmdBinary;

    if (opCodes.find(aluCmd) != opCodes.end()) {

        cmdBinary = opCodes.at(aluCmd);

    } else {

        std::cerr << "ERROR: Unrecognized command string \"" << aluCmd << "\"\n";
        exit(EXIT_FAILURE);

    }

    return cmdBinary;
}

/* ---------------------------------------------------------------------------------------------- */

std::string Assembler::binaryDestCode(const std::string & command) const {

    std::string destA = "0";
    std::string destD = "0";
    std::string destM = "0";

    auto equalPos = command.find("=");

    // !! BEGIN DEBUG !!
    std::cout << "In binaryDestCode(), equalPos is: " << equalPos << '\n';
    // !! END DEBUG !!

    if (equalPos == std::string::npos) {
        std::cerr << "ERROR: Malformed command instruction \"" << command << "\"\n";
        exit(EXIT_FAILURE);
    }

    std::string destPart = command.substr(0, equalPos);

    // !! BEGIN DEBUG !!
    std::cout << "In binaryDestCode(), destPart is: " << destPart << '\n';
    // !! END DEBUG !!

    if (destPart.find('A') != std::string::npos)
        destA = "1";

    if (destPart.find('D') != std::string::npos)
        destD = "1";

    if (destPart.find('M') != std::string::npos)
        destM = "1";

    return destA + destD + destM;
}

/* ---------------------------------------------------------------------------------------------- */

std::string Assembler::binaryJumpCode(const std::string & command) const {

    std::string jmpBinary = "000";
    auto semiPos = command.find(jmpSeparator);

    if (semiPos != std::string::npos) {

        std::string jmpString = command.substr(semiPos + 1);

        // !! BEGIN DEBUG !!
        std::cout << "In binaryJumpCode(), jmpString is: " << jmpString << '\n';
        // !! END DEBUG !!

        if (!jmpString.empty()) {

            if (jumpCodes.find(jmpString) != jumpCodes.end()) {

                jmpBinary = jumpCodes.at(jmpString);

            } else {

                std::cerr << "ERROR: Unrecognized jump instruction \"" << jmpString << "\"\n";
                exit(EXIT_FAILURE);

            }
        }
    }

    return jmpBinary;
}

/* ---------------------------------------------------------------------------------------------- */
