all: prog4
prog4: prog4.c	
	gcc -Wall -fsanitize=address,undefined -o prog4 prog4.c
.PHONY: clean all
clean:
	rm prog4