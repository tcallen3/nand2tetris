#ifndef CODE_WRITER_H
#define CODE_WRITER_H

#include "parser.h"

#include <string>

class CodeWriter {
    public:
        CodeWriter(const std::string & outName);
        // delete unwanted constructors

        void SetFileName(const std::string & fname) { infileName = fname; }
        void WriteArithmetic(const std::string & command);
        void WritePushPop(const Command ptype, const std::string & segment,
            const int index);
        void Close();

    private:
        std::ofstream outFile;
        std::string infileName;
};

#endif /* CODE_WRITER_H */
