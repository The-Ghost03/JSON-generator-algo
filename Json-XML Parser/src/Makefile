# Makefile pour ProjetAlgo

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -pedantic
INCLUDES:= -I. -Iio -Igraph -Ioptimize -Iscenario -Iutils

SRC := \
  main.c \
  cli.c \
  io/io_utils.c \
  io/parser_json.c \
  io/parser_xml.c \
  graph/graph.c \
  graph/graph_applications.c \
  optimize/optimize.c \
  optimize/scheduler.c \
  scenario/scenario.c \
  utils/logger.c \
  utils/error.c

OBJ := $(SRC:.c=.o)

.PHONY: all clean

all: projetalgo

projetalgo: $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $(OBJ)

# règle générique pour compiler un .c en .o
%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJ) projetalgo
