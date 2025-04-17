# Makefile racine (à placer dans Project/Makefile)

# Compilateur et flags
CC       := gcc
CFLAGS   := -I./src -Wall -Wextra -g
LDFLAGS  :=

# Répertoires
SRC_DIR    := src
BIN_DIR    := bin
TEST_DIR   := $(SRC_DIR)/tests

# Liste des .c à compiler pour le projet
SRC_FILES := \
  $(SRC_DIR)/main.c \
  $(SRC_DIR)/cli.c \
  $(wildcard $(SRC_DIR)/io/*.c) \
  $(wildcard $(SRC_DIR)/graph/*.c) \
  $(wildcard $(SRC_DIR)/optimize/*.c) \
  $(wildcard $(SRC_DIR)/scenario/*.c) \
  $(wildcard $(SRC_DIR)/utils/*.c)

OBJ_FILES := $(SRC_FILES:.c=.o)

# Tests unitaires
TEST_SRC_FILES := $(wildcard $(TEST_DIR)/test_*.c)
TEST_OBJ_FILES := $(TEST_SRC_FILES:.c=.o)
TEST_BIN        := $(BIN_DIR)/run_tests

# Exécutable principal
TARGET := $(BIN_DIR)/projetalgo

.PHONY: all clean test

# --- Règles principales ---

all: $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Construction de l'exécutable principal
$(TARGET): | $(BIN_DIR) $(OBJ_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ_FILES)

# Compilation des .o pour le projet
$(SRC_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Tests ---

test: $(TEST_BIN)
	./$(TEST_BIN)

# Construction de l'exécutable de tests
$(TEST_BIN): | $(BIN_DIR) $(OBJ_FILES) $(TEST_OBJ_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(OBJ_FILES) $(TEST_OBJ_FILES)

# Compilation des .o pour les tests
$(TEST_DIR)/%.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Nettoyage ---

clean:
	rm -rf $(BIN_DIR) \
	       $(OBJ_FILES) \
	       $(TEST_OBJ_FILES)
