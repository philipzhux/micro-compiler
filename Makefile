
compiler: scanner.l parser.y
	@bison -d parser.y
	@flex scanner.l
	@g++ lex.yy.c parser.tab.c -lfl -o micro added_struct_functions.cpp

clean:
	@rm lex.yy.c micro parser.tab.c parser.tab.h