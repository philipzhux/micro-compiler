/*
 * Created on Tue Feb 21 2023
 *
 * Copyright (c) 2023 Philip Zhu Chuyan <me@cyzhudev>
 */

#include "symbol_table.hpp"
#include <vector>
#include <unordered_map>
#include <string>

namespace MicroCompiler
{
    class Code;
    class SymbolTable;
    typedef uint32_t SymEntry;
    typedef std::unordered_map<std::string, SymEntry> SymbolMap;
    typedef std::string Register; // to be revised as fine-grained register class

    class Code
    {
    public:
        Code();
        void sym2Reg(SymEntry, Register);
        void int2Reg(int, Register);
        void reg2Sym(Register, SymEntry);
        void sym2Sym(SymEntry, SymEntry);
        void sysRead(SymEntry);
        void sysWrite(SymEntry);
        void sysPrintNewLine();
        void moveStack(int offset);
        std::vector<std::string>& getAssembly();
        void addAsmLine(std::string);
        SymbolTable symbolTable;

    private:
        std::vector<std::string> codeLines;
    };

};
