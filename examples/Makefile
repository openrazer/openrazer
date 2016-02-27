LIBS = -lm
# -lSDL2 -lSDL2_image
MAJOR_VERSION = 0
MINOR_VERSION = 3
CSTD = -std=c99
CFLAGS= -Wall -Wextra -W -w -Os $(CSTD) -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
DEBUG_CFLAGS = -Wall -Wextra -Wformat=0 -pedantic -fbounds-check -Wuninitialized -O0 -g3 $(CSTD) -DUSE_DEBUGGING -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
#-pedantic -fbounds-check -Wuninitialized -O0 -g3 -Wstrict-prototypes
LDFLAGS= $(LIBS)
DEBUG_LDFLAGS = $(LIBS)
CC=gcc

LIBDBUS=$(shell pkg-config --libs dbus-1 > /dev/null 2>&1 && echo 0)

ifeq ($(LIBDBUS), 0)
	LIBS+= `pkg-config --libs dbus-1`
	CFLAGS+= `pkg-config --cflags dbus-1` -DUSE_DBUS
	DEBUG_CFLAGS+= `pkg-config --cflags dbus-1` -DUSE_DBUS
endif


all: examples_debug examples snake dynamic

examples_debug:
	@echo "\n::\033[32m COMPILING examples [DEBUG]\033[0m"
	@echo "=============================="
	$(CC) example1.c $(DEBUG_CFLAGS) ../lib/librazer_chroma.da $(LIBS) -o example1_debug
	$(CC) raw_example.c $(DEBUG_CFLAGS) $(LIBS) -o raw_example_debug
	$(CC) input_example.c $(DEBUG_CFLAGS) ../lib/librazer_chroma.da $(LIBS) -o input_example_debug
	$(CC) input_map.c $(DEBUG_CFLAGS) ../lib/librazer_chroma.da $(LIBS) -o input_map_debug
	$(CC) dota_keys.c $(DEBUG_CFLAGS) ../lib/librazer_chroma.da $(LIBS) -o dota_keys_debug
	$(CC) controller_example.c $(DEBUG_CFLAGS) ../lib/librazer_chroma_controller.da $(LIBS) -o controller_example_debug

examples:
	@echo "\n::\033[32m COMPILING examples\033[0m"
	@echo "======================"
	$(CC) example1.c $(CFLAGS) ../lib/librazer_chroma.a $(LIBS) -o example1
	@strip example1
	$(CC) raw_example.c $(CFLAGS) $(LIBS) -o raw_example
	@strip raw_example
	$(CC) input_example.c $(CFLAGS) ../lib/librazer_chroma.a $(LIBS) -o input_example
	@strip input_example
	$(CC) input_map.c $(CFLAGS) ../lib/librazer_chroma.a $(LIBS) -o input_map
	@strip input_map
	$(CC) dota_keys.c $(CFLAGS) ../lib/librazer_chroma.a $(LIBS) -o dota_keys
	@strip dota_keys
	$(CC) controller_example.c $(CFLAGS) ../lib/librazer_chroma_controller.a $(LIBS) -o controller_example
	@strip controller_example

snake:
	make -C snake_game all

snake_clean:
	make -C snake_game clean

dynamic:
	@echo "\n::\033[32m COMPILING dynamic\033[0m"
	@echo "======================"

	$(CC) dynamic.c $(CFLAGS) ../lib/librazer_chroma.a $(LIBS) -o dynamic
	@strip dynamic

clean: snake_clean
	rm -f *.do *.o input_example input_example_debug example1 example1_debug dota_keys dota_keys_debug dynamic input_map controller_example controller_example_debug input_map_debug raw_example raw_example_debug

indent:
	indent -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 *.c *.h
