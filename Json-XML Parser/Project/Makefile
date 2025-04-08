# Makefile
CC = gcc
CFLAGS = -Wall -Wextra -std=c99
OBJS = main.o graph.o parser.o graph_applications.o

all: my_program

my_program: $(OBJS)
	$(CC) $(CFLAGS) -o my_program $(OBJS)

main.o: main.c graph.h parser.h graph_applications.h
	$(CC) $(CFLAGS) -c main.c

graph.o: graph.c graph.h
	$(CC) $(CFLAGS) -c graph.c

parser.o: parser.c parser.h graph.h
	$(CC) $(CFLAGS) -c parser.c

graph_applications.o: graph_applications.c graph_applications.h graph.h
	$(CC) $(CFLAGS) -c graph_applications.c

clean:
	rm -f *.o my_program
