LIBS=-lm -ldl
CFLAGS=
DEBUG_CFLAGS=
LIBDBUS = $(shell echo \\\#include \<dbus/dbus.h\>\\n int main\(int c,char**v\){} | gcc `pkg-config --libs dbus-1` `pkg-config --cflags dbus-1`  -xc -o a.out - 2>/dev/null ;echo $$? ;rm a.out 2>/dev/null )

ifeq ($(LIBDBUS), 0)
	LIBS+= `pkg-config --libs dbus-1` 
	CFLAGS+= `pkg-config --cflags dbus-1` -DUSE_DBUS
	DEBUG_CFLAGS+= `pkg-config --cflags dbus-1` -DUSE_DBUS
endif

#-lSDL2 -lSDL2_image
MAJOR_VERSION=0
MINOR_VERSION=2
CSTD=-std=c99
CC=gcc
CFLAGS+= -Wl,-E -Wall -Wextra -W -w -Os $(CSTD) -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
DEBUG_CFLAGS+= -Wl,-E -Wall -Wextra -Wformat=0 -pedantic -fbounds-check -Wuninitialized -O0 -g3 $(CSTD) -DUSE_DEBUGGING -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
#-DUSE_VERBOSE_DEBUGGING
#-pedantic -fbounds-check -Wuninitialized -O0 -g3 -Wstrict-prototypes

RAZER_CHROMA_DAEMON_FILES=razer_daemon.c
RAZER_CHROMA_DAEMON_INCLUDE_FILES=razer_daemon.h
RAZER_CHROMA_DAEMON_OBJ=razer_daemon.o
RAZER_CHROMA_DAEMON_DEBUG_OBJ=razer_daemon.do

all: daemon_shared daemon_static daemon_debug daemon_fx

daemon_debug: 
	@echo "::\033[32m COMPILING razer_bcd daemon [DEBUG]\033[0m"
	@echo "======================================"
	$(CC) razer_daemon.c $(DEBUG_CFLAGS) $(LIBS) librazer_chroma.da -o razer_bcd_debug_static $<
	$(CC) razer_daemon.c $(DEBUG_CFLAGS) $(LIBS) librazer_chroma_debug.so -o razer_bcd_debug_shared $<

daemon_static: 
	@echo "::\033[32m COMPILING razer_bcd daemon\033[0m"
	@echo "=============================="
	$(CC) razer_daemon.c $(CFLAGS) $(LIBS) librazer_chroma.a -o razer_bcd_static $<
	@strip razer_bcd_static

daemon_shared: 
	@echo "::\033[32m COMPILING razer_bcd [SHARED]\033[0m"
	@echo "================================"
	$(CC) razer_daemon.c $(CFLAGS) $(LIBS) librazer_chroma.so -o razer_bcd $<
	@strip razer_bcd

daemon_fx: 
	make -C fx all

daemon_fx_clean: 
	make -C fx clean 

install:
	@echo "::\033[32m INSTALLING razer_bcd daemon\033[0m"
	@echo "======================================"
	@cp razer_bcd /usr/sbin

uninstall:
	@echo "::\033[32m REMOVING razer_bcd daemon\033[0m"
	@echo "====================================="
	@rm /usr/sbin/razer_bcd

clean: daemon_fx_clean
	rm -f *.do *.o razer_bcd razer_bcd_debug_shared razer_bcd_debug_static razer_bcd_static

indent:	
	indent -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 *.c *.h

