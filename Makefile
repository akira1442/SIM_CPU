# Variables
CC = gcc
CFLAGS = -Wall -Wextra -g
SRC = parser.c hashmap.c MemoryHandler.c
EXEC = sim_cpu
TESTS = parsertest hashmaptest MemoryHandlertest

# Règle par défaut
all: $(EXEC) $(TESTS)

# Compilation de l'exécutable principal
$(EXEC): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^

# Compilation des tests
parsertest: parser.c hashmap.c testparser.c
	$(CC) $(CFLAGS) -o $@ parser.c hashmap.c testparser.c

hashmaptest: hashmap.c hashmap.h
	$(CC) $(CFLAGS) -o $@ hashmap.c

MemoryHandlertest: MemoryHandler.c MemoryHandler.h
	$(CC) $(CFLAGS) -o $@ MemoryHandler.c

cpu_main: cpu.c parser.c hashmap.c MemoryHandler.c
	$(CC) $(CFLAGS) -o $@ cpu.c parser.c hashmap.c MemoryHandler.c

# Nettoyage des fichiers objets et des exécutables
clean:
	rm -f $(EXEC) $(TESTS)

# Nettoyage complet
mrpropre: clean
	rm -f *~