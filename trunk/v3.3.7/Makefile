
# Flags
CROSS=1
OBJEXT=o
DLLEXT=dll
BINEXT=exe

# Set command paths and mkfb (which has to run locally) based on whether
# compiling natively on Cygwin, or cross-compiling on Linux.
ifeq ($(CROSS),1)
MKFB = mkfb
CC = mingw32-gcc
NATIVECC = gcc
WINDRES = mingw32-windres
PKG_CONFIG_PATH=/home/perry/mingw/lib/pkgconfig
else
MKFB = mkfb.exe
CC = gcc
NATIVECC = gcc
WINDRES = windres
endif

XCPPFLAGS = -D_WIN32 -DWC3270 -D_WIN32_WINNT=0x0500
CFLAGS = $(EXTRA_FLAGS) -mno-cygwin -g -Wall -Werror $(XCPPFLAGS) -Isrc/include $(SSLCPP)
DLLFLAGS = $(EXTRA_FLAGS) -mno-cygwin -shared -Wl,--export-all-symbols -Wl,--enable-auto-import

#
GTK_MODULES = glib-2.0 gtk+-2.0

# Sources
C3270_SRCS =	c3270.c iocallback.c
G3270_SRCS =	g3270.c iocallback.c
LIBRARY_SRCS =	XtGlue.c actions.c ansi.c apl.c charset.c ctlr.c \
				ft.c ft_cut.c ft_dft.c glue.c help.c host.c icmd.c idle.c kybd.c \
				macros.c print.c printer.c proxy.c resources.c rpq.c screen.c see.c \
				sf.c tables.c telnet.c toggles.c trace_ds.c utf8.c util.c \
				xio.c keymap.c w3misc.c winvers.c windirs.c resolver.c


# Main Targets
Debug: bin/Debug/g3270.$(BINEXT)

Release:


# Rules
GLOBAL_DEPENDS=src/include/*.h src/include/lib3270/*.h

# Debug Rules
obj/Debug/src/lib/%.o: src/lib/%.c src/lib/*.h $(GLOBAL_DEPENDS)
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) -DDEBUG=1 $(CFLAGS) -Isrc/lib -ggdb -o $@ -c $<

obj/Debug/src/c3270/%.o: src/c3270/%.c src/c3270/*.h $(GLOBAL_DEPENDS)
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) -DDEBUG=1 $(CFLAGS) -Isrc/lib -ggdb -o $@ -c $<

obj/Debug/src/g3270/%.o: src/g3270/%.c src/g3270/*.h $(GLOBAL_DEPENDS)
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) -DDEBUG=1 $(CFLAGS) `pkg-config --cflags $(GTK_MODULES)` -Isrc/lib -ggdb -o $@ -c $<

obj/Debug/version.o: src/version.c
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) -DDEBUG=1 $(CFLAGS) -Isrc/lib -ggdb -o $@ -c $<

# Release rules
obj/Release/src/lib/%.o: src/lib/%.c src/lib/*.h $(GLOBAL_DEPENDS)
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) $(CFLAGS) -Isrc/lib -o $@ -c $<

obj/Release/version.o: src/version.c
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) $(CFLAGS) -Isrc/lib -o $@ -c $<

obj/Release/src/lib/w3n46.o: src/lib/resolver.c
	@echo $@ ...
	@mkdir -p `dirname $@`
	$(CC) $(CFLAGS) -Isrc/lib -DW3N46 -c -o $@ $^


# Debug Targets
bin/Debug/c3270.$(BINEXT): bin/Debug/lib3270.$(DLLEXT) $(foreach SRC, $(basename $(C3270_SRCS)), obj/Debug/src/c3270/$(SRC).$(OBJEXT))
	@echo $@ ...
	@mkdir -p `dirname $@`
	@$(CC) -o $@ $^ -lws2_32

bin/Debug/g3270.$(BINEXT): bin/Debug/lib3270.$(DLLEXT) $(foreach SRC, $(basename $(G3270_SRCS)), obj/Debug/src/g3270/$(SRC).$(OBJEXT))
	@echo $@ ...
	@mkdir -p `dirname $@`
	@$(CC) -o $@ $^ `pkg-config --libs $(GTK_MODULES)` -lws2_32


bin/Debug/lib3270.$(DLLEXT): obj/Debug/version.o obj/Release/src/lib/fallbacks.o $(foreach SRC, $(basename $(LIBRARY_SRCS)), obj/Debug/src/lib/$(SRC).$(OBJEXT))
	@echo $@ ...
	@mkdir -p `dirname $@`
	@$(CC) $(DLLFLAGS) -o $@ $^ -lws2_32

bin/Debug/w3n46.dll: obj/Release/src/lib/w3n46.o
	@$(CC) $(DLLFLAGS) -o $@ $^ -lws2_32

# Other targets
src/lib/fallbacks.c:  src/lib/X3270.xad
	@make -C src/lib fallbacks.c

src/version.c:	src/lib/*.c src/mkversion.sh Makefile $(GLOBAL_DEPENDS)
	@src/mkversion.sh

clean:
	@rm -fr obj
	@rm -fr bin
	@rm -f	src/lib/fallbacks.c
	@rm -f	src/lib/version.o
	@rm -f	src/lib/mkfb.exe
	@rm -f	src/version.c
