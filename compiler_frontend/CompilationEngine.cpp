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

void CompilationEngine::Compile() {
    // write tree root
    outFile << "<tokens>\n";

    const std::map<JackTokenizer::Token, std::string> xmlMap = {
        {JackTokenizer::KEYWORD, "keyword"},
        {JackTokenizer::SYMBOL, "symbol"},
        {JackTokenizer::IDENTIFIER, "identifier"},
        {JackTokenizer::INT_CONST, "integerConstant"},
        {JackTokenizer::STRING_CONST, "stringConstant"}};

    const std::map<std::string, std::string> specialChars = {
        {"<", "&lt;"}, {">", "&gt;"}, {"&", "&amp;"}};

    while (jtok.HasMoreTokens()) {
        auto tokenType = jtok.TokenType();
        if (xmlMap.find(tokenType) == xmlMap.end()) {
            std::string errMsg = "Unexpected token type reported to compiler";
            compilerErrorHandler.Report(currInputFile, jtok.LineNum(), errMsg);
        }

        PrintXMLTag(xmlMap.at(tokenType), OPENING);

        std::string currToken = jtok.GetToken();

        if (specialChars.find(currToken) != specialChars.end()) {
            currToken = specialChars.at(currToken);
        }

        outFile << " " << currToken << " ";

        PrintXMLTag(xmlMap.at(tokenType), CLOSING);
    }

    // close tree root
    outFile << "</tokens>\n";
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
