# CMPUT 379 - Assignment 3
# Costa Zervos

all: saucer
.PHONY: all

saucer: saucer.c
	gcc saucer.c -lcurses -lpthread -o saucer

tar: README.md design.txt *.c Makefile
	tar cvf as3.tar design.txt README.md *.c Makefile

clean: 
	rm -f saucer *.o