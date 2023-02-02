#include "CompilationEngine.h"

#include <cstdlib>
#include <iostream>

/* -------------------------------------------------------------------------- */

CompilationEngine::CompilationEngine(const std::string& infileName,
                                     const std::string& outfileName) :
        currInputFile(infileName),
        currClass(),
        outFile(outfileName),
        jtok(infileName),
        compilerErrorHandler(),
        symTable(),
        vmWriter(outFile) {
    if (!outFile.is_open()) {
        std::cerr << "ERROR: Could not open file \"" << outfileName << "\"\n";
        std::exit(EXIT_FAILURE);
    }
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileClass() {
    // syntax: 'class' className '{' classVarDec* subroutineDec* '}'

    // first call so advance the tokenizer
    jtok.Advance();

    // 'class'
    if (!(jtok.TokenType() == JackTokenizer::KEYWORD &&
          jtok.KeywordType() == JackTokenizer::CLASS)) {
        std::string errMsg =
            "Missing 'class' declaration. All code must be wrapped in classes.";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    // PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // className
    if (jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        std::string errMsg = "Class name must be a valid identifier";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    // class name must prefix all function declarations
    currClass = jtok.GetToken();

    // PrintIdentifier(CLASS, DEFINED);
    jtok.Advance();

    // '{' literal
    CheckLiteralSymbol("{", "class declaration");

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
    CheckLiteralSymbol("}", "class declaration");
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::PrintToken(const std::string& typeName,
                                   const std::string& token) {
    std::string outputToken = token;

    if (outputToken == "<") {
        outputToken = "&lt;";

    } else if (outputToken == ">") {
        outputToken = "&gt;";

    } else if (outputToken == "&") {
        outputToken = "&amp;";
    }

    outFile << currIndent;
    PrintXMLTag(typeName, OPENING);
    outFile << " " << outputToken << " ";
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

void CompilationEngine::CheckLiteralSymbol(const std::string& symbol,
                                           const std::string& locationDesc) {
    if (jtok.GetToken() != symbol) {
        const std::string errMsg = "Missing " + symbol + " in " + locationDesc;
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    jtok.Advance();
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

void CompilationEngine::PrintIdentifier(const Category category,
                                        const Status status) {
    outFile << currIndent;

    outFile << '<';
    outFile << categoryNames.at(category);

    outFile << " ";
    outFile << "status=";

    if (status == DEFINED) {
        outFile << "\"defined\" ";
    } else {
        outFile << "\"used\" ";
    }

    outFile << "isVar=";

    if (category == VAR || category == ARGUMENT || category == STATIC ||
        category == FIELD) {
        outFile << "\"true\" ";
        outFile << "index=";

        outFile << "\"" << symTable.IndexOf(jtok.GetToken()) << "\"";
    } else {
        outFile << "\"false\"";
    }

    outFile << '>';

    outFile << " " << jtok.GetToken() << " ";

    outFile << "</" << categoryNames.at(category) << ">\n";
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileClassVarDec() {
    const std::string xmlName = "classVarDec";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: ('static' | 'field') type varName (',' varName)* ';'

    // static/field dec (already selected in caller)
    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());

    // determine variable category
    const Category category = (jtok.GetToken() == "static") ? STATIC : FIELD;

    jtok.Advance();

    CompileVarDecCommon(";", category);

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

void CompilationEngine::CompileVarDecCommon(const std::string& terminal,
                                            const Category category) {
    // store variable info
    SymbolTable::VarData data;
    if (category == VAR) {
        data.kind = SymbolTable::VAR;

    } else if (category == ARGUMENT) {
        data.kind = SymbolTable::ARG;

    } else if (category == STATIC) {
        data.kind = SymbolTable::STATIC;

    } else if (category == FIELD) {
        data.kind = SymbolTable::FIELD;

    } else {
        const std::string errMsg =
            "Unsupported variable category in variable declaration";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    // type must be int, char, boolean, or class ID
    if (validTypes.find(jtok.GetToken()) == validTypes.end() &&
        jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        const std::string errMsg = "Invalid type for variable declaration";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    data.type = jtok.GetToken();

    // PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // varName
    if (jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        const std::string errMsg = "Invalid name for variable declaration";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    // add to symbol table
    symTable.Define(jtok.GetToken(), data.type, data.kind);

    // print
    // PrintIdentifier(category, DEFINED);
    jtok.Advance();

    // optional comma-separated identifiers
    while (jtok.GetToken() != terminal) {
        if (!(jtok.TokenType() == JackTokenizer::SYMBOL ||
              jtok.TokenType() == JackTokenizer::IDENTIFIER ||
              jtok.TokenType() == JackTokenizer::KEYWORD)) {
            const std::string errMsg =
                "Unexpected token in variable declaration";
            compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
        }

        if (jtok.TokenType() == JackTokenizer::IDENTIFIER) {
            symTable.Define(jtok.GetToken(), data.type, data.kind);
            // PrintIdentifier(category, DEFINED);
        } else {
            // PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
        }

        jtok.Advance();
    }
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileSubroutine() {
    // syntax: ('constructor' | 'function' | 'method') ('void' | type)
    //         subroutineName '(' parameterList ')' subroutineBody

    // clear symbol table
    symTable.StartSubroutine();

    // advance over function type (already checked in caller)
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

    bool isVoid = (jtok.GetToken() == "void");
    jtok.Advance();

    // subroutineName -> identifier
    if (jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        const std::string errMsg = "Invalid identifier for function name";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    const auto funcName = jtok.GetToken();
    jtok.Advance();

    // literal '('
    CheckLiteralSymbol("(", "subroutine parameter list");

    // optional parameterList -> variable declaration
    if (jtok.GetToken() != ")") {
        CompileParameterList();
    }

    // literal ')'
    CheckLiteralSymbol(")", "subroutine parameter list");

    // subroutineBody
    CompileSubroutineBody(funcName);

    if (isVoid) {
        vmWriter.WritePush(VMWriter::CONST, 0);
    }

    vmWriter.WriteReturn();
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileParameterList() {
    // syntax: ((type varName) (',' type varName)*)?
    // note that this is the same (apart from parens) as standard var dec
    CompileVarDecCommon(")", ARGUMENT);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileSubroutineBody(const std::string& funcName) {
    // syntax: '{' varDec* statements '}'

    // literal '{'
    CheckLiteralSymbol("{", "subroutine body");

    // varDec*
    // Note: enforcing variable-first ordering is key for our current
    //       compilation model
    while (jtok.GetToken() == "var") {
        CompileVarDec();
    }

    auto nLocal = symTable.VarCount(SymbolTable::VAR);
    auto vmName = currClass + "." + funcName;
    vmWriter.WriteFunction(vmName, nLocal);

    // statements
    while (jtok.GetToken() != "}") {
        CompileStatements();
    }

    // literal '}'
    CheckLiteralSymbol("}", "subroutine body");
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
    CompileVarDecCommon(";", VAR);

    // literal ';'
    PrintLiteralSymbol(";", "variable declaration");

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileStatements() {
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
            break;
        } else {
            const std::string errMsg = "Unexpected token in statement";
            compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
        }
    }
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileDo() {
    // syntax: 'do' subroutineCall ';'

    // advance over 'do' -> checked by caller
    jtok.Advance();

    // subroutineCall -> identifier
    if (jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        const std::string errMsg = "Invalid identifier for subroutine call";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    CompileSubroutineCall();

    // literal ';'
    CheckLiteralSymbol(";", "subroutine call");

    // function is assumed void, so pop its value and ignore
    vmWriter.WritePop(VMWriter::TEMP, 0);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileLet() {
    const std::string xmlName = "letStatement";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: 'let' varName ('[' expression ']')? '=' expression ';'

    // 'let' -> checked by caller
    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // varName -> identifier
    if (jtok.TokenType() != JackTokenizer::IDENTIFIER) {
        const std::string errMsg =
            "Invalid identifier for variable name in let statement";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    const std::string varName = jtok.GetToken();
    if (!symTable.Check(varName)) {
        const std::string errMsg =
            "Variable " + varName + " not defined in current scope";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    auto kind = symTable.KindOf(varName);
    const Category category = kindMap.at(kind);

    PrintIdentifier(category, USED);

    jtok.Advance();

    // optional brackets
    if (jtok.GetToken() == "[") {
        // literal '['
        PrintLiteralSymbol("[", "array dereference");

        // expression
        CompileExpression();

        // literal ']'
        PrintLiteralSymbol("]", "array dereference");
    }

    // literal '='
    PrintLiteralSymbol("=", "let statement");

    // expression
    CompileExpression();

    // literal ';'
    PrintLiteralSymbol(";", "let statement");

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileWhile() {
    const std::string xmlName = "whileStatement";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: 'while' '(' expression ')' '{' statements '}'

    // 'while' -> checked by caller
    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // literal '('
    PrintLiteralSymbol("(", "while statement condition");

    // expression
    CompileExpression();

    // literal ')'
    PrintLiteralSymbol(")", "while statement condition");

    // literal '{'
    PrintLiteralSymbol("{", "while statement body");

    // statements
    while (jtok.GetToken() != "}") {
        CompileStatements();
    }

    // literal '}'
    PrintLiteralSymbol("}", "while statement body");

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileReturn() {
    // syntax: 'return' expression? ';'

    // advance over 'return' -> checked by caller
    jtok.Advance();

    // check for expression
    if (jtok.GetToken() != ";") {
        CompileExpression();
    }

    // literal ';'
    CheckLiteralSymbol(";", "return statement");
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileIf() {
    const std::string xmlName = "ifStatement";

    PrintNodeTag(xmlName, OPENING);
    currIndent.push_back('\t');

    // syntax: 'if' '(' expression ')' '{' statements '}' ('else' '{' statements
    // '}')?

    // 'if' -> checked by caller
    PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // literal '('
    PrintLiteralSymbol("(", "if condition");

    // expression
    CompileExpression();

    // literal ')'
    PrintLiteralSymbol(")", "if condition");

    // literal '{'
    PrintLiteralSymbol("{", "if condition body");

    // expression
    CompileStatements();

    // literal '}'
    PrintLiteralSymbol("}", "if condition body");

    // optional else clause
    if (jtok.GetToken() == "else") {
        // 'else'
        PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
        jtok.Advance();

        // literal '{'
        PrintLiteralSymbol("{", "if condition body");

        // expression
        CompileStatements();

        // literal '}'
        PrintLiteralSymbol("}", "if condition body");
    }

    currIndent.pop_back();
    PrintNodeTag(xmlName, CLOSING);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileSubroutineCall() {
    // syntax: subroutineName '(' expressionList ')' |
    //         (className | varName) '.' subroutineName '(' expressionList ')'

    // subroutineName | (className | varName) -> identifier checked by caller
    std::string className = currClass;
    std::string funcName = "dummy";
    if (jtok.LookaheadToken() == "(") {
        // regular subroutine
        funcName = jtok.GetToken();
    } else {
        // class or variable (class won't be in lookup table)
        const std::string varName = jtok.GetToken();
        if (symTable.Check(varName)) {
            className = symTable.TypeOf(jtok.GetToken());
        } else {
            className = jtok.GetToken();
        }
    }

    jtok.Advance();

    if (jtok.GetToken() == "(") {
        // literal '('
        CheckLiteralSymbol("(", "subroutine call");

        int expressionCount = 0;
        // expressionList
        if (jtok.GetToken() != ")") {
            expressionCount = CompileExpressionList();
        }

        // literal ')'
        CheckLiteralSymbol(")", "subroutine call");

        const auto fname = className + "." + funcName;
        vmWriter.WriteCall(fname, expressionCount);

    } else if (jtok.GetToken() == ".") {
        // literal '.'
        CheckLiteralSymbol(".", "subroutine call");

        // subroutineName -> identifier
        if (jtok.TokenType() != JackTokenizer::IDENTIFIER) {
            const std::string errMsg = "could not parse name in method call";
            compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
        }

        funcName = jtok.GetToken();
        jtok.Advance();

        // literal '('
        CheckLiteralSymbol("(", "subroutine call");

        int expressionCount = 0;
        // expressionList
        if (jtok.GetToken() != ")") {
            expressionCount = CompileExpressionList();
        }

        // literal ')'
        CheckLiteralSymbol(")", "subroutine call");

        const auto fname = className + "." + funcName;
        vmWriter.WriteCall(fname, expressionCount);

    } else {
        const std::string errMsg = "Unexpected symbol in subroutine call";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileExpression() {
    // syntax: term (op term)*

    // term
    auto tokenType = jtok.TokenType();
    if (tokenType == JackTokenizer::INT_CONST ||
        tokenType == JackTokenizer::STRING_CONST ||
        tokenType == JackTokenizer::KEYWORD) {
        CompileTerm();
    } else if (tokenType == JackTokenizer::IDENTIFIER) {
        CompileTerm();
    } else if (jtok.GetToken() == "(") {
        CompileTerm();
    } else if (unaryOpTypes.find(jtok.GetToken()) != unaryOpTypes.end()) {
        CompileTerm();
    } else {
        const std::string errMsg = "Unrecognized token in expression";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    // optional operators and terms
    std::string currOp = "";
    while (expressionTerminals.find(jtok.GetToken()) ==
           expressionTerminals.end()) {
        if (expressionOpTypes.find(jtok.GetToken()) !=
            expressionOpTypes.end()) {
            // operator
            currOp = jtok.GetToken();
            jtok.Advance();
        } else {
            // term
            CompileTerm();

            if (binaryOpCommands.find(currOp) != binaryOpCommands.end()) {
                vmWriter.WriteArithmetic(binaryOpCommands.at(currOp));

            } else {
                const std::string errMsg =
                    "Unrecognized operator \"" + currOp + "\" in expression";
                compilerErrorHandler.Report(currInputFile, jtok.LineNum(),
                                            errMsg);
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileTerm() {
    // syntax: integerConstant | stringConstant | keywordConstant |
    //         varName | varName '[' expression ']' | subroutineCall |
    //         '(' expression ')' | unaryOp term

    auto tokenType = jtok.TokenType();
    if (tokenType == JackTokenizer::INT_CONST) {
        // integerConstant
        vmWriter.WritePush(VMWriter::CONST, std::stoi(jtok.GetToken()));
        jtok.Advance();

    } else if (tokenType == JackTokenizer::STRING_CONST) {
        // stringConstant
        PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
        jtok.Advance();

    } else if (tokenType == JackTokenizer::KEYWORD) {
        // keywordConstant
        if (keywordConstants.find(jtok.GetToken()) == keywordConstants.end()) {
            const std::string errMsg = "Unrecognized keyword in expression";
            compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
        }

        PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
        jtok.Advance();

    } else if (tokenType == JackTokenizer::IDENTIFIER) {
        if (jtok.LookaheadToken() == "[") {
            // array dereference, syntax: varName '[' expression ']'

            // varName
            const std::string varName = jtok.GetToken();
            if (!symTable.Check(varName)) {
                const std::string errMsg =
                    "Variable " + varName + " not defined in current scope";
                compilerErrorHandler.Report(currInputFile, jtok.LineNum(),
                                            errMsg);
            }

            auto kind = symTable.KindOf(varName);
            const Category category = kindMap.at(kind);

            PrintIdentifier(category, USED);

            jtok.Advance();

            // literal '['
            PrintLiteralSymbol("[", "expression term");

            // expression
            CompileExpression();

            // literal ']'
            PrintLiteralSymbol("]", "expression term");

        } else if (jtok.LookaheadToken() == "(" ||
                   jtok.LookaheadToken() == ".") {
            // subroutine call
            CompileSubroutineCall();

        } else {
            // varName
            const std::string varName = jtok.GetToken();
            if (!symTable.Check(varName)) {
                const std::string errMsg =
                    "Variable " + varName + " not defined in current scope";
                compilerErrorHandler.Report(currInputFile, jtok.LineNum(),
                                            errMsg);
            }

            auto kind = symTable.KindOf(varName);
            const Category category = kindMap.at(kind);

            PrintIdentifier(category, USED);

            jtok.Advance();
        }

    } else if (jtok.GetToken() == "(") {
        // '(' expression ')'

        // literal '('
        CheckLiteralSymbol("(", "expression term");

        // expression
        CompileExpression();

        // literal ')'
        CheckLiteralSymbol(")", "expression term");

    } else if (unaryOpTypes.find(jtok.GetToken()) != unaryOpTypes.end()) {
        // unaryOp
        const auto currOp = jtok.GetToken();

        // term
        CompileTerm();

        vmWriter.WriteArithmetic(unaryOpCommands.at(currOp));

    } else {
        // unrecognized input
        const std::string errMsg = "Unrecognized token in expression term";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }
}

/* -------------------------------------------------------------------------- */

int CompilationEngine::CompileExpressionList() {
    // syntax: (expression(',' expression)*)?

    // NOTE: this syntactic element only appears in subroutine calls and
    //       the handling in the caller deals with the empty case.
    //       Additionally, the only terminal in the language rules here is
    //       a literal ')'.

    int expressionCount = 1;

    // expression
    CompileExpression();

    // optional comma-separated expressions
    while (jtok.GetToken() != ")") {
        if (jtok.GetToken() == ",") {
            CheckLiteralSymbol(",", "expression list");
        } else {
            ++expressionCount;
            CompileExpression();
        }
    }

    return expressionCount;
}

/* -------------------------------------------------------------------------- */
