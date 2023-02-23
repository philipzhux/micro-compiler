/*
 * Created on Tue Feb 21 2023
 *
 * Copyright (c) 2023 Philip Zhu Chuyan <me@cyzhudev>
 */

#include "code.hpp"
#include "utils.hpp"

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