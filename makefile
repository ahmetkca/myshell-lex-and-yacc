BISON_FILE	=	myshell_parser.y
FLEX_FILE	=	myshell_lexer.l
EXE			=	myshell

all: $(EXE)

myshell_parser.tab.c myshell_parser.tab.h:	$(BISON_FILE)
	bison -t -v -d $(BISON_FILE)

lex.yy.c:	$(FLEX_FILE) myshell_parser.tab.h
	flex $(FLEX_FILE)

myshell:	lex.yy.c myshell_parser.tab.c myshell_parser.tab.h myshell.c utils.c utils.h myshell.h
	gcc -Wall myshell_parser.tab.c lex.yy.c myshell.c utils.c -o myshell -g

clean:
	rm -f myshell myshell_parser.tab.c lex.yy.c myshell_parser.tab.h myshell_parser.output || true