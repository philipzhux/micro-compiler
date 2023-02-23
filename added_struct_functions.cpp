/*
 * Created on Tue Feb 21 2023
 *
 * Copyright (c) 2023 Philip Zhu Chuyan <me@cyzhudev>
 */

#include "added_struct_functions.hpp"
#include <memory>
#include <assert.h>
#include <stdexcept>

template <typename... Args>
std::string stringFormat(const std::string &format, Args... args)
{
    int size_s = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    if (size_s <= 0)
    {
        throw std::runtime_error("Error during formatting.");
    }
    auto size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

MicroCompiler::SymbolTable::SymbolTable(MicroCompiler::Code* _code)
{
    currIdx = 0;
    code = _code;
}

MicroCompiler::SymEntry MicroCompiler::SymbolTable::getSymbol(std::string symbol)
{
    return map[symbol];
}

MicroCompiler::SymEntry MicroCompiler::SymbolTable::declareSymbol(std::string symbol)
{
    map[symbol] = currIdx++;
    code->moveStack(4);
    return map[symbol];
}

void MicroCompiler::SymbolTable::freeTempSymbol(MicroCompiler::SymEntry symbolIdx)
{
    freedTempSyms.insert(symbolIdx);
}

MicroCompiler::SymEntry MicroCompiler::SymbolTable::declareTempSymbol()
{
    SymEntry ret;
    if (freedTempSyms.empty())
    {
        ret = currIdx++;
        code->moveStack(4);
        tempSet.insert(ret);
    }
    else
    {
        ret = *freedTempSyms.begin();
        freedTempSyms.erase(ret);
    }

    return ret;
}

bool MicroCompiler::SymbolTable::isSymbolExist(std::string symbol)
{
    return map.count(symbol) > 0;
}

bool MicroCompiler::SymbolTable::isEntTemp(MicroCompiler::SymEntry ent)
{
    return tempSet.count(ent) > 0;
}

// void MicroCompiler::SymbolTable::printSymbolTable()
// {
//     for (auto &it : map)
//     {

//         printf("%s\t|\t", it.first.c_str(), );
//         if (it.second.inReg)
//         {
//             printf("in_stack\t|\t%s\n",it.second.reg);
//         }
//         else
//         {
//             printf("in_memory\t|\t");
//         }

//     }
// }

MicroCompiler::Code::Code(): symbolTable(this)
{
    codeLines.push_back("move $fp,$sp"); // initialize $fp to $sp, $fp being start of stack
}

std::vector<std::string> &MicroCompiler::Code::getAssembly()
{
    return codeLines;
}

void MicroCompiler::Code::reg2Sym(MicroCompiler::Register reg, MicroCompiler::SymEntry symbolIdx)
{
    codeLines.push_back(::stringFormat("sw %s,%d($fp)", reg.c_str(), int(symbolIdx) * 4));
}

void MicroCompiler::Code::sym2Reg(MicroCompiler::SymEntry symbolIdx, MicroCompiler::Register reg)
{
    codeLines.push_back(::stringFormat("lw %s,%d($fp)", reg.c_str(), int(symbolIdx) * 4));
}

void MicroCompiler::Code::sym2Sym(MicroCompiler::SymEntry destSymIdx, MicroCompiler::SymEntry srcSymIdx)
{
    Register t1("$t1");
    sym2Reg(srcSymIdx, t1);
    reg2Sym(t1, destSymIdx);
}

void MicroCompiler::Code::int2Reg(int intLiteral, MicroCompiler::Register reg)
{
    addAsmLine(::stringFormat("li %s,%d", reg.c_str(), intLiteral));
}

void MicroCompiler::Code::addAsmLine(std::string line)
{
    codeLines.push_back(line);
}

void MicroCompiler::Code::moveStack(int offset)
{
    addAsmLine(::stringFormat("addi $sp,$sp,%d", offset));
}

void MicroCompiler::Code::sysRead(MicroCompiler::SymEntry symbolIdx)
{   
    addAsmLine("li $v0,5");
    addAsmLine("syscall");
    reg2Sym(Register("$v0"),symbolIdx);
}

void MicroCompiler::Code::sysWrite(MicroCompiler::SymEntry symbolIdx)
{   
    addAsmLine("li $v0,1");
    sym2Reg(symbolIdx, Register("$a0"));
    addAsmLine("syscall");
}

MicroCompiler::SyntaxTreeNode::SyntaxTreeNode(MicroCompiler::NodeType _nodeType) : nodeType(_nodeType) {}

MicroCompiler::SyntaxTreeNode::SyntaxTreeNode(MicroCompiler::NodeType _nodeType, std::string stringVal) : nodeType(_nodeType) 
{
    switch(nodeType) {
        case ID:
            id = stringVal;
            break;
        case EXPR:
            op = stringVal;
            break;
        default:
            break;
    }
}
MicroCompiler::SyntaxTreeNode::SyntaxTreeNode(MicroCompiler::NodeType _nodeType, int _intVal) : nodeType(_nodeType) {
    intVal = _intVal;
}

MicroCompiler::SymEntry MicroCompiler::SyntaxTreeNode::generateCode(MicroCompiler::Code &code)
{
    Register reg1("$t1"), reg2("$t2"), reg3("$t3");
    switch (nodeType)
    {
    case INTLITERAL:
    {
        code.int2Reg(intVal, reg1);
        SymEntry temp = code.symbolTable.declareTempSymbol();
        code.reg2Sym(Register(reg1), temp);
        return temp;
    }
    case ASSIGN:
    {
        SymEntry leftSym = left->generateCode(code);
        SymEntry rightSym = right->generateCode(code);
        code.sym2Sym(leftSym, rightSym);
        if (code.symbolTable.isEntTemp(leftSym))
            code.symbolTable.freeTempSymbol(leftSym);
        if (code.symbolTable.isEntTemp(rightSym))
            code.symbolTable.freeTempSymbol(rightSym);
        return MicroCompiler::NORETURN;
    }
    case EXPR:
    {
        if(left==nullptr) {
            assert(right!=nullptr);
            return right->generateCode(code);
        }
        if(right==nullptr) {
            assert(left!=nullptr);
            return left->generateCode(code);
        }
        SymEntry leftSym = left->generateCode(code);
        SymEntry rightSym = right->generateCode(code);
        code.sym2Reg(leftSym, reg2);
        code.sym2Reg(rightSym, reg3);
        if (code.symbolTable.isEntTemp(leftSym))
            code.symbolTable.freeTempSymbol(leftSym);
        if (code.symbolTable.isEntTemp(rightSym))
            code.symbolTable.freeTempSymbol(rightSym);
        code.addAsmLine(::stringFormat("%s %s,%s,%s", op.c_str(), reg1.c_str(), reg2.c_str(), reg3.c_str()));
        SymEntry temp = code.symbolTable.declareTempSymbol();
        code.reg2Sym(reg1, temp);
        return temp;
    }
    case ID:
        {
        if (code.symbolTable.isSymbolExist(id))
        {
            return code.symbolTable.getSymbol(id);
        }
        SymEntry newSymbolIdx = code.symbolTable.declareSymbol(id);
        code.reg2Sym(Register("$zero"), newSymbolIdx);
        return newSymbolIdx;
        }

    default:
        break;
    }
    return NORETURN;
}



