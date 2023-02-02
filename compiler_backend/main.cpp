#include "code_writer.h"
#include "parser.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

const std::string outExt = ".asm";

void TranslateVMFile(Parser& parser, CodeWriter& writer);

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <.vm file or directory>\n";
        std::exit(EXIT_FAILURE);
    }

    fs::path inputPath(argv[1]);
    std::string outName = "";

    if (fs::exists(inputPath)) {
        if (fs::is_regular_file(inputPath)) {
            outName = inputPath.stem();
            outName += outExt;

            CodeWriter writer(outName);
            writer.SetFileName(inputPath.stem());
            Parser parser(inputPath.filename());

            writer.WriteInit();

            TranslateVMFile(parser, writer);

        } else if (fs::is_directory(inputPath)) {
            // assumes translator is called on relative path of dir
            // i.e. using -> ./VMTranslator mydir
            fs::path outPath =
                inputPath.parent_path() / inputPath.parent_path();
            outName = outPath;
            outName += outExt;

            CodeWriter writer(outName);

            writer.WriteInit();

            for (auto& p : fs::directory_iterator(inputPath)) {
                Parser parser(p.path());
                writer.SetFileName(p.path());
                TranslateVMFile(parser, writer);
            }

        } else {
            std::cerr << "ERROR: Unsupported file type for " << inputPath
                      << '\n';
        }

    } else {
        std::cerr << inputPath << " does not exist\n";
    }

    return 0;
}

/* -------------------------------------------------------------------------- */

void TranslateVMFile(Parser& parser, CodeWriter& writer) {
    while (parser.Advance()) {
        if (parser.CommandText() == "") continue;

        Command currCommand = parser.CommandType();
        if (currCommand == Command::ARITHMETIC) {
            writer.WriteArithmetic(parser.CommandText());

        } else if (currCommand == Command::PUSH ||
                   currCommand == Command::POP) {
            int index = std::stoi(parser.SecondArg());
            writer.WritePushPop(currCommand, parser.FirstArg(), index);

        } else if (currCommand == Command::LABEL) {
            writer.WriteLabel(parser.FirstArg());

        } else if (currCommand == Command::GOTO) {
            writer.WriteGoto(parser.FirstArg());

        } else if (currCommand == Command::IF) {
            writer.WriteIf(parser.FirstArg());

        } else if (currCommand == Command::FUNCTION) {
            int nLocals = std::stoi(parser.SecondArg());
            writer.WriteFunction(parser.FirstArg(), nLocals);

        } else if (currCommand == Command::CALL) {
            int nArgs = std::stoi(parser.SecondArg());
            writer.WriteCall(parser.FirstArg(), nArgs);

        } else if (currCommand == Command::RETURN) {
            writer.WriteReturn();

        } else {
            std::cerr << "WARNING: Unsupported command type\n";
        }
    }
}

/* -------------------------------------------------------------------------- */
