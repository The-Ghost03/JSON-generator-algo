CC = gcc
CFLAGS = -Wall -Wextra -std=c99
OBJ = main.o graph.o parser.o graph_applications.o optimize.o

my_program.exe: $(OBJ)
	$(CC) $(CFLAGS) -o my_program.exe $(OBJ)

main.o: main.c graph.h parser.h graph_applications.h optimize.h
	$(CC) $(CFLAGS) -c main.c

graph.o: graph.c graph.h
	$(CC) $(CFLAGS) -c graph.c

parser.o: parser.c parser.h graph.h
	$(CC) $(CFLAGS) -c parser.c

graph_applications.o: graph_applications.c graph_applications.h graph.h
	$(CC) $(CFLAGS) -c graph_applications.c

optimize.o: optimize.c optimize.h graph.h
	$(CC) $(CFLAGS) -c optimize.c

clean:
	rm -f *.o my_program.exe
