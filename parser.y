/*
 * Created on Tue Feb 21 2023
 *
 * Copyright (c) 2023 Philip Zhu Chuyan <me@cyzhudev>
 */


%{

#include <stdio.h>
#include <iostream>
#include "added_struct_functions.hpp"
#include <memory>
#include <vector>
#include <string>
#include <cassert>



int yyerror(char *s); 
extern "C" int yylex();
extern "C" FILE* yyin;
extern "C" int yylineno;
extern "C" int yyparse();
MicroCompiler::Code code;

%}

%start system_goal
%token PROGRAM
%token _BEGIN
%token _END
%token _READ
%token _WRITE
%token _LPAREN
%token _RPAREN
%token _SEMICOLON
%token _COMMA
%token _ASSIGNOP
%token _PLUSOP
%token _MINUSOP
%token <nodePtr> _ID
%token <nodePtr> _INTLITERAL
%token _SCANEOF

%type <nodePtr> id expression expression_tail primary int_literal;
%type <nodeVec> id_tail id_list expr_list expr_list_tail;
%type <stringVal>  add_op;


%union {
    MicroCompiler::SyntaxTreeNode* nodePtr;
    std::vector<MicroCompiler::SyntaxTreeNode*>* nodeVec;
    std::string* stringVal;

}



%%
/*LR Grammars and code generating functions*/ 
system_goal : program _SCANEOF
{
    return 0;
};
program: _BEGIN statement_list _END | 
{
    // for(auto &stat: $2) {
    //     stat->generateCode(code);
    // }
};
statement_list: statement statement_list 
{
    // $2->push_back($1);
    // $$ = $2;
}|
{
    // $$ = std::make_unique(std::vector<MicroCompiler::SyntaxTreeNode*>);
};
statement: assign_stat | read_stat | write_stat;
assign_stat: id _ASSIGNOP expression _SEMICOLON
{
    MicroCompiler::SyntaxTreeNode* assignNode = new MicroCompiler::SyntaxTreeNode(MicroCompiler::ASSIGN);
    assignNode->left = $1;
    assignNode->right = $3;
    assert(assignNode->generateCode(code)==MicroCompiler::NORETURN);
};
read_stat: _READ _LPAREN id_list _RPAREN _SEMICOLON
{
    for(auto it = $3->rbegin();it!=$3->rend();it++) {
        MicroCompiler::SymEntry symbol = (*it)->generateCode(code);
        code.sysRead(symbol);
    }
};
write_stat: _WRITE _LPAREN expr_list _RPAREN _SEMICOLON {
    for(auto it = $3->rbegin();it!=$3->rend();it++) {
        MicroCompiler::SymEntry symbol = (*it)->generateCode(code);
        code.sysWrite(symbol);
        }
};
id: _ID
{

};
int_literal: _INTLITERAL {}| _MINUSOP _INTLITERAL {
    $2->intVal*=-1;
    $$ = $2;
};
id_list: id id_tail
{
    $2->push_back($1);
    $$ = $2;
};
id_tail:  _COMMA id id_tail 
{
    $3->push_back($2);
    $$ = $3;
}
| {
    $$ = new std::vector<MicroCompiler::SyntaxTreeNode*>;
};
expr_list: expression expr_list_tail
{
    $2->push_back($1);
    $$ = $2;
};
expr_list_tail:  _COMMA expression expr_list_tail {
    $3->push_back($2);
    $$ = $3;
} | {
    $$ = new std::vector<MicroCompiler::SyntaxTreeNode*>;
};
expression: primary expression_tail {
    if($2==nullptr) 
    {
        $$=$1;
    }
    else
    {
        assert($2->left==nullptr);
        $2->left = $1;
        $$=$2;
    }
};
expression_tail: add_op expression
{
    MicroCompiler::SyntaxTreeNode* newNode = new MicroCompiler::SyntaxTreeNode(MicroCompiler::EXPR,*$1);
    newNode->left = nullptr;
    newNode->right = $2;
    $$ = newNode;
}
|
{
    $$ = nullptr;
};
primary: id
{
    $$ = $1;
}| int_literal
{
    $$ = $1;
} | _LPAREN expression _RPAREN
{
    $$ = $2;
};
add_op: _PLUSOP
{
    $$ = new std::string("add");
} | _MINUSOP {
    $$ = new std::string("sub");
};

%%
/*Implement main function and the functions you want to use*/ 
int yyerror (char *s) { 
    printf ("%s: on Line %d\n", s,yylineno); 
    
    return -1;
    } 

int main(int argc , char const *argv []) {
    if(!yyparse()) 
    {
        for(auto& line: code.getAssembly())
        {
            std::cout<<line<<std::endl;
        }
    }

    return 0;
}
