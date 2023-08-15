CC=clang
CFLAGS= -Wall -Werror -Wpedantic -Wextra
LFLAGS=-lm
all: httpserver
httpserver: httpserver.o
	$(CC) -o httpserver httpserver.c asgn2_helper_funcs.a $(LFLAGS)
httpserver.o: httpserver.c asgn2_helper_funcs.h 
	$(CC) $(CFLAGS) -c httpserver.c asgn2_helper_funcs.h
clean:
	rm -f httpserver.o httpserver asgn2_helper_funcs.h.gch
format:
	clang-format -i -style=file *.[ch]

