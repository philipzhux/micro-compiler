/*
 * Created on Tue Feb 21 2023
 *
 * Copyright (c) 2023 Philip Zhu Chuyan <me@cyzhudev>
 */
%{
/*C++ Stuff(headers, declarations , variables , etc.)*/ 
#include <stdio.h>
#include <memory>
#include "added_struct_functions.hpp"
#include "parser.tab.h"
extern "C" int yylex();
%}

/*Options*/
%option yylineno
%option noyywrap

/*Regular Expressions*/ 
integer ([0−9])+
id [a-zA-Z][a-zA-Z0-9]*
comment --.*\n

/*Matching Rules*/
%%
begin {return _BEGIN;} 
end {return _END;}
read {return _READ;}
write {return _WRITE;}
\( {return _LPAREN;}
\) {return _RPAREN;}
; {return _SEMICOLON;}
, {return _COMMA;}
:= {return _ASSIGNOP;}
\+ {return _PLUSOP;}
- {return _MINUSOP;}
[ \t\n] {}
[a-zA-Z][-_a-zA-Z0-9]* {
    yylval.nodePtr = new MicroCompiler::SyntaxTreeNode(MicroCompiler::ID,std::string(yytext));
    // printf("Catch ID %s\n",yytext);
    return _ID;
}
[1-9][0-9]* {
    yylval.nodePtr = new MicroCompiler::SyntaxTreeNode(MicroCompiler::INTLITERAL,atoi(yytext));
    // printf("Catch int %s\n",yytext);
    return _INTLITERAL;
}
{comment} {}
<<EOF>> return _SCANEOF;
%%
/*Implement the functions you want to use*/ int yywrap  = 1;
