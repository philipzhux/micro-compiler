# Micro Compiler Constructed with Flex and Bison

First course project for CSC4180: Compiler Constuction @ CUHK.

## Code Structure

* `namespace MicroCompiler`
    * `class SymbolTable` *(symbol_table.cpp/hpp)*
    * `class SyntaxTreeNode` *(syntax_tree.cpp/hpp)*
    * `class Code` *(code.cpp/hpp)*, entry point for parser


## How do I design the Scanner?

The scanner is designed to recognized the tokens as per the definitions of MICRO language expressed in RegEx. Special cases include `INTLITERAL` and `ID` token, whose token texts are meaningful. Here instead of passing a string or a integer to yylval, I created an ad-hoc `SyntaxTreeNode` object (a self-defined class further discussed in parser and code-gen part) initialized with the string or integer and pass the pointer of the `SyntaxTreeNode` to yyval.

For example:

```C++
//for an ID ([a-zA-Z][-_a-zA-Z0-9]* )
yylval.nodePtr = new MicroCompiler::SyntaxTreeNode(MicroCompiler::ID,std::string(yytext));

//for an (positive) INTLITERAL ([1-9][0-9]* )
yylval.nodePtr = new MicroCompiler::SyntaxTreeNode(MicroCompiler::INTLITERAL,atoi(yytext));

```

## How do I design the Parser?

The implementation of parser is basically translating the extended CFG of Micro Language:
```js
1. <program> → BEGIN <statement list> END
2. <statement list> → <statement> {<statement>}
3. <statement> → ID ASSIGNOP <expression>;
4. <statement> → READ LPAREN <id list> RPAREN;
5. <statement> → WRITE LPAREN<expr list> RPAREN;
6. <id list > → ID {COMMA ID}
7. <expr list > → <expression> {COMMA <expression>} 8. <expression> → <primary> {<add op> <primary>}
9. <primary> → LPAREN <expression> RPAREN
10. <primary> → ID
11. <primary> → INTLITERAL
12. <add op> → PLUOP
13. <add op> → MINUSOP
14. <system goal> → <program> SCANEOF
```

 *Note: there is a correction in (5): \<id list> -changeInto-> \<expr list>*

For the grammar with extended notation like `{<statement>}`, it needs some modifications. For example, I created two non-terminal symbols for `<id list>`: `id_list` and `id_list_tail`:

```yacc
id_list: id id_list_tail;
id_list_tail:  _COMMA id id_list_tail | ;
```

Similar pattern are also applied in `expr_list`.

It is also noteworthy the `INTLITERAL` provided by the scanner is only positive numbers (the RegEx represents a string of digits). Therefore, there is an extra rule of a non-terminal symbol `intliteral`:
```yacc
int_literal: _INTLITERAL | _MINUSOP _INTLITERAL;
```

## How the code is generated?

My implementation highlights one central concept I adopt: **everything is a SyntaxTreeNode**. For primary like ``ID`` and ``INTLITERAL``, the corresponding ``SyntaxTreeNode`` is already created by the scanner. For types like expression and operation, they would be a node with left children and right children, and the leaves of the syntax tree must be primaries.

```C++
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
```

Note that every tree node has a method `generateCode`, which returns a `SymEntry`. A `SymEntry` is a reference to a symbol on the symbol (for **identifier** the symbol is permanent, and for intermediate result, the symbol is temporary) table, which in fact is a reference to a memory address on stack. Upon calling `generateCode`, the result of the node will then reside in the corresponding memory address. In such way, each statement in the Micro source file is a `Syntax Tree` and the whole Micro source code is a `Syntax Forest`. The `generateCode` is called on the root of each tree and triggers a recursive call on all nodes, and code is then generated recursively.

The core idea on `generateCode` depends on the type of the node (the code snippet shown below is already partially de-capsulate from the original code):

* INTLITERAL: For intliteral node we have the intliteral in data field `intVal`, the assembly is generated to:

    * put intVal to a temp register:

        ```C++
        addAsmLine(::stringFormat("li %s,%d", reg.c_str(), intLiteral));
        ```

    * create a temporary symbol (including allocating memory in stack):

        ```C++
        SymEntry tempSym = ++currIdx;
        addAsmLine(::stringFormat("addi $sp,$sp,%d", -4));
        ```

    * move the value in temp register to memory (tempSym) and return the reference (index) of the temp symbol:

        ```C++
        addAsmLine(::stringFormat("sw %s,%d($fp)", reg.c_str(), int(tempSym) * -4));
        return tempSym;
        ```
* ASSIGN:
    ```C++
    SymEntry rightSym = right->generateCode(code);
    SymEntry leftSym = left->generateCode(code);
    code.sym2Sym(leftSym, rightSym); // code::sym2Sym(dest,src)
    return MicroCompiler::NORETURN;
    ```
* EXPR: Operation expression like add op or minus op, also call `generateCode` on both right and left nodes, then add code to i.move content of result symbols from memory to register ii. conduct the operation (add/sub) iii. put result from register to a temporary symbol and return the symbol

* ID: if ID key not in symbol table, create a new (permanent) symbol (i.e. generate code to: allocate mem for the symbol, initialize to zero), add (ID-key, symbolIndex) into the symbol table and return the symbol reference (index).

Since every intermediate results may claim temporary symbols and occupy space in stack memory, I also implemented logic to free temporary symbols when they are no longer needed and put them into free pool open for reuse by new temp symbols. Therefore, even if the height of syntax tree is enormous, only a few temporary symbols may be used.



## Possible Optimizations

Only around three registers are used and there are a lot of stores and loads back and forth from the memory and the register flying around, because all intermediate results are returned as symbol references, which means they have to reside in memory. To optimize, I can decouple the symbol object with memory, in this way a symbol (permanent/temporary) can be either in a register or in memory depending on the availability. Most of the changes only resides in the implementation of the SymEntry type in my code (now SymEntry is an alias to `uint32_t` because it is strongly coupled with relative memory address), so incremental changes is possible without changing most of the code.