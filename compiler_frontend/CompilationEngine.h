#ifndef COMPILATION_ENGINE_H
#define COMPILATION_ENGINE_H

#include "ErrorHandler.h"
#include "JackTokenizer.h"
#include "SymbolTable.h"
#include "VMWriter.h"

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
    std::string currClass;
    std::ofstream outFile;
    unsigned loopCount;
    unsigned branchCount;
    JackTokenizer jtok;
    ErrorHandler compilerErrorHandler;
    SymbolTable symTable;
    VMWriter vmWriter;

    enum TagType { OPENING, CLOSING };

    enum Category { VAR, ARGUMENT, STATIC, FIELD, CLASS, SUBROUTINE };
    enum Status { USED, DEFINED };

    std::string currIndent;

    const std::string loopBase = "WHILE_LOOP";
    const std::string branchBase = "IF_STATEMENT";
    const std::string endPrefix = "END_";

    const std::map<JackTokenizer::Token, std::string> tokenString = {
        {JackTokenizer::KEYWORD, "keyword"},
        {JackTokenizer::SYMBOL, "symbol"},
        {JackTokenizer::IDENTIFIER, "identifier"},
        {JackTokenizer::INT_CONST, "integerConstant"},
        {JackTokenizer::STRING_CONST, "stringConstant"},
    };

    const std::map<Category, std::string> categoryNames = {
        {VAR, "var"},     {ARGUMENT, "argument"}, {STATIC, "static"},
        {FIELD, "field"}, {CLASS, "class"},       {SUBROUTINE, "subroutine"}};

    const std::map<SymbolTable::VARKIND, Category> kindMap = {
        {SymbolTable::VAR, VAR},
        {SymbolTable::ARG, ARGUMENT},
        {SymbolTable::FIELD, FIELD},
        {SymbolTable::STATIC, STATIC}};

    const std::set<std::string> validTypes = {"int", "char", "boolean"};
    const std::set<std::string> unaryOpTypes = {"-", "~"};
    const std::set<std::string> expressionOpTypes = {"+", "-", "*", "/", "&",
                                                     "|", "<", ">", "="};
    const std::set<std::string> expressionTerminals = {"]", ";", ",", ")"};
    const std::set<std::string> keywordConstants = {"true", "false", "null",
                                                    "this"};

    const std::map<std::string, VMWriter::Command> binaryOpCommands = {
        {"+", VMWriter::ADD}, {"-", VMWriter::SUB}, {"*", VMWriter::MULT},
        {"/", VMWriter::DIV}, {"&", VMWriter::AND}, {"|", VMWriter::OR},
        {"<", VMWriter::LT},  {">", VMWriter::GT},  {"=", VMWriter::EQ},
    };

    const std::map<std::string, VMWriter::Command> unaryOpCommands = {
        {"-", VMWriter::NEG}, {"~", VMWriter::NOT}};

    // methods
  private:
    void PrintToken(const std::string& typeName, const std::string& token);
    void PrintXMLTag(const std::string& tagName, TagType type);
    void PrintNodeTag(const std::string& tagName, TagType type);
    void PrintLiteralSymbol(const std::string& symbol,
                            const std::string& locationDesc);
    void CheckLiteralSymbol(const std::string& symbol,
                            const std::string& locationDesc);
    void PrintIdentifier(const Category category, const Status status);

    void CompileClassVarDec();
    void CompileVarDecCommon(const std::string& terminal,
                             const Category category);
    void CompileSubroutine();
    void CompileParameterList();
    void CompileSubroutineBody(const std::string& funcName);

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
    int CompileExpressionList();
};

#endif /* COMPILATION_ENGINE_H */
