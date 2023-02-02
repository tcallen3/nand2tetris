#include "CompilationEngine.h"

#include <cstdlib>
#include <iostream>
#include <set>

/* -------------------------------------------------------------------------- */

CompilationEngine::CompilationEngine(const std::string& infileName,
                                     const std::string& outfileName) :
        currInputFile(infileName),
        outFile(outfileName),
        jtok(infileName),
        compilerErrorHandler() {
    if (!outFile.is_open()) {
        std::cerr << "ERROR: Could not open file \"" << outfileName << "\"\n";
        std::exit(EXIT_FAILURE);
    }
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileClass() {
    const std::string xmlName = "class";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: 'class' className '{' classVarDec* subroutineDec* '}'

    // first call so advance the tokenizer
    jtok.Advance();

    // 'class'
    if (!(jtok.TokenType() == JackTokenizer::KEYWORD &&
          jtok.KeywordType() == JackTokenizer::CLASS)) {
        std::string errMsg =
            "Missing 'class' declaration. All code must be wrapped in classes";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());

    // className
    jtok.Advance();

    if (jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        std::string errMsg = "Class name must be a valid identifier";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());

    // '{' literal
    jtok.Advance();

    if (!(jtok.TokenType() == JackTokenizer::SYMBOL &&
          jtok.GetToken() == "{")) {
        std::string errMsg = "Missing opening bracket in class declaration";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());

    // check for classVarDec vs. subroutineDec and repeat
    jtok.Advance();

    while (jtok.TokenType() != JackTokenizer::SYMBOL) {
        const std::string token = jtok.GetToken();

        if (token == "static" || token == "field") {
            CompileClassVarDec();
        } else if (token == "constructor" || token == "function" ||
                   token == "method") {
            CompileSubroutine();
        } else {
            const std::string errMsg =
                "Unrecognized statement in class declaration";
            compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
        }
    }

    // '}' literal
    // no advancing since previous loop caught us up to here
    if (!(jtok.TokenType() == JackTokenizer::SYMBOL &&
          jtok.GetToken() == "}")) {
        std::string errMsg = "Missing closing bracket in class declaration";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::PrintToken(const std::string& typeName,
                                   const std::string& token) {
    outFile << currIndent;
    PrintXMLTag(typeName, OPENING);
    outFile << " " << token << " ";
    PrintXMLTag(typeName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::PrintXMLTag(const std::string& tagName, TagType type) {
    outFile << '<';

    if (type == CLOSING) {
        outFile << '/';
    }

    outFile << tagName << '>';

    if (type == CLOSING) {
        outFile << '\n';
    }
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::PrintNodeTag(const std::string& tagName, TagType type) {
    outFile << currIndent;

    outFile << '<';

    if (type == CLOSING) {
        outFile << '/';
    }

    outFile << tagName << '>';

    outFile << '\n';
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileClassVarDec() {
    const std::string xmlName = "classVarDec";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: ('static' | 'field') type varName (',' varName)* ';'

    // static/field dec (already selected in caller)
    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());

    jtok.Advance();

    CompileVarDecCommon(";");

    // terminal semicolon
    if (jtok.GetToken() != ";") {
        const std::string errMsg = "Missing semicolon";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileVarDecCommon(const std::string& terminal) {

    // type must be int, char, boolean, or class ID
    const std::set<std::string> allowedTypes = {"int", "char", "boolean"};

    if (allowedTypes.find(jtok.GetToken()) == allowedTypes.end() &&
        jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        const std::string errMsg =
            "Invalid type for variable declaration";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // varName
    if (jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        const std::string errMsg =
            "Invalid name for variable declaration";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // optional comma-separated identifiers
    while (jtok.GetToken() != terminal) {
        if (!(jtok.TokenType() == JackTokenizer::SYMBOL ||
              jtok.TokenType() == JackTokenizer::IDENTIFIER)) {
            const std::string errMsg =
                "Unexpected token in variable declaration";
            compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
        }

        PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
        jtok.Advance();
    }

}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileSubroutine() {
    const std::string xmlName = "subroutineDec";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // TODO: add content

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileParameterList() {}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileSubroutineBody() {}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileVarDec() {}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileStatements() {}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileDo() {}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileLet() {}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileWhile() {}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileReturn() {}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileIf() {}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileExpression() {}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileTerm() {
    // TODO: implement term detection
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileExpressionList() {
    // TODO: implement recursive expression list compilation
}

/* -------------------------------------------------------------------------- */
