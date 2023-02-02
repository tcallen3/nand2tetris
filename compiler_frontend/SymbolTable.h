#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <map>
#include <string>

class SymbolTable {
  public:
    SymbolTable();

    // remove unwanted constructors
    SymbolTable(const SymbolTable& that) = delete;
    SymbolTable(const SymbolTable&& that) = delete;
    SymbolTable& operator=(const SymbolTable& that) = delete;
    SymbolTable& operator=(const SymbolTable&& that) = delete;

    enum VARKIND { STATIC, FIELD, ARG, VAR };

    struct VarData {
        std::string type;
        VARKIND kind;
        int index;

        VarData() : type(), kind(), index(-1) {}
        VarData(const std::string& t, const VARKIND k, const int i) :
                type(t),
                kind(k),
                index(i) {}
    };

    void StartSubroutine();

    void Define(const std::string& name, const std::string& type,
                const VARKIND kind);
    int VarCount(const VARKIND kind) const;
    VARKIND KindOf(const std::string& name) const;
    std::string TypeOf(const std::string& name) const;
    int IndexOf(const std::string& name) const;
    bool Check(const std::string& name) const;

    // data
  private:
    int staticVarCount;
    int fieldVarCount;
    int argVarCount;
    int plainVarCount;

    std::map<std::string, VarData> classTable;
    std::map<std::string, VarData> subroutineTable;

    // methods
  private:
    VarData GetVarData(const std::string& name) const;
    void ReportMissingVar(const std::string& name) const;
};

#endif /* SYMBOL_TABLE_H */
