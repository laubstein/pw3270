
# Flags
CROSS=1


# Set command paths and mkfb (which has to run locally) based on whether
# compiling natively on Cygwin, or cross-compiling on Linux.
ifeq ($(CROSS),1)
MKFB = mkfb
CC = mingw32-gcc
NATIVECC = gcc
WINDRES = mingw32-windres
else
MKFB = mkfb.exe
CC = gcc
NATIVECC = gcc
WINDRES = windres
endif

XCPPFLAGS = -D_WIN32 -DWC3270 -D_WIN32_WINNT=0x0500
CFLAGS = $(EXTRA_FLAGS) -mno-cygwin -g -Wall -Werror $(XCPPFLAGS) -Isrc/include $(SSLCPP)

# Sources
C3270_SRCS =	c3270.c
LIBRARY_SRCS =	XtGlue.c actions.c ansi.c apl.c c3270.c charset.c ctlr.c \
				ft.c ft_cut.c ft_dft.c glue.c help.c host.c icmd.c idle.c kybd.c \
				macros.c print.c printer.c proxy.c resources.c rpq.c screen.c see.c \
				sf.c tables.c telnet.c toggles.c trace_ds.c utf8.c util.c \
				xio.c fallbacks.c keymap.c w3misc.c winvers.c windirs.c resolver.c


# Rules
GLOBAL_DEPENDS=src/include/*.h src/include/lib3270/*.h

obj/Debug/src/lib/%.o: src/lib/%.c src/lib/*.h $(GLOBAL_DEPENDS)
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) -DDEBUG=1 $(CFLAGS) -ggdb -o $@ -c $<


# Targets
Debug:

Release:

clean:
	@rm -fr obj
