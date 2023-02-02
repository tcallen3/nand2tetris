#include "parser.h"
#include "code_writer.h"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

const std::string outExt = ".asm";

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
            Parser parser(inputPath.filename());

            // TODO: logic here



        } else if (fs::is_directory(inputPath)) {

            // TODO: handle with directory iterator



        } else {
            std::cerr << "ERROR: Unsupported file type for " << inputPath << '\n';

        }

    } else {
        std::cerr << inputPath << " does not exist\n";

    }

    return 0;
}
