# CMPUT 379 - Assignment 3
# Costa Zervos

saucer: saucer.c
	gcc saucer.c -o saucer

tar: README questions.txt *.c Makefile
	tar cvf as3.tar design.txt README *.c Makefile

clean: 
	rm -f saucer *.o