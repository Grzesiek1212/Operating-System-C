all: prog9
prog9: prog9.c	
	gcc -Wall -fsanitize=address,undefined -o prog9 prog9.c
.PHONY: clean all
clean:
	rm prog9