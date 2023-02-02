#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#include "parser.h"

#include <set>
#include <string>

class CodeWriter {
  public:
    CodeWriter(const std::string& outName);

    // delete unwanted constructors
    CodeWriter(const CodeWriter& that) = delete;
    CodeWriter(const CodeWriter&& that) = delete;
    CodeWriter& operator=(const CodeWriter& that) = delete;
    CodeWriter& operator=(const CodeWriter&& that) = delete;

    void SetFileName(const std::string& fname) { infileName = fname; }
    void WriteArithmetic(const std::string& command);
    void WritePushPop(const Command ptype, const std::string& segment,
                      const int index);

  private:
    int jumpIndex;
    std::ofstream outFile;
    std::string infileName;

    const std::string pushCommand = "push";
    const std::string popCommand = "pop";
    const std::string constSegment = "constant";

    const std::set<std::string> binaryCommands = {"add", "sub", "eq", "gt",
                                                  "lt",  "and", "or"};

    const std::set<std::string> unaryCommands = {"neg", "not"};

    // methods
    void WritePush(const std::string& segment, const int index);
    void WritePop(const std::string& segment, const int index);
    void WriteBinaryOp(const std::string& command);
    void WriteUnaryOp(const std::string& command);
    void WriteOpCommand(const std::string& command);
    void PushRegister(const std::string& reg);
    void PopRegister(const std::string& reg);
    void WriteComparison(const std::string& op);
};

#endif /* CODE_WRITER_H */
