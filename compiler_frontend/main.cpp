#include "CompilationEngine.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

const std::string inExt = ".jack";
const std::string outExt = ".vm";

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <.jack file or directory>\n";
        std::exit(EXIT_FAILURE);
    }

    fs::path inputPath(argv[1]);
    std::string outName = "";

    if (fs::exists(inputPath)) {
        if (fs::is_regular_file(inputPath) && inputPath.extension() == inExt) {
            outName = inputPath.stem();
            outName += outExt;

            CompilationEngine compiler(inputPath.filename(), outName);

            compiler.CompileClass();

        } else if (fs::is_directory(inputPath)) {
            for (auto& p : fs::directory_iterator(inputPath)) {
                auto path = p.path();
                if (path.extension() == inExt) {
                    fs::path currFilePath = path.parent_path() / path.stem();
                    outName = currFilePath.string() + outExt;

                    CompilationEngine compiler(path.string(), outName);

                    compiler.CompileClass();
                }
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
