#include "CompilationEngine.h"

#include <cctype>
#include <cstdlib>
#include <iostream>

/* -------------------------------------------------------------------------- */

CompilationEngine::CompilationEngine(const std::string& infileName,
                                     const std::string& outfileName) :
        currInputFile(infileName),
        currClass(),
        outFile(outfileName),
        loopCount(0),
        branchCount(0),
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
    // syntax: ('static' | 'field') type varName (',' varName)* ';'

    // determine variable category
    const Category category = (jtok.GetToken() == "static") ? STATIC : FIELD;

    jtok.Advance();

    CompileVarDecCommon(";", category);

    // terminal semicolon
    if (jtok.GetToken() != ";") {
        const std::string errMsg = "Missing semicolon";
        compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
    }

    jtok.Advance();
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
    const auto funcType = jtok.GetToken();
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
    CompileSubroutineBody(funcName, funcType);

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

void CompilationEngine::CompileSubroutineBody(const std::string& funcName,
                                              const std::string& funcType) {
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

    if (funcType == "method") {
        // make sure we track arg variables with 1-offset
        symTable.SetMethod();
    }

    vmWriter.WriteFunction(vmName, nLocal);

    if (funcType == "method") {
        // need to align "this" segment with hidden arg
        vmWriter.WritePush(VMWriter::ARG, 0);
        vmWriter.WritePop(VMWriter::POINTER, 0);

    } else if (funcType == "constructor") {
        // need to allocate space for the object and store in "this" pointer
        const auto size = symTable.VarCount(SymbolTable::FIELD);
        const std::string allocFunction = "Memory.alloc";

        vmWriter.WritePush(VMWriter::CONST, size);
        vmWriter.WriteCall(allocFunction, 1);

        vmWriter.WritePop(VMWriter::POINTER, 0);
    }

    // statements
    while (jtok.GetToken() != "}") {
        CompileStatements();
    }

    // literal '}'
    CheckLiteralSymbol("}", "subroutine body");
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileVarDec() {
    // const std::string xmlName = "varDec";

    // PrintNodeTag(xmlName, OPENING);
    // currIndent.push_back('\t');

    // syntax: 'var' type varName (',' varName)* ';'

    // 'var', already checked by caller
    // PrintToken(tokenString.at(jtok.TokenType()), jtok.GetToken());
    jtok.Advance();

    // type, etc.
    CompileVarDecCommon(";", VAR);

    // literal ';'
    CheckLiteralSymbol(";", "variable declaration");

    // currIndent.pop_back();
    // PrintNodeTag(xmlName, CLOSING);
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
    // syntax: 'let' varName ('[' expression ']')? '=' expression ';'

    // 'let' -> checked by caller
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

    jtok.Advance();

    bool isArray = false;

    // optional brackets
    if (jtok.GetToken() == "[") {
        isArray = true;

        // literal '['
        CheckLiteralSymbol("[", "array dereference");

        // expression
        CompileExpression();

        // find memory location to store result
        // vmWriter.WritePush(seg, index);
        // vmWriter.WriteArithmetic(VMWriter::ADD);

        // pointer 1 is "that" segment
        // vmWriter.WritePop(VMWriter::TEMP, 1);

        // literal ']'
        CheckLiteralSymbol("]", "array dereference");
    }

    // literal '='
    CheckLiteralSymbol("=", "let statement");

    // expression
    CompileExpression();

    // pop result to selected variable

    auto kind = symTable.KindOf(varName);
    auto index = symTable.IndexOf(varName);
    VMWriter::Segment seg = kindToSegment.at(kind);

    if (isArray) {
        // hold return value
        vmWriter.WritePop(VMWriter::TEMP, 1);

        // current stack element is bracket expression, so add var val
        vmWriter.WritePush(seg, index);
        vmWriter.WriteArithmetic(VMWriter::ADD);

        // move to "that" pointer
        vmWriter.WritePop(VMWriter::POINTER, 1);

        // get value from temp
        vmWriter.WritePush(VMWriter::TEMP, 1);
        vmWriter.WritePop(VMWriter::THAT, 0);

    } else {
        vmWriter.WritePop(seg, index);
    }

    // literal ';'
    CheckLiteralSymbol(";", "let statement");
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileWhile() {
    // syntax: 'while' '(' expression ')' '{' statements '}'

    const auto myLoopCount = loopCount;
    ++loopCount;

    // 'while' -> checked by caller
    jtok.Advance();

    // literal '('
    CheckLiteralSymbol("(", "while statement condition");

    // label loop beginning
    const std::string loopID = loopBase + std::to_string(myLoopCount);
    vmWriter.WriteLabel(loopID);

    // expression
    CompileExpression();

    // check negated expression
    vmWriter.WriteArithmetic(VMWriter::NOT);

    // jump to end if condition fails
    const std::string endLabel = endPrefix + loopID;
    vmWriter.WriteIf(endLabel);

    // literal ')'
    CheckLiteralSymbol(")", "while statement condition");

    // literal '{'
    CheckLiteralSymbol("{", "while statement body");

    // statements
    while (jtok.GetToken() != "}") {
        CompileStatements();
    }

    // jump to top of loop
    vmWriter.WriteGoto(loopID);

    // label loop end
    vmWriter.WriteLabel(endLabel);

    // literal '}'
    CheckLiteralSymbol("}", "while statement body");
}

/* -------------------------------------------------------------------------- */

// NOTE: actual VM return command is appended at the end of the calling
//       function, so this compilation only checks for an expression
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
    // syntax: 'if' '(' expression ')' '{' statements '}' ('else' '{' statements
    // '}')?

    const auto myBranchCount = branchCount;
    ++branchCount;

    // 'if' -> checked by caller
    jtok.Advance();

    // literal '('
    CheckLiteralSymbol("(", "if condition");

    // expression
    CompileExpression();

    // check negated expression
    vmWriter.WriteArithmetic(VMWriter::NOT);

    // jump to end of if branch if condition fails
    const std::string midLabel =
        "MID_" + branchBase + std::to_string(myBranchCount);
    vmWriter.WriteIf(midLabel);

    // literal ')'
    CheckLiteralSymbol(")", "if condition");

    // literal '{'
    CheckLiteralSymbol("{", "if condition body");

    // expression
    CompileStatements();

    // literal '}'
    CheckLiteralSymbol("}", "if condition body");

    // jump to end of all blocks (including else)
    const std::string endLabel =
        endPrefix + branchBase + std::to_string(myBranchCount);
    vmWriter.WriteGoto(endLabel);

    // write label for end of if branch
    vmWriter.WriteLabel(midLabel);

    // optional else clause
    if (jtok.GetToken() == "else") {
        // 'else'
        jtok.Advance();

        // literal '{'
        CheckLiteralSymbol("{", "if condition body");

        // expression
        CompileStatements();

        // literal '}'
        CheckLiteralSymbol("}", "if condition body");
    }

    // label end of all blocks
    vmWriter.WriteLabel(endLabel);
}

/* -------------------------------------------------------------------------- */

void CompilationEngine::CompileSubroutineCall() {
    // syntax: subroutineName '(' expressionList ')' |
    //         (className | varName) '.' subroutineName '(' expressionList ')'

    // subroutineName | (className | varName) -> identifier checked by caller
    std::string className = currClass;
    std::string funcName = "dummy";
    bool isMethod = false;
    if (jtok.LookaheadToken() == "(") {
        // regular subroutine
        funcName = jtok.GetToken();
    } else {
        // class or variable (class won't be in lookup table)
        const std::string varName = jtok.GetToken();
        if (symTable.Check(varName)) {
            // method, need to push "this" segment stored in variable
            className = symTable.TypeOf(varName);
            isMethod = true;

            auto kind = symTable.KindOf(varName);
            auto index = symTable.IndexOf(varName);
            VMWriter::Segment seg = kindToSegment.at(kind);

            // push stored "this" value
            vmWriter.WritePush(seg, index);

        } else {
            className = jtok.GetToken();
        }
    }

    jtok.Advance();

    if (jtok.GetToken() == "(") {
        // literal '('
        CheckLiteralSymbol("(", "subroutine call");

        int expressionCount = 0;

        // naked subroutine calls are method calls by definition, so push "this"
        vmWriter.WritePush(VMWriter::POINTER, 0);

        // expressionList
        if (jtok.GetToken() != ")") {
            expressionCount = CompileExpressionList();
        }

        // factor in "this" pointer
        ++expressionCount;

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
        if (isMethod) ++expressionCount;

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
        // NOTE: JackOS only supports upper-case letters
        const std::string targetString = jtok.GetToken();
        const std::string allocator = "String.new";
        const std::string accumulator = "String.appendChar";

        // first allocate space for full string
        vmWriter.WritePush(VMWriter::CONST, targetString.size());
        vmWriter.WriteCall(allocator, 1);

        // store in "that" pointer
        vmWriter.WritePop(VMWriter::POINTER, 1);

        // copy string
        for (unsigned i = 0; i < targetString.size(); ++i) {
            // set base for String object
            vmWriter.WritePush(VMWriter::POINTER, 1);

            // push ACII representation of next character
            int asciiVal = static_cast<int>(std::toupper(targetString[i]));
            vmWriter.WritePush(VMWriter::CONST, asciiVal);

            // call append method
            vmWriter.WriteCall(accumulator, 2);
            vmWriter.WritePop(VMWriter::POINTER, 1);
        }

        // copy start of string as output
        vmWriter.WritePush(VMWriter::POINTER, 1);

        jtok.Advance();

    } else if (tokenType == JackTokenizer::KEYWORD) {
        // keywordConstant
        if (keywordConstants.find(jtok.GetToken()) == keywordConstants.end()) {
            const std::string errMsg = "Unrecognized keyword in expression";
            compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
        }

        if (jtok.GetToken() == "null" || jtok.GetToken() == "false") {
            // null and false map to constant 0
            vmWriter.WritePush(VMWriter::CONST, 0);

        } else if (jtok.GetToken() == "true") {
            // true maps to -1 (i.e. all '1's in 2's complement binary)
            vmWriter.WritePush(VMWriter::CONST, 1);
            vmWriter.WriteArithmetic(VMWriter::NEG);

        } else {
            // "this" address pointed to by pointer 0
            vmWriter.WritePush(VMWriter::POINTER, 0);
        }

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
            auto seg = kindToSegment.at(kind);
            auto index = symTable.IndexOf(varName);

            // const Category category = kindMap.at(kind);

            // PrintIdentifier(category, USED);

            jtok.Advance();

            // literal '['
            CheckLiteralSymbol("[", "expression term");

            // expression
            CompileExpression();

            vmWriter.WritePush(seg, index);
            vmWriter.WriteArithmetic(VMWriter::ADD);

            // get address of result into "that" pointer
            vmWriter.WritePop(VMWriter::POINTER, 1);

            // push dereferenced value onto the stack
            vmWriter.WritePush(VMWriter::THAT, 0);

            // literal ']'
            CheckLiteralSymbol("]", "expression term");

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
            auto index = symTable.IndexOf(varName);
            VMWriter::Segment seg = kindToSegment.at(kind);

            vmWriter.WritePush(seg, index);

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
        jtok.Advance();

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
