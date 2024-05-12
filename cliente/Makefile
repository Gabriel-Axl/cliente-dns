
BINFOLDER := bin/

INCFOLDER := inc/

SRCFOLDER := src/

OBJFOLDER := obj/
CC := gcc
CFLAGS := -std=c99 -w
SRCFILES := $(wildcard src/*.c)
dominio:= unb.br
servidorDns:= 8.8.8.8
all: $(SRCFILES:src/%.c=obj/%.o)
	$(CC) $(CFLAGS) obj/*.o -o bin/main $(LINKFLAGS)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I./inc $(LINKFLAGS)

run: bin/main
	bin/main ${dominio} ${servidorDns}

.PHONY: clean
clean:
	rm -rf obj/*.o
	rm -rf bin/main
	touch obj/.gitkeep
	touch bin/.gitkeep