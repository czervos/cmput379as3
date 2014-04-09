# CMPUT 379 - Assignment 3
# Costa Zervos

saucer: saucer.c
	gcc saucer.c -lcurses -lpthread -o saucer

tar: README questions.txt *.c Makefile
	tar cvf as3.tar design.txt README.md *.c Makefile

clean: 
	rm -f saucer *.o