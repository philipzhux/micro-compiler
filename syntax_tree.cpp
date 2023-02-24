/*
 * Created on Tue Feb 21 2023
 *
 * Copyright (c) 2023 Philip Zhu Chuyan <me@cyzhudev>
 */

#include <assert.h>
#include "syntax_tree.hpp"
#include "code.hpp"
#include "utils.hpp"

MicroCompiler::SyntaxTreeNode::SyntaxTreeNode(MicroCompiler::NodeType _nodeType) : nodeType(_nodeType) {}

MicroCompiler::SyntaxTreeNode::SyntaxTreeNode(MicroCompiler::NodeType _nodeType, std::string stringVal) : nodeType(_nodeType)
{
    switch (nodeType)
    {
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
MicroCompiler::SyntaxTreeNode::SyntaxTreeNode(MicroCompiler::NodeType _nodeType, int _intVal) : nodeType(_nodeType)
{
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
        SymEntry rightSym = right->generateCode(code);
        SymEntry leftSym = left->generateCode(code);
        code.sym2Sym(leftSym, rightSym); // code::sym2Sym(dest,src)
        if (code.symbolTable.isEntTemp(leftSym))
            code.symbolTable.freeTempSymbol(leftSym);
        if (code.symbolTable.isEntTemp(rightSym))
            code.symbolTable.freeTempSymbol(rightSym);
        return MicroCompiler::NORETURN;
    }
    case EXPR:
    {
        if (left == nullptr)
        {
            assert(right != nullptr);
            return right->generateCode(code);
        }
        if (right == nullptr)
        {
            assert(left != nullptr);
            return left->generateCode(code);
        }
        SymEntry rightSym = right->generateCode(code);
        SymEntry leftSym = left->generateCode(code);
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
