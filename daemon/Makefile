LIBS = -lm -lSDL2 -lSDL2_image
MAJOR_VERSION = 0
MINOR_VERSION = 1
CSTD = -std=c99
CC=gcc
CFLAGS= -Wall -Wextra -W -w -Os $(CSTD) -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
DEBUG_CFLAGS = -Wall -Wextra -Wformat=0 -pedantic -fbounds-check -Wuninitialized -O0 -g3 $(CSTD) -DUSE_DEBUGGING -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
#-pedantic -fbounds-check -Wuninitialized -O0 -g3 -Wstrict-prototypes

RAZER_CHROMA_DAEMON_FILES = razer_daemon.c
RAZER_CHROMA_DAEMON_BINS = razer_daemon.a
RAZER_CHROMA_DAEMON_INCLUDE_FILES = razer_daemon.h
RAZER_CHROMA_DAEMON_OBJ = razer_daemon.o
RAZER_CHROMA_DAEMON_DEBUG_OBJ = razer_daemon.do

all: daemon_shared daemon_static daemon_debug

daemon_debug: 
	@echo "====================================================="
	@echo "::::::::::::::::\033[32m COMPILING razer_bcd daemon [DEBUG]\033[0m ::::::::::::::"
	@echo "====================================================="
	$(CC) razer_daemon.c $(DEBUG_CFLAGS) $(LIBS) librazer_chroma.da -o razer_bcd_debug_static $<
	$(CC) razer_daemon.c $(DEBUG_CFLAGS) $(LIBS) librazer_chroma_debug.so -o razer_bcd_debug_shared $<

daemon_static: 
	@echo "====================================================="
	@echo "::::::::::::::::\033[32m COMPILING razer_bcd daemon\033[0m ::::::::::::::"
	@echo "====================================================="
	$(CC) razer_daemon.c $(CFLAGS) $(LIBS) librazer_chroma.a -o razer_bcd_static $<
	@strip razer_bcd_static

daemon_shared: 
	@echo "====================================================="
	@echo "::::::::::::::::\033[32m COMPILING razer_bcd [SHARED]\033[0m ::::::::::::::"
	@echo "====================================================="
	$(CC) razer_daemon.c $(CFLAGS) $(LIBS) librazer_chroma.so -o razer_bcd $<
	@strip razer_bcd

install:
	@echo "====================================================="
	@echo "::::::::::::::::\033[32m INSTALLING razer_bcd daemon\033[0m ::::::::::::::"
	@echo "====================================================="
	@cp razer_bcd /usr/sbin

uninstall:
	@echo "====================================================="
	@echo ":::::::::::::::::\033[32m REMOVING razer_bcd daemon\033[0m :::::::::::::::"
	@echo "====================================================="
	@rm /usr/sbin/razer_bcd

clean: 
	rm -f *.do *.o razer_bcd razer_bcd_debug_shared razer_bcd_debug_static razer_bcd_static

indent:	
	indent -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 *.c *.h

