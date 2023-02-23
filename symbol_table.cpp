/*
 * Created on Tue Feb 21 2023
 *
 * Copyright (c) 2023 Philip Zhu Chuyan <me@cyzhudev>
 */

#include "symbol_table.hpp"
#include "code.hpp"
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