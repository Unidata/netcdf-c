#!/bin/bash

# These rules are used if someone wants to rebuild ncgenyy.c or ncgentab.c
# Otherwise never invoked, but records how to do it.
# BTW: note that renaming is essential because otherwise
# autoconf will forcibly delete files of the name *.tab.*

flex -Pncg -8 ncgen.l
rm -f ncgenyy.c
sed -e s/lex.ncg.c/ncgenyy.c/g <lex.ncg.c >ncgenyy.c
bison -pncg -t -d ncgen.y
rm -f ncgentab.c ncgentab.h
sed -e s/ncgen.tab.c/ncgentab.c/g -e s/ncgen.tab.h/ncgentab.h/g <ncgen.tab.c >ncgentab.c
sed -e s/ncgen.tab.c/ncgentab.c/g -e s/ncgen.tab.h/ncgentab.h/g <ncgen.tab.h >ncgentab.h
