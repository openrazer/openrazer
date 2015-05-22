LIBS = -lm -lSDL2 -lSDL2_image 
MAJOR_VERSION = 0
MINOR_VERSION = 1
CSTD = -std=c99
CFLAGS= -Wall -Wextra -W -w -Os $(CSTD) -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
DEBUG_CFLAGS = -Wall -Wextra -Wformat=0 -pedantic -fbounds-check -Wuninitialized -O0 -g3 $(CSTD) -DUSE_DEBUGGING -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
#-pedantic -fbounds-check -Wuninitialized -O0 -g3 -Wstrict-prototypes
LDFLAGS= $(LIBS)
DEBUG_LDFLAGS = $(LIBS)
CC=gcc

all: examples_debug examples

examples_debug: 
	@echo "====================================================="
	@echo "::::::::::::::::\033[32m COMPILING examples [DEBUG]\033[0m ::::::::::::::"
	@echo "====================================================="
	$(CC) example1.c $(DEBUG_CFLAGS) $(LIBS) ../lib/librazer_chroma.da -o example1_debug $<
	$(CC) input_example.c $(DEBUG_CFLAGS) $(LIBS) ../lib/librazer_chroma.da -o input_example_debug $<

examples: 
	@echo "====================================================="
	@echo "::::::::::::::::\033[32m COMPILING examples\033[0m ::::::::::::::"
	@echo "====================================================="
	$(CC) example1.c $(CFLAGS) $(LIBS) ../lib/librazer_chroma.a -o example1 $<
	@strip example1
	$(CC) input_example.c $(CFLAGS) $(LIBS) ../lib/librazer_chroma.a -o input_example $<
	@strip input_example

clean: 
	rm -f *.do *.o input_example input_example_debug example1 example1_debug

indent:	
	indent -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 *.c *.h

