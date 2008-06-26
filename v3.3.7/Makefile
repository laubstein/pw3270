
# Flags
CROSS=1
OBJEXT=o
DLLEXT=dll

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
DLLFLAGS = $(EXTRA_FLAGS) -mno-cygwin -shared -Wl,--export-all-symbols -Wl,--enable-auto-import


# Sources
C3270_SRCS =	c3270.c
LIBRARY_SRCS =	XtGlue.c actions.c ansi.c apl.c charset.c ctlr.c \
				ft.c ft_cut.c ft_dft.c glue.c help.c host.c icmd.c idle.c kybd.c \
				macros.c print.c printer.c proxy.c resources.c rpq.c screen.c see.c \
				sf.c tables.c telnet.c toggles.c trace_ds.c utf8.c util.c \
				xio.c keymap.c w3misc.c winvers.c windirs.c resolver.c



# Rules
GLOBAL_DEPENDS=src/include/*.h src/include/lib3270/*.h

obj/Debug/src/lib/%.o: src/lib/%.c src/lib/*.h $(GLOBAL_DEPENDS)
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) -DDEBUG=1 $(CFLAGS) -Isrc/lib -ggdb -o $@ -c $<

obj/Debug/lib3270.$(DLLEXT): $(foreach SRC, $(basename $(LIBRARY_SRCS)), obj/Debug/src/lib/$(SRC).$(OBJEXT))
	@echo $@ ...
	@mkdir -p `dirname $@`
	@$(CC) $(DLLFLAGS) -o $@ $^ -lws2_32


# Targets
Debug:

Release:

src/lib/fallbacks.c:  src/lib/X3270.xad
	@make -C src/lib fallbacks.c

src/version.c:	src/lib/*.c src/lib/mkversion.sh $(GLOBAL_DEPENDS)
	@src/lib/mkversion.sh

clean:
	@rm -fr obj
	@rm -f	src/lib/fallbacks.c
	@rm -f	src/lib/version.o
