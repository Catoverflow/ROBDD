lexer = lex.yy.c
parser = expression.tab.c expression.tab.h
src = ROBDD.cpp

ROBDD: $(lexer) $(parser) $(src)
	g++ -L . $(lexer) $(parser) $(src) -o ROBDD -g
$(lexer): expression.l
	flex expression.l
$(parser): expression.y
	bison expression.y -d

.PHONY: clean
clean:
	rm -f $(lexer) $(parser) ROBDD