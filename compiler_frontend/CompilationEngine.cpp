#include "CompilationEngine.h"

#include <cstdlib>
#include <iostream>

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
    jtok.Advance();

    // className
    if (jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        std::string errMsg = "Class name must be a valid identifier";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // '{' literal
    PrintLiteralSymbol("{", "class declaration");

    // check for classVarDec vs. subroutineDec and repeat
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
    PrintLiteralSymbol("}", "class declaration");

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

void CompilationEngine::PrintLiteralSymbol(const std::string& symbol,
                                           const std::string& locationDesc) {
    if (jtok.GetToken() != symbol) {
        const std::string errMsg = "Missing " + symbol + " in " + locationDesc;
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();
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
    if (validTypes.find(jtok.GetToken()) == validTypes.end() &&
        jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        const std::string errMsg = "Invalid type for variable declaration";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // varName
    if (jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        const std::string errMsg = "Invalid name for variable declaration";
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

    // syntax: ('constructor' | 'function' | 'method') ('void' | type)
    //         subroutineName '(' parameterList ')' subroutineBody

    // print function type (already checked in caller)
    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // ('void' | type)
    std::set<std::string> functionTypes(validTypes);
    functionTypes.insert("void");

    if (functionTypes.find(jtok.GetToken()) == functionTypes.end() &&
        jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        const std::string errMsg =
            "Invalid type for function return type declaration";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // subroutineName -> identifier
    if (jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        const std::string errMsg = "Invalid identifier for function name";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // literal '('
    PrintLiteralSymbol("(", "subroutine parameter list");

    // optional parameterList -> variable declaration
    if (jtok.GetToken() != ")") {
        CompileParameterList();
    }

    // literal ')'
    PrintLiteralSymbol(")", "subroutine parameter list");

    // subroutineBody
    CompileSubroutineBody();

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileParameterList() {
    const std::string xmlName = "parameterList";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: ((type varName) (',' type varName)*)?
    // note that this is the same (apart from parens) as standard var dec
    CompileVarDecCommon(")");

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileSubroutineBody() {
    const std::string xmlName = "subroutineBody";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: '{' varDec* statements '}'

    // literal '{'
    PrintLiteralSymbol("{", "subroutine body");

    // varDec* statements
    while (jtok.GetToken() != "}") {
        if (jtok.GetToken() == "var") {
            CompileVarDec();
        } else {
            CompileStatements();
        }
    }

    // literal '}'
    PrintLiteralSymbol("}", "subroutine body");

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileVarDec() {
    const std::string xmlName = "varDec";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: 'var' type varName (',' varName)* ';'

    // 'var', already checked by caller
    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // type, etc.
    CompileVarDecCommon(";");

    // literal ';'
    PrintLiteralSymbol(";", "variable declaration");

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileStatements() {
    const std::string xmlName = "statements";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: statement* where statement is prefixed by one of
    // 'let', 'if', 'while', 'do', 'return'
    while (jtok.TokenType() == JackTokenizer::KEYWORD) {
        const std::string token = jtok.GetToken();
        if (token == "let") {
            CompileLet();
        } else if (token == "if") {
            CompileIf();
        } else if (token == "while") {
            CompileWhile();
        } else if (token == "do") {
            CompileDo();
        } else if (token == "return") {
            CompileReturn();
        } else {
            const std::string errMsg = "Unexpected token in statement";
            compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
        }
    }

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileDo() {
    const std::string xmlName = "doStatement";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: 'do' subroutineCall ';'

    // 'do' -> checked by caller
    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // subroutineCall -> identifier

    // literal ';'
    PrintLiteralSymbol(";", "subroutine call");

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileLet() {
    const std::string xmlName = "letStatement";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // TODO: add content

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileWhile() {
    const std::string xmlName = "whileStatement";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // TODO: add content

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileReturn() {
    const std::string xmlName = "returnStatement";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: 'return' expression? ';'

    // 'return' -> checked by caller
    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // check for expression
    if (jtok.GetToken() != ";") {
        CompileExpression();
    }

    // literal ';'
    PrintLiteralSymbol(";", "return statement");

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileIf() {
    const std::string xmlName = "ifStatement";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // TODO: add content

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

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
