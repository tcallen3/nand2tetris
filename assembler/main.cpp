#include "assembler.h"

#include <iostream>
#include <cstdlib>

int main(int argc, char * argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <asm_file>\n";
        exit(EXIT_FAILURE);
    }

    Assembler hackAssembler(argv[1]);

    hackAssembler.parseCode();

    hackAssembler.writeOutput();

    return 0;
}
