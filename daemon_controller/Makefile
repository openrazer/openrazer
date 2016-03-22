LIBS=-lm -ldl
CFLAGS=
DEBUG_CFLAGS=
DESTDIR?=/
#LDFLAGS=
#DEBUG_LDFLAGS=
LIBDBUS=$(shell pkg-config --libs dbus-1 > /dev/null 2>&1 && echo 0)

ifeq ($(LIBDBUS), 0)
	LIBS+= `pkg-config --libs dbus-1` 
	CFLAGS+= `pkg-config --cflags dbus-1` -DUSE_DBUS
	DEBUG_CFLAGS+= `pkg-config --cflags dbus-1` -DUSE_DBUS
#	CFLAGS+= -DUSE_DBUS
#	DEBUG_CFLAGS+= -DUSE_DBUS
endif

#-lSDL2 -lSDL2_image
MAJOR_VERSION=0
MINOR_VERSION=3
CSTD=-std=c99
CC=gcc
CFLAGS+= -Wl,-E -Wall -Wextra -W -w -Os $(CSTD) -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
DEBUG_CFLAGS+= -Wl,-E -Wall -Wextra -Wformat=0 -pedantic -fbounds-check -Wuninitialized -O0 -g3 $(CSTD) -DUSE_DEBUGGING -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
LDFLAGS=$(LIBS)
DEBUG_LDFLAGS= $(LIBS)
#-DUSE_VERBOSE_DEBUGGING
#-pedantic -fbounds-check -Wuninitialized -O0 -g3 -Wstrict-prototypes

RAZER_CHROMA_DAEMON_CONTROLLER_FILES=razer_daemon_controller.c
RAZER_CHROMA_DAEMON_CONTROLLER_INCLUDE_FILES=razer_daemon_controller.h
RAZER_CHROMA_DAEMON_CONTROLLER_OBJ=razer_daemon_controller.o
RAZER_CHROMA_DAEMON_CONTROLLER_DEBUG_OBJ=razer_daemon_controller.do

all: daemon_controller_shared daemon_controller_static daemon_controller_debug

daemon_controller_debug: $(RAZER_CHROMA_DAEMON_CONTROLLER_DEBUG_OBJ)
	@echo "\n::\033[32m COMPILING razer_bcd controller [DEBUG]\033[0m"
	@echo "================================================="
	$(CC) -rdynamic $(RAZER_CHROMA_DAEMON_CONTROLLER_DEBUG_OBJ) ../lib/librazer_chroma.da ../lib/librazer_chroma_controller.da $(LIBS) -o razer_bcd_controller_debug_static 
	$(CC) -L../lib -rdynamic $(RAZER_CHROMA_DAEMON_CONTROLLER_DEBUG_OBJ) -lrazer_chroma_debug -lrazer_chroma_controller $(LIBS) -o razer_bcd_controller_debug_shared 

daemon_controller_static: $(RAZER_CHROMA_DAEMON_CONTROLLER_OBJ)
	@echo "\n::\033[32m COMPILING razer_bcd controller\033[0m"
	@echo "========================================="
	$(CC) -rdynamic $(RAZER_CHROMA_DAEMON_CONTROLLER_OBJ) ../lib/librazer_chroma.a ../lib/librazer_chroma_controller.a $(LIBS) -o razer_bcd_controller_static 
	@strip razer_bcd_controller_static

daemon_controller_shared: $(RAZER_CHROMA_DAEMON_CONTROLLER_OBJ)
	@echo "\n::\033[32m COMPILING razer_bcd controller [SHARED]\033[0m"
	@echo "==========================================="
	$(CC) -L../lib -rdynamic $(RAZER_CHROMA_DAEMON_CONTROLLER_OBJ) -lrazer_chroma -lrazer_chroma_controller $(LIBS) -o razer_bcd_controller
	@strip razer_bcd_controller

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $<

%.do: %.c 
	$(CC) $(DEBUG_CFLAGS) -c -o $@ $<

install:
	@echo "\n::\033[34m Installing razer_bcd daemon controller\033[0m"
	@echo "======================================="
	@install -v -D razer_bcd_controller $(DESTDIR)/usr/sbin/razer_bcd_controller

uninstall:
	@echo "\n::\033[34m Uninstalling razer_bcd daemon controller\033[0m"
	@echo "====================================="
	@rm -f $(DESTDIR)/usr/sbin/razer_bcd_controller

clean: 
	rm -f *.do *.da *.o razer_bcd_controller razer_bcd_controller_debug_shared razer_bcd_controller_debug_static razer_bcd_controller_static

indent:	
	indent -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 *.c *.h

