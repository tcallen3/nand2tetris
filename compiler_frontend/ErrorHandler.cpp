#include "ErrorHandler.h"

#include <cstdlib>
#include <iostream>

/* -------------------------------------------------------------------------- */

void ErrorHandler::Report(const std::string& file, const int line,
                          const std::string& message) {
    std::cerr << "[file "
              << "\"" << file << "\", line " << line << "] Error: " << message
              << '\n';
    std::exit(65);
}

/* -------------------------------------------------------------------------- */

void ErrorHandler::Report(const std::string& file, const int line,
                          const size_t col, const std::string& message) {
    std::cerr << "[file "
              << "\"" << file << "\", line " << line << ", column " << col
              << "] Error: " << message << '\n';
    std::exit(65);
}

/* -------------------------------------------------------------------------- */
