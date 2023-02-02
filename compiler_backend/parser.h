#ifndef PARSER_H
#define PARSER_H

#include <fstream>
#include <string>

enum class Command {    ARITHMETIC, PUSH, POP, LABEL,
                        GOTO, IF, FUNCTION, RETURN, CALL  };
class Parser {
    public:
        Parser(const std::string & fileName);
        // remove unwanted constructors

        bool Advance();
        Command CommandType();
        std::string CommandText();
        std::string FirstArg();
        std::string SecondArg();

    private:
        std::ifstream inFile;

        std::string currLine;
        std::string command;
        std::string arg1;
        std::string arg2;
};

#endif /* PARSER_H */
