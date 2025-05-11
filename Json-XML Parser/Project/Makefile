# Makefile

# Compilateur et flags
CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -I.        # on inclut le dossier courant
LDFLAGS :=                               

# Récupère tous les .c du dossier courant
SRCS    := $(wildcard *.c)
# En déduit les .o
OBJS    := $(SRCS:.c=.o)

# Nom de la bibliothèque statique et de l'exécutable
LIB     := libprojet.a

ifeq ($(OS),Windows_NT)
  TARGET := projet_algo.exe
  RM     := del /Q
else
  TARGET := projet_algo
  RM     := rm -f
endif

.PHONY: all clean

all: $(LIB) $(TARGET)

# 1) Création de la lib à partir de tous les .o sauf main.o
$(LIB): $(filter-out main.o,$(OBJS))
	@echo "[AR] $@"
	ar rcs $@ $^

# 2) Edition de lien de l'exécutable : lie main.o avec la lib
$(TARGET): main.o $(LIB)
	@echo "[LD] $@"
	$(CC) $(CFLAGS) $< -L. -lprojet -o $@ $(LDFLAGS)

# 3) Règle générique de compilation
%.o: %.c
	@echo "[CC] $<"
	$(CC) $(CFLAGS) -c $< -o $@

# 4) Nettoyage
clean:
	@echo "[CLEAN]"
	-$(RM) $(OBJS) $(LIB) $(TARGET)
