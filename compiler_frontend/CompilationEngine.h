#ifndef COMPILATION_ENGINE_H
#define COMPILATION_ENGINE_H

#include "JackTokenizer.h"

#include <fstream>
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
        {JackTokenizer::IDENTFIER, "identifier"},
        {JackTokenizer::INT_CONST, "integerConstant"},
        {JackTokenizer::STRING_CONST, "stringConstant"},
    };

    // methods
  private:
    void CompilationEngine::PrintToken(const std::string& typeName,
                                       const std::string& token);
    void PrintXMLTag(const std::string& tagName, TagType type);

    void CompileClassVarDec();
    void CompileSubroutine();
    void CompileParameterList();

    void CompileVarDec();

    void CompileStatements();
    void CompileDo();
    void CompileLet();
    void CompileWhile();
    void CompileReturn();
    void CompileIf();

    void CompileExpression();
    void CompileTerm();
    void CompileExpressionList();
};

#endif /* COMPILATION_ENGINE_H */
