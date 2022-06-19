lexer = lex.yy.c
parser = expression.tab.c expression.tab.h
src = ROBDD.cpp
graphviz_src = res.dot

ROBDD: $(lexer) $(parser) $(src)
	g++ -L . $(lexer) $(parser) $(src) -o ROBDD -g
$(lexer): expression.l
	flex expression.l
$(parser): expression.y
	bison expression.y -d

image: $(graphviz_src)
	dot -Tsvg $(graphviz_src) -o res.svg

.PHONY: clean
clean:
	rm -f $(lexer) $(parser) ROBDD