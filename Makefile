CC		:= gcc
CFLAGS		:= -Wall -Wextra -Wpedantic 
	
PREFIX    	:= /usr/local
BINPREFIX 	:= $(PREFIX)/bin

ifeq ($(DEBUG),1)
	CFLAGS+=-DDEBUG -g
endif

SCRATCH_SRC	:= scratchpad_util.c util.c shell_cmd.c
SCRATCH_OBJ 	:= $(SCRATCH_SRC:.c=.o)
RESIZE_SRC	:= resize_win.c util.c shell_cmd.c
RESIZE_OBJ 	:= $(RESIZE_SRC:.c=.o)
VALG 		:= $(wildcard vgcore.*)

all: 		build/scratchpad_util build/resize_win

$(OBJ): 	Makefile 

scratchpad_util: 	$(SCRATCH_OBJ)

resize_win: 		$(RESIZE_OBJ)


install: 
	mkdir -p "$(DESTDIR)$(BINPREFIX)"
	cp -p scratchpad_util "$(DESTDIR)$(BINPREFIX)"
	cp -p resize_win "$(DESTDIR)$(BINPREFIX)"

uninstall:
	rm -f "$(DESTDIR)$(BINPREFIX)/scratchpad_util"
	rm -f "$(DESTDIR)$(BINPREFIX)/resize_win"

clean: 	
	rm -f build/resize_win build/scratchpad_util $(RESIZE_OBJ) $(SCRATCH_OBJ) $(VALG)


