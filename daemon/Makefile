LIBS=-lm -ldl
CFLAGS=
DEBUG_CFLAGS=
#LDFLAGS=
#DEBUG_LDFLAGS=
LIBDBUS = $(shell echo \\\#include \<dbus/dbus.h\>\\n int main\(int c,char**v\){} | gcc `pkg-config --libs dbus-1` `pkg-config --cflags dbus-1`  -xc -o a.out - 2>/dev/null ;echo $$? ;rm a.out 2>/dev/null )

ifeq ($(LIBDBUS), 0)
	LIBS+= `pkg-config --libs dbus-1` 
	CFLAGS+= `pkg-config --cflags dbus-1` -DUSE_DBUS
	DEBUG_CFLAGS+= `pkg-config --cflags dbus-1` -DUSE_DBUS
endif

#-lSDL2 -lSDL2_image
MAJOR_VERSION=0
MINOR_VERSION=4
CSTD=-std=c99
CC=gcc
CFLAGS+= -Wl,-E -Wall -Wextra -W -w -Os $(CSTD) -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
DEBUG_CFLAGS+= -Wl,-E -Wall -Wextra -Wformat=0 -pedantic -fbounds-check -Wuninitialized -O0 -g3 $(CSTD) -DUSE_DEBUGGING -DMAJOR_VERSION=$(MAJOR_VERSION) -DMINOR_VERSION=$(MINOR_VERSION) -fPIC -DPIC -DLINUX -D_XOPEN_SOURCE=700
LDFLAGS=$(LIBS)
DEBUG_LDFLAGS= $(LIBS)
#-DUSE_VERBOSE_DEBUGGING
#-pedantic -fbounds-check -Wuninitialized -O0 -g3 -Wstrict-prototypes

RAZER_CHROMA_DAEMON_FILES=razer_daemon.c razer_daemon_dbus.c razer_daemon_effects.c razer_daemon_libraries.c razer_daemon_parameters.c razer_daemon_render_nodes.c razer_daemon_types.c
RAZER_CHROMA_DAEMON_INCLUDE_FILES=razer_daemon.h razer_daemon_dbus.h razer_daemon_effects.h razer_daemon_libraries.h razer_daemon_parameters.h razer_daemon_render_nodes.h razer_daemon_types.h
RAZER_CHROMA_DAEMON_OBJ=razer_daemon.o razer_daemon_dbus.o razer_daemon_effects.o razer_daemon_libraries.o razer_daemon_parameters.o razer_daemon_render_nodes.o razer_daemon_types.o
RAZER_CHROMA_DAEMON_DEBUG_OBJ=razer_daemon.do razer_daemon_dbus.do razer_daemon_effects.do razer_daemon_libraries.do razer_daemon_parameters.do razer_daemon_render_nodes.do razer_daemon_types.do

all: daemon_shared daemon_static daemon_debug daemon_fx

daemon_debug: $(RAZER_CHROMA_DAEMON_DEBUG_OBJ)
	@echo "::\033[32m COMPILING razer_bcd daemon [DEBUG]\033[0m"
	@echo "======================================"
	$(CC) -rdynamic $(RAZER_CHROMA_DAEMON_DEBUG_OBJ) librazer_chroma.da $(LIBS) -o razer_bcd_debug_static 
	$(CC) -rdynamic $(RAZER_CHROMA_DAEMON_DEBUG_OBJ) librazer_chroma_debug.so $(LIBS) -o razer_bcd_debug_shared 

daemon_static: $(RAZER_CHROMA_DAEMON_OBJ)
	@echo "::\033[32m COMPILING razer_bcd daemon\033[0m"
	@echo "=============================="
	$(CC) -rdynamic $(RAZER_CHROMA_DAEMON_OBJ) librazer_chroma.a $(LIBS) -o razer_bcd_static 
	@strip razer_bcd_static

daemon_shared: $(RAZER_CHROMA_DAEMON_OBJ)
	@echo "::\033[32m COMPILING razer_bcd [SHARED]\033[0m"
	@echo "================================"
	$(CC) -rdynamic $(RAZER_CHROMA_DAEMON_OBJ) librazer_chroma.so $(LIBS) -o razer_bcd
	@strip razer_bcd

%.o: %.c 
	$(CC) $(CFLAGS) -c -o $@ $<

%.do: %.c 
	$(CC) $(DEBUG_CFLAGS) -c -o $@ $<


daemon_fx: 
	make -C fx all

daemon_fx_clean: 
	make -C fx clean 

install:
	@echo "::\033[32m INSTALLING razer_bcd daemon\033[0m"
	@echo "======================================"
	@cp razer_bcd /usr/sbin
	@echo "::\033[32m INSTALLING razer daemon fx files\033[0m"
	@echo "====================================================="
	mkdir -p /usr/share/razer_bcd/fx
	cp fx/pez2001_collection.so /usr/share/razer_bcd/fx
	cp fx/pez2001_mixer.so /usr/share/razer_bcd/fx
	cp fx/pez2001_light_blast.so /usr/share/razer_bcd/fx

uninstall:
	@echo "::\033[32m REMOVING razer_bcd daemon\033[0m"
	@echo "====================================="
	@rm -f /usr/sbin/razer_bcd
	@echo "::\033[32m REMOVING razer daemon fx files\033[0m"
	@echo "====================================================="
	@rm -f /usr/share/razer_bcd/fx/pez2001_collection.so
	@rm -f /usr/share/razer_bcd/fx/pez2001_mixer.so
	@rm -f /usr/share/razer_bcd/fx/pez2001_light_blast.so

clean: daemon_fx_clean
	rm -f *.do *.da *.o razer_bcd razer_bcd_debug_shared razer_bcd_debug_static razer_bcd_static

indent:	
	indent -bap -bli0 -i4 -l79 -ncs -npcs -npsl -fca -lc79 -fc1 -ts4 *.c *.h

