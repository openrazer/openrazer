LIBS = -lm
MAJOR_VERSION = 0
MINOR_VERSION = 3
CSTD = -std=c99
CFLAGS= -Wall -Wextra -W -w -Os $(CSTD) -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
DEBUG_CFLAGS = -Wall -Wextra -Wformat=0 -pedantic -fbounds-check -Wuninitialized -O0 -g3 $(CSTD) -DUSE_DEBUGGING -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
LDFLAGS= $(LIBS)
DEBUG_LDFLAGS = $(LIBS)
CC=g++

all: snake

snake:
	@echo "\n::\033[32m COMPILING snake game\033[0m"
	@echo "======================"
	$(CC) snake.cpp $(CFLAGS) ../../lib/librazer_chroma.a $(LIBS) -o snake_game
	@strip snake_game
clean: 
	rm -f *.do *.o snake_game

indent:	
	indent -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 *.c *.h
