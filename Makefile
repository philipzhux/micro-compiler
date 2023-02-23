
compiler: scanner.l parser.y
	@bison -d parser.y
	@flex scanner.l
	@g++ lex.yy.c parser.tab.c -lfl -o micro syntax_tree.cpp symbol_table.cpp code.cpp

clean:
	@rm lex.yy.c micro parser.tab.c parser.tab.h