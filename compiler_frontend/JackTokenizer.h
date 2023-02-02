#ifndef JACK_TOKENIZER_H
#define JACK_TOKENIZER_H

#include "ErrorHandler.h"

#include <fstream>
#include <map>
#include <set>
#include <string>

class JackTokenizer {
  public:
    JackTokenizer(const std::string& fileName);

    // remove unwanted constructors
    JackTokenizer(const JackTokenizer& that) = delete;
    JackTokenizer(const JackTokenizer&& that) = delete;
    JackTokenizer& operator=(const JackTokenizer& that) = delete;
    JackTokenizer& operator=(const JackTokenizer&& that) = delete;

    // enums for various program components
    enum Token {
        KEYWORD,
        SYMBOL,
        IDENTIFIER,
        INT_CONST,
        STRING_CONST,
        COMMENT,
        SPACE,
        EMPTY,
    };

    enum Keyword {
        CLASS,
        METHOD,
        FUNCTION,
        CONSTRUCTOR,
        INT,
        BOOLEAN,
        CHAR,
        VOID,
        VAR,
        STATIC,
        FIELD,
        LET,
        DO,
        IF,
        ELSE,
        WHILE,
        RETURN,
        TRUE,
        FALSE,
        NULL_KEY,
        THIS,
    };

    // public interface
    bool HasMoreTokens() const { return moreTokens; }
    void Advance();
    Token TokenType() const { return currTokenType; }
    std::string GetToken() const { return currToken; }
    std::string LookaheadToken() const;
    Keyword KeywordType() const { return currKeyword; }
    char Symbol() const { return currSymbol; }
    std::string Identifier() const { return currIdentifier; }
    int IntVal() const { return currIntVal; }
    std::string StringVal() const { return currStringVal; }
    unsigned LineNum() const { return currLineNum; }
    size_t ColNum() const { return currColumnNum; }

    // data
  private:
    std::ifstream inFile;
    std::string fname;
    unsigned currLineNum;
    size_t currColumnNum;

    std::string currLine;

    bool moreTokens;
    std::string currToken;
    Token currTokenType;
    Keyword currKeyword;
    char currSymbol;
    std::string currIdentifier;
    int currIntVal;
    std::string currStringVal;

    ErrorHandler tokErrHandler;

    const std::string multiCommentCloser = "*/";

    const std::set<char> symbolSet = {'{', '}', '(', ')', '[', ']', '.',
                                      ',', ';', '+', '-', '*', '/', '&',
                                      '|', '<', '>', '=', '~'};

    const std::map<std::string, Keyword> keywordMap = {
        {"class", CLASS},
        {"method", METHOD},
        {"function", FUNCTION},
        {"constructor", CONSTRUCTOR},
        {"int", INT},
        {"boolean", BOOLEAN},
        {"char", CHAR},
        {"void", VOID},
        {"var", VAR},
        {"static", STATIC},
        {"field", FIELD},
        {"let", LET},
        {"do", DO},
        {"if", IF},
        {"else", ELSE},
        {"while", WHILE},
        {"return", RETURN},
        {"true", TRUE},
        {"false", FALSE},
        {"null", NULL_KEY},
        {"this", THIS}};

    // methods
  private:
    void AdvanceDriver();
    void ReadNextLine();
    void ParseDelimitedComment();
    void ParseStringLiteral();
    void ParseIntLiteral();
    void ParseKeywordIdentifier();
};

#endif /* JACK_TOKENIZER_H */
