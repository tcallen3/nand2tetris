#ifndef COMPILATION_ENGINE_H
#define COMPILATION_ENGINE_H

#include "JackTokenizer.h"

#include <fstream>
#include <set>
#include <string>

class CompilationEngine {
  public:
    CompilationEngine(const std::string& infileName,
                      const std::string& outfileName);

    // remove unwanted constructors
    CompilationEngine(const CompilationEngine& that) = delete;
    CompilationEngine(const CompilationEngine&& that) = delete;
    CompilationEngine& operator=(const CompilationEngine& that) = delete;
    CompilationEngine& operator=(const CompilationEngine&& that) = delete;

    void CompileClass();

    // data
  private:
    std::string currInputFile;
    std::ofstream outFile;
    JackTokenizer jtok;
    ErrorHandler compilerErrorHandler;

    enum TagType { OPENING, CLOSING };

    std::string currIndent;

    std::map<JackTokenizer::Token, std::string> tokenString = {
        {JackTokenizer::KEYWORD, "keyword"},
        {JackTokenizer::SYMBOL, "symbol"},
        {JackTokenizer::IDENTIFIER, "identifier"},
        {JackTokenizer::INT_CONST, "integerConstant"},
        {JackTokenizer::STRING_CONST, "stringConstant"},
    };

    const std::set<std::string> validTypes = {"int", "char", "boolean"};
    const std::set<std::string> unaryOpTypes = {"-", "~"};
    const std::set<std::string> expressionOpTypes = {"+", "-", "*", "/", "&",
                                                     "|", "<", ">", "="};
    const std::set<std::string> expressionTerminals = {"]", ";", ",", ")"};
    const std::set<std::string> keywordConstants = {"true", "false", "null",
                                                    "this"};

    // methods
  private:
    void PrintToken(const std::string& typeName, const std::string& token);
    void PrintXMLTag(const std::string& tagName, TagType type);
    void PrintNodeTag(const std::string& tagName, TagType type);
    void PrintLiteralSymbol(const std::string& symbol,
                            const std::string& locationDesc);

    void CompileClassVarDec();
    void CompileVarDecCommon(const std::string& terminal);
    void CompileSubroutine();
    void CompileParameterList();
    void CompileSubroutineBody();

    void CompileVarDec();

    void CompileStatements();
    void CompileDo();
    void CompileLet();
    void CompileWhile();
    void CompileReturn();
    void CompileIf();

    void CompileSubroutineCall();
    void CompileExpression();
    void CompileTerm();
    void CompileExpressionList();
};

#endif /* COMPILATION_ENGINE_H */
