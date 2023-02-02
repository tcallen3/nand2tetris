#include "VMWriter.h"

#include <iostream>

/* -------------------------------------------------------------------------- */

VMWriter::VMWriter(std::ofstream& out) : outFile(out) {
    /*
        if (!outFile.is_open()) {
            std::cerr << "ERROR: Could not open file \"" << outName << "\"\n";
            std::exit(EXIT_FAILURE);
        }
    */
}

/* -------------------------------------------------------------------------- */

void VMWriter::WritePush(const Segment segment, const int index) {
    outFile << "push " << segNames.at(segment) << ' ' << index << '\n';
}

/* -------------------------------------------------------------------------- */

void VMWriter::WritePop(const Segment segment, const int index) {
    outFile << "pop " << segNames.at(segment) << ' ' << index << '\n';
}

/* -------------------------------------------------------------------------- */

void VMWriter::WriteArithmetic(const Command command) {
    outFile << commandNames.at(command) << '\n';
}

/* -------------------------------------------------------------------------- */

void VMWriter::WriteLabel(const std::string& label) {
    outFile << "label " << label << '\n';
}

/* -------------------------------------------------------------------------- */

void VMWriter::WriteGoto(const std::string& label) {
    outFile << "goto " << label << '\n';
}

/* -------------------------------------------------------------------------- */

void VMWriter::WriteIf(const std::string& label) {
    outFile << "if-goto " << label << '\n';
}

/* -------------------------------------------------------------------------- */

void VMWriter::WriteCall(const std::string& name, const int nArgs) {
    outFile << "call " << name << ' ' << nArgs << '\n';
}

/* -------------------------------------------------------------------------- */

void VMWriter::WriteFunction(const std::string& name, const int nLocals) {
    outFile << "function " << name << ' ' << nLocals << '\n';
}

/* -------------------------------------------------------------------------- */

void VMWriter::WriteReturn() { outFile << "return\n"; }

/* -------------------------------------------------------------------------- */
