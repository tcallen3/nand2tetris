#include "parser.h"

using tokenizer = boost::tokenizer<boost::char_separator<char> >;

// TODO: Add debug output logging

/* -------------------------------------------------------------------------- */

Parser::Parser(const std::string & fileName) :
    inFile(fileName),
    currLine(),
    command(),
    arg1(),
    arg2()
{
    if (!inFile.is_open()) {
        std::err << "ERROR: Could not open file \"" << fileName << "\"\n";
        std::exti(EXIT_FAILURE);
    }
}

/* -------------------------------------------------------------------------- */

bool Parser::Advance() {

    if (!inFile.good()) {
        return false;
    }

    std::getline(inFile, currLine);

    size_t commentPos = currLine.find(commentInitializer);
    std::string workingLine = currLine.substr(0, commentPos);

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

std::string Parser::CommandText() {

    return command;

}

/* -------------------------------------------------------------------------- */

std::string Parser::FirstArg() {

    return arg1;

}

/* -------------------------------------------------------------------------- */

std::string Parser::SecondArg() {

    return arg2;

}

/* -------------------------------------------------------------------------- */
