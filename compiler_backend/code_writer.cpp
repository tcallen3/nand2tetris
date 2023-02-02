#include "code_writer.h"

#include <cstdlib>

/* -------------------------------------------------------------------------- */

CodeWriter::CodeWriter(const std::string & outName) :
    jumpIndex(0),
    outFile(outName),
    infileName("XXX")
{
    if (!outFile.is_open()) {
        std::cerr << "ERROR: Could not open output file " << outName << '\n';
        std::exit(EXIT_FAILURE);
    }
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WriteArithmetic(const std::string & command) {

    if (binaryCommands.find(command) != binaryCommands.end()) {

        WriteBinaryOp(command);

    } else if (unaryCommands.find(command) != unaryCommands.end()) {

        WriteUnaryOp(command);

    } else {

        std::cerr << "WARNING: Unrecognized arithmetic command \"" <<
            command << "\"\n";

    }
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WritePushPop(const Command ptype, const std::string & segment,
                              const int index) {

    if (ptype == pushCommand) {

        WritePush(segment, index);

    } else if (ptype == popCommand) {

        WritePop(segment, index);

    } else {
        std::cerr << "WARNING: Unrecognized stack command \"" <<
            ptype << "\"\n";
    }
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WritePush(const std::string & segment, const int index) {

    if (segment == constSegment) {

        outFile << '@' << index << '\n';    // int literal
        outFile << "D=A\n";                 // transfer to register
        outFile << "@SP\n";                 // look up stack pointer
        outFile << "A=M\n";                 // A = pointer val
        outFile << "M=D\n";                 // M[val] = index
        outFile << "D=A\n";                 // D = val
        outFile << "@SP\n";                 // look up stack pointer
        outFile << "M=D+1\n";               // increment stack pointer

    } else {

        std::cerr << "WARNING: unrecognized segment \"" << segment << "\"\n";

    }

}

/* -------------------------------------------------------------------------- */

void CodeWriter::WritePop(const std::string & segment, const int index) {

    std::cerr << "WARNING: stack pop command not yet supported\n";

}

/* -------------------------------------------------------------------------- */

void CodeWriter::WriteBinaryOp(const std::string & command) {

    // push result from D
    const std::string targetReg = "D";

    // pop y to D, x to A
    PopRegister("D");
    PopRegister("A");

    WriteOpCommand(command);

    PushRegister(targetReg);
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WriteUnaryOp(const std::string & command) {

    // push result from D
    const std::string targetReg = "D";

    // pop x to D
    PopRegister("D");

    WriteOpCommand(command);

    PushRegister(targetReg)
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WriteOpCommand(const std::string & command) {

    switch (command) {

        case "add" :

            outFile << "D=A+D\n";
            break;

        case "sub" :

            outFile << "D=A-D\n";
            break;

        case "eq" :

            WriteComparison("JEQ");
            break;

        case "gt" :

            WriteComparison("JGT");
            break;

        case "lt" :

            WriteComparison("JLT");
            break;

        case "and" :

            outFile << "D=A&D\n";
            break;

        case "or" :

            outFile << "D=A|D\n";
            break;

        case "neg" :

            outFile << "D=-D\n";
            break;

        case "not" :

            outFile << "D=!D\n";
            break;

        default :
            std::cerr << "WARNING: Unrecognized operator \"" <<
                command << "\"\n";
            break;
    }

}

/* -------------------------------------------------------------------------- */

void CodeWriter::PushRegister(const std::string & reg) {

        outFile << "@SP\n";                 // look up stack pointer
        outFile << "A=M\n";                 // A = pointer val
        outFile << "M=" << reg << "\n";     // M[val] = reg
        outFile << "D=A\n";                 // D = val
        outFile << "@SP\n";                 // look up stack pointer
        outFile << "M=D+1\n";               // increment stack pointer
}

/* -------------------------------------------------------------------------- */

void CodeWriter::PopRegister(const std::string & reg) {

        outFile << "@SP\n";         // look up stack pointer
        outFile << "M=M-1\n";       // decrement SP
        outFile << "@SP\n";         // reload SP
        outFile << "A=M\n";         // A = pointer val
        outFile << reg << "=M\n";   // target reg = M[val]
}

/* -------------------------------------------------------------------------- */

void CodeWriter::WriteComparison(const std::string & op) {

    outFile << "D=A-D\n";
    outFile << "@EQ" << jumpIndex << '\n';
    outFile << "D;" << op << '\n';
    outFile << "@SP\n";
    outFile << "A=M\n";
    outFile << "M=0\n";
    outFile << "@TERM" << jumpIndex << '\n';
    outFile << "0;JMP\n";
    outFile << "(EQ" << jumpIndex << ")\n";
    outFile << "@SP\n";
    outFile << "A=M\n";
    outFile << "M=-1\n";
    outFile << "(TERM" << jumpIndex << ")\n";

    ++jumpIndex;
}

/* -------------------------------------------------------------------------- */

