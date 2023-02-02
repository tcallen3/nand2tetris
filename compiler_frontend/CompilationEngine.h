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

    void Compile();  // temporary

  private:
    std::string currInputFile;
    std::ofstream outFile;
    JackTokenizer jtok;
    ErrorHandler compilerErrorHandler;

    enum TagType = {OPENING, CLOSING};
};

#endif /* COMPILATION_ENGINE_H */
