#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#include "parser.h"

#include <map>
#include <set>
#include <stack>
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
    void WriteInit();
    void WriteArithmetic(const std::string& command);
    void WritePushPop(const Command ptype, const std::string& segment,
                      const int index);
    void WriteLabel(const std::string& label);
    void WriteGoto(const std::string& label);
    void WriteIf(const std::string& label);
    void WriteCall(const std::string& functionName, int nArgs);
    void WriteFunction(const std::string& functionName, int nLocals);
    void WriteReturn();

  private:
    int jumpIndex;
    int returnIndex;
    std::ofstream outFile;
    std::string infileName;
    std::stack<std::string> currFunction;

    const std::string pushCommand = "push";
    const std::string popCommand = "pop";

    const std::string constSegment = "constant";
    const std::string tempSegment = "temp";
    const std::string pointerSegment = "pointer";
    const std::string staticSegment = "static";

    const int tempBase = 5;
    const int tempMaxOffset = 7;
    const int pointerBase = 3;
    const int pointerMaxOffset = 1;

    const std::set<std::string> binaryCommands = {"add", "sub", "eq", "gt",
                                                  "lt",  "and", "or"};

    const std::set<std::string> unaryCommands = {"neg", "not"};

    const std::map<std::string, std::string> regMap = {{"local", "LCL"},
                                                       {"argument", "ARG"},
                                                       {"this", "THIS"},
                                                       {"that", "THAT"}};

    // methods
    void WritePush(const std::string& segment, const int index);
    void PushFixed(const std::string& segment, const int index, const int base,
                   const int maxOffset);
    void WritePop(const std::string& segment, const int index);
    void PopFixed(const std::string& segment, const int index, const int base,
                  const int maxOffset);
    void WriteBinaryOp(const std::string& command);
    void WriteUnaryOp(const std::string& command);
    void WriteOpCommand(const std::string& command);
    void PushRegister(const std::string& reg);
    void PopRegister(const std::string& reg);
    void WriteComparison(const std::string& op);
};

#endif /* CODE_WRITER_H */
