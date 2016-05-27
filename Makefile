
all: 	
	gcc -o main main.c -fsanitize=address -lpthread -lm

