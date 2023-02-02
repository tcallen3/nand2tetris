#include "code_writer.h"

#include <cstdlib>
#include <iostream>

/* -------------------------------------------------------------------------- */

CodeWriter::CodeWriter(const std::string& outName) :
        jumpIndex(0),
        outFile(outName),
        infileName("XXX") {
    if (!outFile.is_open()) {
        std::cerr << "ERROR: Could not open output file " << outName << '\n';
        std::exit(EXIT_FAILURE);
    }
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WriteArithmetic(const std::string& command) {
    if (binaryCommands.find(command) != binaryCommands.end()) {
        WriteBinaryOp(command);

    } else if (unaryCommands.find(command) != unaryCommands.end()) {
        WriteUnaryOp(command);

    } else {
        std::cerr << "WARNING: Unrecognized arithmetic command \"" << command
                  << "\"\n";
    }
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WritePushPop(const Command ptype, const std::string& segment,
                              const int index) {
    if (ptype == Command::PUSH) {
        WritePush(segment, index);

    } else if (ptype == Command::POP) {
        WritePop(segment, index);

    } else {
        std::cerr << "WARNING: Unrecognized stack command\n";
    }
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WritePush(const std::string& segment, const int index) {
    if (segment == constSegment) {
        outFile << '@' << index << '\n';  // int literal
        outFile << "D=A\n";               // transfer to register
        PushRegister("D");

    } else if (regMap.find(segment) != regMap.end()) {
        outFile << '@' << index << '\n';               // load index
        outFile << "D=A\n";                            // D = index
        outFile << '@' << regMap.at(segment) << '\n';  // load segment
        outFile << "A=M+D\n";                          // load Seg[index]
        outFile << "D=M\n";                            // D = Seg[index]
        PushRegister("D");

    } else if (segment == tempSegment) {
        PushFixed(segment, index, tempBase, tempMaxOffset);

    } else if (segment == pointerSegment) {
        PushFixed(segment, index, pointerBase, pointerMaxOffset);

    } else if (segment == staticSegment) {
        outFile << '@' << infileName << '.' << index << '\n';
        outFile << "D=M\n";
        PushRegister("D");

    } else {
        std::cerr << "WARNING: unrecognized segment \"" << segment << "\"\n";
    }
}

/* -------------------------------------------------------------------------- */

void CodeWriter::PushFixed(const std::string& segment, const int index,
                           const int base, const int maxOffset) {
    if (index > maxOffset) {
        std::cerr << "WARNING: attempt to push invalid " << segment
                  << " offset\n";
        return;
    }

    const int address = base + index;

    outFile << '@' << address << '\n';
    outFile << "D=M\n";
    PushRegister("D");
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WritePop(const std::string& segment, const int index) {
    if (regMap.find(segment) != regMap.end()) {
        PopRegister("D");  // put val in D reg

        outFile << "@R13\n";                           // load scratch mem
        outFile << "M=D\n";                            // R13 = val
        outFile << '@' << index << '\n';               // load index
        outFile << "D=A\n";                            // D = index
        outFile << '@' << regMap.at(segment) << '\n';  // load segment
        outFile << "A=M+D\n";                          // load Seg[index]
        outFile << "D=A\n";                            // D = addr Seg[index]
        outFile << "@R14\n";                           // load scratch mem
        outFile << "M=D\n";                            // R14 = address
        outFile << "@R13\n";                           // load scratch mem
        outFile << "D=M\n";                            // D = val
        outFile << "@R14\n";                           // load scratch mem
        outFile << "A=M\n";                            // load address
        outFile << "M=D\n";                            // M[address] = val

    } else if (segment == tempSegment) {
        PopFixed(segment, index, tempBase, tempMaxOffset);

    } else if (segment == pointerSegment) {
        PopFixed(segment, index, pointerBase, pointerMaxOffset);

    } else if (segment == staticSegment) {
        PopRegister("D");

        outFile << '@' << infileName << '.' << index << '\n';
        outFile << "M=D\n";

    } else {
        std::cerr << "WARNING: unrecognized segment \"" << segment << "\"\n";
    }
}

/* -------------------------------------------------------------------------- */

void CodeWriter::PopFixed(const std::string& segment, const int index,
                          const int base, const int maxOffset) {
    if (index > maxOffset) {
        std::cerr << "WARNING: attempt to pop invalid " << segment
                  << " offset\n";
        return;
    }

    const int address = base + index;

    PopRegister("D");

    outFile << '@' << address << '\n';
    outFile << "M=D\n";
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WriteBinaryOp(const std::string& command) {
    // push result from D
    const std::string targetReg = "D";

    // pop y to D, x to A
    PopRegister("D");
    PopRegister("A");

    WriteOpCommand(command);

    PushRegister(targetReg);
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WriteUnaryOp(const std::string& command) {
    // push result from D
    const std::string targetReg = "D";

    // pop x to D
    PopRegister("D");

    WriteOpCommand(command);

    PushRegister(targetReg);
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WriteOpCommand(const std::string& command) {
    if (command == "add")
        outFile << "D=A+D\n";

    else if (command == "sub")
        outFile << "D=A-D\n";

    else if (command == "eq")
        WriteComparison("JEQ");

    else if (command == "gt")
        WriteComparison("JGT");

    else if (command == "lt")
        WriteComparison("JLT");

    else if (command == "and")
        outFile << "D=A&D\n";

    else if (command == "or")
        outFile << "D=A|D\n";

    else if (command == "neg")
        outFile << "D=-D\n";

    else if (command == "not")
        outFile << "D=!D\n";

    else
        std::cerr << "WARNING: Unrecognized operator \"" << command << "\"\n";
}

/* -------------------------------------------------------------------------- */

void CodeWriter::PushRegister(const std::string& reg) {
    outFile << "@SP\n";              // look up stack pointer
    outFile << "A=M\n";              // A = pointer val
    outFile << "M=" << reg << "\n";  // M[val] = reg
    outFile << "D=A\n";              // D = val
    outFile << "@SP\n";              // look up stack pointer
    outFile << "M=D+1\n";            // increment stack pointer
}

/* -------------------------------------------------------------------------- */

void CodeWriter::PopRegister(const std::string& reg) {
    outFile << "@SP\n";        // look up stack pointer
    outFile << "M=M-1\n";      // decrement SP
    outFile << "@SP\n";        // reload SP
    outFile << "A=M\n";        // A = pointer val
    outFile << reg << "=M\n";  // target reg = M[val]
}

/* -------------------------------------------------------------------------- */

// NOTE: truth value indicator must be placed in D register, as this is
//       canonical operator return location
void CodeWriter::WriteComparison(const std::string& op) {
    outFile << "D=A-D\n";
    outFile << "@EQ" << jumpIndex << '\n';
    outFile << "D;" << op << '\n';
    outFile << "@SP\n";
    outFile << "A=M\n";
    outFile << "D=0\n";
    outFile << "@TERM" << jumpIndex << '\n';
    outFile << "0;JMP\n";
    outFile << "(EQ" << jumpIndex << ")\n";
    outFile << "@SP\n";
    outFile << "A=M\n";
    outFile << "D=-1\n";
    outFile << "(TERM" << jumpIndex << ")\n";

    ++jumpIndex;
}

/* -------------------------------------------------------------------------- */
