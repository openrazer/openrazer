LIBS=-lm
MAJOR_VERSION=0
MINOR_VERSION=2
CSTD=-std=c99
CFLAGS=`pkg-config --cflags dbus-1` -Wall -Wextra -W -w -Os $(CSTD) -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
DEBUG_CFLAGS=`pkg-config --cflags dbus-1` -Wall -Wextra -Wformat=0 -pedantic -fbounds-check -Wuninitialized -O0 -g3 $(CSTD) -DUSE_DEBUGGING -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
#-pedantic -fbounds-check -Wuninitialized -O0 -g3 -Wstrict-prototypes
LDFLAGS=$(LIBS)
DEBUG_LDFLAGS =$(LIBS)
CC=gcc

all: fx_debug fx

fx_debug: 
	@echo "::\033[32m COMPILING pez2001 fx collection [DEBUG]\033[0m"
	@echo "==========================================="
	$(CC) pez2001_collection.c $(DEBUG_CFLAGS) $(LIBS) -c -o pez2001_collection.do 
	$(CC) -rdynamic -nostdlib -DCREATELIB -shared pez2001_collection.do $(LDFLAGS) -o pez2001_collection_debug.so

	@echo "::\033[32m COMPILING pez2001 fx mixer [DEBUG]\033[0m"
	@echo "======================================"
	$(CC) pez2001_mixer.c $(DEBUG_CFLAGS) $(LIBS) -c -o pez2001_mixer.do 
	$(CC) -rdynamic -nostdlib -DCREATELIB -shared pez2001_mixer.do $(LDFLAGS) -o pez2001_mixer_debug.so

	@echo "::\033[32m COMPILING pez2001 fx light blast [DEBUG]\033[0m"
	@echo "============================================"
	$(CC) pez2001_light_blast.c $(DEBUG_CFLAGS) $(LIBS) -c -o pez2001_light_blast.do 
	$(CC) -rdynamic -nostdlib -DCREATELIB -shared pez2001_light_blast.do $(LDFLAGS) -o pez2001_light_blast_debug.so

	@echo "::\033[32m COMPILING pez2001 fx progress bar [DEBUG]\033[0m"
	@echo "============================================"
	$(CC) pez2001_progress_bar.c $(DEBUG_CFLAGS) $(LIBS) -c -o pez2001_progress_bar.do 
	$(CC) -rdynamic -nostdlib -DCREATELIB -shared pez2001_progress_bar.do $(LDFLAGS) -o pez2001_progress_bar_debug.so

fx: 
	@echo "::\033[32m COMPILING pez2001 fx collection\033[0m"
	@echo "==================================="
	$(CC) pez2001_collection.c $(CFLAGS) $(LIBS) -c -o pez2001_collection.o 
	$(CC) -rdynamic -nostdlib -DCREATELIB -shared pez2001_collection.o $(LDFLAGS) -o pez2001_collection.so
	@strip pez2001_collection.so

	@echo "::\033[32m COMPILING pez2001 fx mixer\033[0m"
	@echo "=============================="
	$(CC) pez2001_mixer.c $(CFLAGS) $(LIBS) -c -o pez2001_mixer.o
	$(CC) -rdynamic -nostdlib -DCREATELIB -shared pez2001_mixer.o $(LDFLAGS) -o pez2001_mixer.so
	@strip pez2001_mixer.so

	@echo "::\033[32m COMPILING pez2001 fx light blast\033[0m"
	@echo "===================================="
	$(CC) pez2001_light_blast.c $(CFLAGS) $(LIBS) -c -o pez2001_light_blast.o
	$(CC) -rdynamic -nostdlib -DCREATELIB -shared pez2001_light_blast.o $(LDFLAGS) -o pez2001_light_blast.so
	@strip pez2001_light_blast.so

	@echo "::\033[32m COMPILING pez2001 fx progress bar\033[0m"
	@echo "===================================="
	$(CC) pez2001_progress_bar.c $(CFLAGS) $(LIBS) -c -o pez2001_progress_bar.o
	$(CC) -rdynamic -nostdlib -DCREATELIB -shared pez2001_progress_bar.o $(LDFLAGS) -o pez2001_progress_bar.so
	@strip pez2001_progress_bar.so

clean: 
	rm -f *.do *.o pez2001_collection.so pez2001_collection_debug.so pez2001_mixer.so pez2001_mixer_debug.so pez2001_light_blast.so pez2001_light_blast_debug.so pez2001_progress_bar.so pez2001_progress_bar_debug.so

indent:	
	indent -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 *.c *.h

