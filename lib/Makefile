LIBS=-lm
MAJOR_VERSION=0
MINOR_VERSION=3
CSTD=-std=c99
CFLAGS=-Wall -Wextra -W -w -Os $(CSTD) -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
DEBUG_CFLAGS=-Wall -Wextra -Wformat=0 -pedantic -fbounds-check -Wuninitialized -O0 -g3 $(CSTD) -DUSE_DEBUGGING -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
#-pedantic -fbounds-check -Wuninitialized -O0 -g3 -Wstrict-prototypes
LDFLAGS=$(LIBS)
DEBUG_LDFLAGS= $(LIBS)
CC=gcc
AR=ar -c
LD=ld

RAZER_CHROMA_FILES=razer_chroma.c
RAZER_CHROMA_INCLUDE_FILES=razer_chroma.h
RAZER_CHROMA_OBJ=razer_chroma.o
RAZER_CHROMA_DEBUG_OBJ=razer_chroma.do

all: library library_debug

library_debug: razer_chroma_static_debug razer_chroma_shared_debug
	@echo "::\033[32m COMPILING librazer_chroma.so [DEBUG]\033[0m"
	@echo "========================================"

library: razer_chroma_static razer_chroma_shared
	@echo "::\033[32m COMPILING librazer_chroma.so\033[0m"
	@echo "================================"

razer_chroma_static: $(RAZER_CHROMA_OBJ)
	$(AR) -rs librazer_chroma.a $(RAZER_CHROMA_OBJ)

razer_chroma_static_debug: $(RAZER_CHROMA_DEBUG_OBJ)
	$(AR) -rs librazer_chroma.da $(RAZER_CHROMA_DEBUG_OBJ)

razer_chroma_shared: $(RAZER_CHROMA_OBJ)
	$(CC) -rdynamic -nostdlib -DCREATELIB -shared $(RAZER_CHROMA_OBJ) $(LDFLAGS) -o librazer_chroma.so
	@strip librazer_chroma.so

razer_chroma_shared_debug: $(RAZER_CHROMA_DEBUG_OBJ)
	$(CC) -rdynamic -nostdlib -DCREATELIB -shared $(RAZER_CHROMA_DEBUG_OBJ) $(LDFLAGS) -o librazer_chroma_debug.so

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $<

%.do: %.c 
	$(CC) $(DEBUG_CFLAGS) -c -o $@ $<

install:
	@echo "::\033[32m INSTALLING librazer_chroma.so\033[0m"
	@echo "===================================="
	@cp librazer_chroma.so /usr/lib

uninstall:
	@echo "::\033[32m REMOVING librazer_chroma.so\033[0m"
	@echo "===================================="
	@rm /usr/lib/librazer_chroma.so

clean: 
	rm -f *.do *.o *.a *.so *.da

indent:	
	indent -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 *.c *.h

