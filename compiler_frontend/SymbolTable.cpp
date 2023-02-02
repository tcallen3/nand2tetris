#include "SymbolTable.h"

#include <cstdlib>
#include <iostream>

/* -------------------------------------------------------------------------- */

SymbolTable::SymbolTable() :
        staticVarCount(0),
        fieldVarCount(0),
        argVarCount(0),
        plainVarCount(0),
        classTable(),
        subroutineTable() {}

/* -------------------------------------------------------------------------- */

void SymbolTable::StartSubroutine() {
    subroutineTable.clear();
    argVarCount = 0;
    plainVarCount = 0;
}

/* -------------------------------------------------------------------------- */

void SymbolTable::Define(const std::string& name, const std::string& type,
                         const VARKIND kind) {
    if (kind == STATIC || kind == FIELD) {
        // class variable
        if (classTable.find(name) != classTable.end()) {
            std::cerr << "ERROR: class variable " << name
                      << " already defined in this scope\n";
            std::exit(1);
        }

        int index = -1;
        if (kind == STATIC) {
            index = staticVarCount;
            ++staticVarCount;
        } else {
            index = fieldVarCount;
            ++fieldVarCount;
        }

        VarData data(type, kind, index);
        classTable.insert({name, data});
    } else {
        // subroutine variable
        if (subroutineTable.find(name) != subroutineTable.end()) {
            std::cerr << "ERROR: subroutine variable " << name
                      << " already defined in this scope\n";
            std::exit(1);
        }

        int index = -1;
        if (kind == ARG) {
            index = argVarCount;
            ++argVarCount;
        } else {
            index = plainVarCount;
            ++plainVarCount;
        }

        VarData data(type, kind, index);
        subroutineTable.insert({name, data});
    }
}

/* -------------------------------------------------------------------------- */

int SymbolTable::VarCount(const VARKIND kind) const {
    int count = -1;

    if (kind == STATIC) {
        count = staticVarCount;
    } else if (kind == FIELD) {
        count = fieldVarCount;
    } else if (kind == ARG) {
        count = argVarCount;
    } else {
        count = plainVarCount;
    }

    return count;
}

/* -------------------------------------------------------------------------- */

SymbolTable::VARKIND SymbolTable::KindOf(const std::string& name) const {
    VarData data = GetVarData(name);

    return data.kind;
}

/* -------------------------------------------------------------------------- */

std::string SymbolTable::TypeOf(const std::string& name) const {
    VarData data = GetVarData(name);

    return data.type;
}

/* -------------------------------------------------------------------------- */

int SymbolTable::IndexOf(const std::string& name) const {
    VarData data = GetVarData(name);

    return data.index;
}

/* -------------------------------------------------------------------------- */

// non-erroring check for identifier
bool SymbolTable::Check(const std::string& identifier) const {
    bool isPresent = false;

    if (classTable.find(identifier) != classTable.end() ||
        subroutineTable.find(identifier) != subroutineTable.end()) {
        isPresent = true;
    }

    return isPresent;
}

/* -------------------------------------------------------------------------- */

SymbolTable::VarData SymbolTable::GetVarData(const std::string& name) const {
    VarData data;

    if (classTable.find(name) != classTable.end()) {
        data = classTable.at(name);
    } else if (subroutineTable.find(name) != subroutineTable.end()) {
        data = subroutineTable.at(name);
    } else {
        ReportMissingVar(name);
    }

    return data;
}

/* -------------------------------------------------------------------------- */

// NOTE: probably better to include a line number or roll into existing
//       error reporting, but going for simplicity at present
void SymbolTable::ReportMissingVar(const std::string& name) const {
    std::cerr << "ERROR: Variable " << name
              << " not defined in current scope\n";
    std::exit(1);
}

/* -------------------------------------------------------------------------- */
