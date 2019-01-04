#!/bin/sh
	flex -Pncg -8 ncgen.l
	rm -f ncgenl.c
	sed -e s/lex.ncg.c/ncgenl.c/g <lex.ncg.c >ncgenl.c
	bison -pncg -t -d ncgen.y
	rm -f ncgeny.c ncgeny.h
	sed -e s/ncgen.tab.c/ncgeny.c/g -e s/ncgen.tab.h/ncgeny.h/g <ncgen.tab.c >ncgeny.c
	sed -e s/ncgen.tab.c/ncgeny.c/g -e s/ncgen.tab.h/ncgeny.h/g <ncgen.tab.h >ncgeny.h
