all: cwiczenie
cwiczenie: cwiczenie.c	
	gcc -Wall -fsanitize=address,undefined -o cwiczenie cwiczenie.c
.PHONY: clean all
clean:
	rm cwiczenie