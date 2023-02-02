#ifndef PARSER_H
#define PARSER_H

#include <boost/tokenizer.hpp>

#include <fstream>
#include <map>
#include <set>
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
    Parser(const Parser& that) = delete;
    Parser(const Parser&& that) = delete;
    Parser& operator=(const Parser& that) = delete;
    Parser& operator=(const Parser&& that) = delete;

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

    boost::char_separator<char> sep;

    const std::string commentInitializer = "//";

    const std::set<std::string> arithmeticCommands = {
        "add", "sub", "eq", "gt", "lt", "and", "or", "neg", "not"};

    const std::map<std::string, Command> controlCommands = {
        {"push", Command::PUSH},
        {"pop", Command::POP},
        {"label", Command::LABEL},
        {"goto", Command::GOTO},
        {"if-goto", Command::IF},
        {"function", Command::FUNCTION},
        {"call", Command::CALL},
        {"return", Command::RETURN}};
};

#endif /* PARSER_H */
