#ifndef VM_WRITER_H
#define VM_WRITER_H

#include <fstream>
#include <map>
#include <string>

class VMWriter {
  public:
    VMWriter(std::ofstream& out);

    // remove unwanted constructors
    VMWriter(const VMWriter& that) = delete;
    VMWriter(const VMWriter&& that) = delete;
    VMWriter& operator=(const VMWriter& that) = delete;
    VMWriter& operator=(const VMWriter&& that) = delete;

    enum Segment { CONST, ARG, LOCAL, STATIC, THIS, THAT, POINTER, TEMP };
    const std::map<Segment, std::string> segNames = {
        {CONST, "constant"},  {ARG, "argument"}, {LOCAL, "local"},
        {STATIC, "static"},   {THIS, "this"},    {THAT, "that"},
        {POINTER, "pointer"}, {TEMP, "temp"}};

    enum Command { ADD, SUB, MULT, DIV, NEG, EQ, GT, LT, AND, OR, NOT };
    const std::map<Command, std::string> commandNames = {
        {ADD, "add"},
        {SUB, "sub"},
        {MULT, "call Math.multiply 2"},
        {DIV, "call Math.divide 2"},
        {NEG, "neg"},
        {EQ, "eq"},
        {GT, "gt"},
        {LT, "lt"},
        {AND, "and"},
        {OR, "or"},
        {NOT, "not"}};

    void WritePush(const Segment segment, const int index);
    void WritePop(const Segment segment, const int index);

    void WriteArithmetic(const Command command);

    void WriteLabel(const std::string& label);
    void WriteGoto(const std::string& label);
    void WriteIf(const std::string& label);

    void WriteCall(const std::string& name, const int nArgs);
    void WriteFunction(const std::string& name, const int nLocals);

    void WriteReturn();

    // data
  private:
    std::ofstream& outFile;
};

#endif /* VM_WRITER_H */
