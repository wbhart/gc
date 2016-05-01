PREFIX=/home/wbhart
INC=
LIB=
OBJS=gc.o
HEADERS=gc.h
CS_FLAGS=-O2 -g -D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS
cs: cs.c $(HEADERS) $(OBJS)
	g++ $(CS_FLAGS) $(INC) cs.c $(OBJS) $(LIB) `$(PREFIX)/bin/llvm-config --libs --cflags --ldflags core analysis executionengine mcjit interpreter native` -o cs -ldl -lpthread -lreadline -lncurses -lz

gc.o: gc.c $(HEADERS)
	gcc $(CS_FLAGS) -c gc.c -o gc.o $(INC)

clean:
	rm -f *.o
	rm -f cs

check: cs
	./cs

.PHONY: doc clean check

