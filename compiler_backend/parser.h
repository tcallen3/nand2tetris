#ifndef PARSER_H
#define PARSER_H

#include <boost/tokenizer.hpp>

#include <fstream>
#include <string>

enum class Command {
    ARITHMETIC,
    PUSH,
    POP,
    LABEL,
    GOTO,
    IF,
    FUNCTION,
    RETURN,
    CALL,
    EMPTY,
    UNKNOWN
};
class Parser {
  public:
    Parser(const std::string& fileName);
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

    boost::char_separator<char> sep(" \t\r\n");

    const std::string commentInitializer = "//";
    const std::set<std::string> arithmeticCommands = {
            "add", "sub", "eq", "gt", "lt", "and", "or", "neg", "not"};
    const std::string pushCommand = "push";
    const std::string popCommand = "pop";
};

#endif /* PARSER_H */
