/*
 * Created on Tue Feb 21 2023
 *
 * Copyright (c) 2023 Philip Zhu Chuyan <me@cyzhudev>
 */

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>
#include <stack>

namespace MicroCompiler
{
    class Code;
    typedef uint32_t SymEntry;
    typedef std::unordered_map<std::string, SymEntry> SymbolMap;
    typedef std::string Register; // to be revised as fine-grained register class
    enum NodeType {ASSIGN, INTLITERAL, EXPR, ID};
    static const SymEntry NORETURN = SymEntry(-1);
    class SymbolTable
    {
    public:
        SymbolTable(Code* code);
        SymEntry getSymbol(std::string);
        SymEntry declareSymbol(std::string);
        SymEntry declareTempSymbol();
        void freeTempSymbol(SymEntry);
        void printSymbolTable();
        bool isSymbolExist(std::string);
        bool isEntTemp(SymEntry);

    private:
        SymbolMap map;
        std::unordered_set<SymEntry> tempSet;
        std::unordered_set<SymEntry> freedTempSyms;
        SymEntry currIdx;
        Code* code;
    };

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
        void moveStack(int offset);
        std::vector<std::string>& getAssembly();
        void addAsmLine(std::string);
        SymbolTable symbolTable;

    private:
        std::vector<std::string> codeLines;
    };

    class SyntaxTreeNode
    {
        public:
        SyntaxTreeNode() = delete;
        SyntaxTreeNode(NodeType, std::string); // for expr and id
        SyntaxTreeNode(NodeType, int);
        SyntaxTreeNode(NodeType);
        NodeType nodeType; 
        std::string id; // symbol key for identifiers
        int intVal; // value for int literals
        std::string op; // for add or sub
        SymEntry generateCode(Code& code);
        SyntaxTreeNode* left; 
        SyntaxTreeNode* right;
    };



};
