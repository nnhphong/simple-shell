all: simple-shell.o simple-test.o
	gcc simple-shell.o simple-test.o -o simple-test
simple-shell.o : simple-shell.c byos.h
	gcc -c simple-shell.c	
simple-test.o: simple-test.c byos.h
	gcc -c simple-test.c
.depend: simple-shell.c simple-test.c
	gcc -MM $^ > .depend
include .depend
clean: 
	rm -f *.o byos 
