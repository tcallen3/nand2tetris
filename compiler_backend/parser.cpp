#include "parser.h"

#include <iostream>

using tokenizer = boost::tokenizer<boost::char_separator<char> >;

/* -------------------------------------------------------------------------- */

Parser::Parser(const std::string& fileName) :
        inFile(fileName),
        currLine(),
        command(),
        arg1(),
        arg2(),
        sep(" \t\r\n") {
    if (!inFile.is_open()) {
        std::cerr << "ERROR: Could not open file \"" << fileName << "\"\n";
        std::exit(EXIT_FAILURE);
    }
}

/* -------------------------------------------------------------------------- */

bool Parser::Advance() {
    if (!inFile.good()) {
        return false;
    }

    std::getline(inFile, currLine);

    // !! BEGIN DEBUG !!
    std::cout << "Line read: " << currLine << '\n';
    // !! END DEBUG !!

    size_t commentPos = currLine.find(commentInitializer);
    std::string workingLine = currLine.substr(0, commentPos);

    // !! BEGIN DEBUG !!
    std::cout << "After comment strip: " << workingLine << '\n';
    // !! END DEBUG !!

    tokenizer tok(workingLine, sep);
    auto tokIter = tok.begin();

    if (tok.begin() == tok.end()) {
        command = "";

    } else {
        command = *tokIter;
        ++tokIter;

        if (tokIter != tok.end()) {
            arg1 = *tokIter;
            ++tokIter;
        }

        if (tokIter != tok.end()) {
            arg2 = *tokIter;
            ++tokIter;
        }
    }

    // !! BEGIN DEBUG !!
    std::cout << "At end of Advance(), command: " << command << '\n';
    std::cout << "At end of Advance(), arg1: " << arg1 << '\n';
    std::cout << "At end of Advance(), arg2: " << arg2 << '\n';
    // !! END DEBUG !!

    return true;
}

/* -------------------------------------------------------------------------- */

Command Parser::CommandType() {
    if (arithmeticCommands.find(command) != arithmeticCommands.end()) {
        return Command::ARITHMETIC;

    } else if (command == pushCommand) {
        return Command::PUSH;

    } else if (command == popCommand) {
        return Command::POP;

    } else if (command.empty()) {
        return Command::EMPTY;

    } else {
        std::cerr << "WARNING: Unrecognized VM command \"" << command << "\"\n";
    }

    return Command::UNKNOWN;
}

/* -------------------------------------------------------------------------- */

std::string Parser::CommandText() { return command; }

/* -------------------------------------------------------------------------- */

std::string Parser::FirstArg() { return arg1; }

/* -------------------------------------------------------------------------- */

std::string Parser::SecondArg() { return arg2; }

/* -------------------------------------------------------------------------- */
