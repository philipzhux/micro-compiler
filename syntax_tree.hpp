/*
 * Created on Tue Feb 21 2023
 *
 * Copyright (c) 2023 Philip Zhu Chuyan <me@cyzhudev>
 */

#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <vector>

namespace MicroCompiler
{
    class Code;
    typedef uint32_t SymEntry;
    enum NodeType {ASSIGN, INTLITERAL, EXPR, ID};

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
