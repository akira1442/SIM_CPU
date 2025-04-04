# Variables
CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = parser.c hashmap.c MemoryHandler.c
TESTS = $(SRC:.c=_test)
EXEC = sim_cpu

# Règle par défaut
all: $(EXEC) $(TESTS)

# Compilation de l'exécutable principal
$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Compilation des tests pour chaque fichier .c
parsertest: parser.c parser.h hashmap.h testparser.c
	$(CC) $(CFLAGS) -o $@ parser.c hashmap.c testparser.c $^

hashmaptest: hashmap.c hashmap.h
	$(CC) $(CFLAGS) -o $@ hashmap.c $^

MemoryHandlertest: MemoryHandler.c MemoryHandler.h
	$(CC) $(CFLAGS) -o $@ MemoryHandler.c $^

# Nettoyage des fichiers objets et des exécutables
clean:
	rm -f $(EXEC) $(TESTS)

# Nettoyage complet
mrpropre: clean
	rm -f *~
