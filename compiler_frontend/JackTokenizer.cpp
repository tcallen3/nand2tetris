#include "JackTokenizer.h"

#include <cctype>
#include <cstdlib>
#include <iostream>
#include <sstream>

/* -------------------------------------------------------------------------- */

JackTokenizer::JackTokenizer(const std::string& fileName) :
        inFile(fileName),
        fname(fileName),
        currLineNum(0),
        currColumnNum(0),
        currLine(),
        moreTokens(true),
        currToken(),
        currTokenType(),
        currKeyword(),
        currSymbol(),
        currIdentifier(),
        currIntVal(),
        currStringVal(),
        tokErrHandler() {
    if (!inFile.is_open()) {
        std::cerr << "ERROR: Could not open file \"" << fileName << "\"\n";
        std::exit(EXIT_FAILURE);
    }
}

/* -------------------------------------------------------------------------- */

void JackTokenizer::Advance() {
    do {
        AdvanceDriver();
    } while (currTokenType == COMMENT || currTokenType == SPACE);
}

/* -------------------------------------------------------------------------- */

void JackTokenizer::AdvanceDriver() {
    // read new line if necessary and check for EOF
    if (currLine.empty() || currColumnNum >= currLine.size()) {
        ReadNextLine();
        if (!inFile.good()) {
            currTokenType = EMPTY;
            return;
        }
    }

    if (currLine.empty()) {
        currTokenType = SPACE;
        return;
    }

    // skip spaces
    while (currLine[currColumnNum] == ' ' || currLine[currColumnNum] == '\t') {
        ++currColumnNum;

        if (currColumnNum >= currLine.size()) {
            currTokenType = SPACE;
            ReadNextLine();
            return;
        }
    }

    // check for comments
    if (currLine[currColumnNum] == '/' &&
        (currColumnNum + 1) < currLine.size()) {
        if (currLine[currColumnNum + 1] == '/') {
            currTokenType = COMMENT;
            ReadNextLine();
        } else if (currLine[currColumnNum + 1] == '*') {
            currTokenType = COMMENT;
            currColumnNum += 2;
            ParseDelimitedComment();
        }
    }

    // check for symbols
    else if (symbolSet.find(currLine[currColumnNum]) != symbolSet.end()) {
        currToken = currLine[currColumnNum];
        currSymbol = currLine[currColumnNum];
        currTokenType = SYMBOL;
        ++currColumnNum;
    }

    // check for string literals
    else if (currLine[currColumnNum] == '"') {
        ++currColumnNum;
        ParseStringLiteral();
    }

    // check for int literals
    else if (std::isdigit(currLine[currColumnNum])) {
        ParseIntLiteral();
    }

    // parse keywords from identifiers
    else {
        ParseKeywordIdentifier();
    }
}

/* -------------------------------------------------------------------------- */

void JackTokenizer::ReadNextLine() {
    std::getline(inFile, currLine);
    ++currLineNum;
    currColumnNum = 0;

    if (!inFile.good()) {
        moreTokens = false;
    }
}

/* -------------------------------------------------------------------------- */

void JackTokenizer::ParseDelimitedComment() {
    unsigned originalLineNum = currLineNum;

    auto commentEnd = currLine.find(multiCommentCloser, currColumnNum);
    while (commentEnd == std::string::npos) {
        ReadNextLine();
        ++currLineNum;
        currColumnNum = 0;

        // unexpected EOF
        if (!inFile.good()) {
            const std::string errMsg = "Unclosed multi-line comment";
            tokErrHandler.Report(fname, originalLineNum, errMsg);
        }

        commentEnd = currLine.find(multiCommentCloser, currColumnNum);
    }

    currTokenType = COMMENT;
    currColumnNum = commentEnd + 2;
}

/* -------------------------------------------------------------------------- */

void JackTokenizer::ParseStringLiteral() {
    size_t startPos = currColumnNum;
    size_t endQuotePos = currLine.find('"', currColumnNum);

    if (endQuotePos == std::string::npos) {
        const std::string errMsg = "Unclosed string literal";
        tokErrHandler.Report(fname, currLineNum, errMsg);
    }

    currStringVal = currLine.substr(startPos, endQuotePos - startPos);
    currToken = currStringVal;
    currTokenType = STRING_CONST;

    currColumnNum = endQuotePos + 1;
}

/* -------------------------------------------------------------------------- */

void JackTokenizer::ParseIntLiteral() {
    std::stringstream intStream;

    while (std::isdigit(currLine[currColumnNum])) {
        intStream << currLine[currColumnNum];
        ++currColumnNum;

        if (currColumnNum >= currLine.size()) {
            const std::string errMsg = "Missing semicolon after int literal";
            tokErrHandler.Report(fname, currLineNum, currColumnNum + 1, errMsg);
        }
    }

    currToken = intStream.str();
    currTokenType = INT_CONST;
    currIntVal = std::stoi(intStream.str());
}

/* -------------------------------------------------------------------------- */

void JackTokenizer::ParseKeywordIdentifier() {
    std::stringstream textStream;

    while (std::isalnum(currLine[currColumnNum]) ||
           currLine[currColumnNum] == '_') {
        textStream << currLine[currColumnNum];
        ++currColumnNum;

        if (currColumnNum >= currLine.size()) {
            const std::string errMsg = "Missing semicolon";
            tokErrHandler.Report(fname, currLineNum, currColumnNum, errMsg);
        }
    }

    currToken = textStream.str();

    if (keywordMap.find(currToken) != keywordMap.end()) {
        currTokenType = KEYWORD;
        currKeyword = keywordMap.at(currToken);

    } else {
        currTokenType = IDENTIFIER;
        currIdentifier = currToken;
    }
}

/* -------------------------------------------------------------------------- */
