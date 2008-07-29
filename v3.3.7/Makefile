
PACKAGE=g3270
PACKAGE_VERSION=3.3.7p5

# Flags
CROSS=1
OBJEXT=o
DLLEXT=dll
BINEXT=exe

# Set command paths and mkfb (which has to run locally)
MKFB = mkfb
CC = mingw32-gcc
NATIVECC = gcc
WINDRES = mingw32-windres
PKG_CONFIG_PATH=/usr/i386-mingw32/lib/pkgconfig

# Set compile/link parameters
XCPPFLAGS = -D_WIN32 -DWC3270 -D_WIN32_WINNT=0x0500
CFLAGS = $(EXTRA_FLAGS) -mno-cygwin -g -Wall $(XCPPFLAGS) -O -DG3270 -Isrc/include
DLLFLAGS = $(EXTRA_FLAGS) -mno-cygwin -shared -Wl,--export-all-symbols -Wl,--enable-auto-import
BINFLAGS = -mwindows

# PKGConfig Stuff
GTK_MODULES = glib-2.0 gtk+-2.0 gthread-2.0
SSL_MODULES = libcrypto libssl openssl

# Sources
G3270_SRCS =	g3270.c iocallback.c topwindow.c terminal.c screen.c actions.c mouse.c \
				clipboard.c print.c config.c colors.c resources.rc

LIBRARY_SRCS =	XtGlue.c actions.c ansi.c apl.c charset.c ctlr.c \
				ft.c ft_cut.c ft_dft.c glue.c help.c host.c icmd.c idle.c kybd.c \
				macros.c print.c printer.c proxy.c resources.c rpq.c screen.c see.c \
				sf.c tables.c telnet.c toggles.c trace_ds.c utf8.c util.c \
				xio.c keymap.c w3misc.c winvers.c windirs.c resolver.c log.c


# Main Targets
Debug: bin/Debug/g3270.$(BINEXT) bin/Debug/w3n46.dll

Release: bin/Release/g3270.$(BINEXT) bin/Release/w3n46.dll

# Rules
GLOBAL_DEPENDS=src/include/*.h src/include/lib3270/*.h Makefile

# Debug Rules
obj/Debug/src/lib/%.o: src/lib/%.c src/lib/*.h $(GLOBAL_DEPENDS)
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) -DDEBUG=1 -DLIB3270=1 -DHAVE_LIBSSL=1 `pkg-config --cflags $(SSL_MODULES)` $(CFLAGS) -Isrc/lib -Isrc/include/lib3270 -ggdb -o $@ -c $<

obj/Debug/src/g3270/%.o: src/g3270/%.c src/g3270/*.h $(GLOBAL_DEPENDS)
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) -DDEBUG=1 $(CFLAGS) `pkg-config --cflags $(GTK_MODULES)` -Isrc/lib -ggdb -o $@ -c $<

obj/Debug/src/g3270/resources.o: src/g3270/resources.rc src/g3270/g3270.ico Makefile
	@echo $< ...
	@$(WINDRES) --include-dir=src/g3270 -i $< -o $@

obj/Debug/version.o: src/version.c
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) -DDEBUG=1 $(CFLAGS) -Isrc/lib -ggdb -o $@ -c $<

# Release rules
obj/Release/src/lib/%.o: src/lib/%.c src/lib/*.h $(GLOBAL_DEPENDS)
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) -DLIB3270=1 -DHAVE_LIBSSL=1 $(CFLAGS) -Werror `pkg-config --cflags $(SSL_MODULES)` -Isrc/lib -Isrc/include/lib3270 -ggdb -o $@ -c $<

obj/Release/src/g3270/%.o: src/g3270/%.c src/g3270/*.h $(GLOBAL_DEPENDS)
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) $(CFLAGS) -Werror `pkg-config --cflags $(GTK_MODULES)` -Isrc/lib -o $@ -c $<

obj/Release/src/g3270/resources.o: src/g3270/resources.rc src/g3270/g3270.ico Makefile
	@echo $< ...
	@$(WINDRES) --include-dir=src/g3270 -i $< -o $@

obj/Release/version.o: src/version.c
	@echo $< ...
	@mkdir -p `dirname $@`
	@$(CC) $(CFLAGS) -Werror -Isrc/lib -o $@ -c $<

obj/Release/src/lib/w3n46.o: src/lib/resolver.c
	@echo $@ ...
	@mkdir -p `dirname $@`
	@$(CC) $(CFLAGS) -Werror -Isrc/lib -DW3N46 -c -o $@ $^

# Release Targets
bin/Release/g3270.$(BINEXT): bin/Release/lib3270.$(DLLEXT) $(foreach SRC, $(basename $(G3270_SRCS)), obj/Release/src/g3270/$(SRC).$(OBJEXT))
	@echo $@ ...
	@mkdir -p `dirname $@`
	@$(CC) $(BINFLAGS) -o $@ $^ `pkg-config --libs $(GTK_MODULES)` -lws2_32

bin/Release/lib3270.$(DLLEXT): obj/Release/version.o obj/Release/src/lib/fallbacks.o $(foreach SRC, $(basename $(LIBRARY_SRCS)), obj/Release/src/lib/$(SRC).$(OBJEXT))
	@echo $@ ...
	@mkdir -p `dirname $@`
	@$(CC) $(DLLFLAGS) -o $@ $^ `pkg-config --libs $(SSL_MODULES)` -lws2_32

bin/Release/w3n46.dll: obj/Release/src/lib/w3n46.o
	@$(CC) $(DLLFLAGS) -o $@ $^ -lws2_32

# Debug Targets
bin/Debug/c3270.$(BINEXT): bin/Debug/lib3270.$(DLLEXT) $(foreach SRC, $(basename $(C3270_SRCS)), obj/Debug/src/c3270/$(SRC).$(OBJEXT))
	@echo $@ ...
	@mkdir -p `dirname $@`
	@$(CC) -o $@ $^ -lws2_32

bin/Debug/g3270.$(BINEXT): bin/Debug/lib3270.$(DLLEXT) $(foreach SRC, $(basename $(G3270_SRCS)), obj/Debug/src/g3270/$(SRC).$(OBJEXT))
	@echo $@ ...
	@mkdir -p `dirname $@`
	@$(CC) $(BINFLAGS) -o $@ $^ `pkg-config --libs $(GTK_MODULES)` -lws2_32

bin/Debug/lib3270.$(DLLEXT): obj/Debug/version.o obj/Release/src/lib/fallbacks.o $(foreach SRC, $(basename $(LIBRARY_SRCS)), obj/Debug/src/lib/$(SRC).$(OBJEXT))
	@echo $@ ...
	@mkdir -p `dirname $@`
	@$(CC) $(DLLFLAGS) -o $@ $^ `pkg-config --libs $(SSL_MODULES)` -lws2_32

bin/Debug/w3n46.dll: obj/Release/src/lib/w3n46.o
	@$(CC) $(DLLFLAGS) -o $@ $^ -lws2_32

# Other targets
src/lib/fallbacks.c:  src/lib/X3270.xad
	@make -C src/lib fallbacks.c

src/g3270/g3270.ico: src/g3270/g3270.jpg Makefile
	@convert $< $@

src/version.c:	src/lib/*.c src/mkversion.sh Makefile $(GLOBAL_DEPENDS)
	@src/mkversion.sh

sources.lst: Makefile
	@rm -f $@
	@for i in $(G3270_SRCS); do echo $$i >> $@; done

$(PACKAGE).pot: sources.lst $(foreach SRC, $(G3270_SRCS), src/g3270/$(SRC))
	@xgettext	--files-from=sources.lst --directory=src/g3270 \
				--default-domain=$(PACKAGE) --language=C \
				--keyword=_ --keyword=N_ --sort-output --output=- \
	| sed "s@PACKAGE VERSION@$(PACKAGE) $(PACKAGE_VERSION)@;s@CHARSET@UTF-8@" > $@

$(PACKAGE).po: $(PACKAGE).pot
	@msginit --no-translator -o $(PACKAGE).po -i $(PACKAGE).pot

clean:
	@rm -fr obj
	@rm -fr bin
	@rm -f	src/lib/fallbacks.c
	@rm -f	src/lib/version.o
	@rm -f	src/lib/mkfb.exe
	@rm -f	src/version.c
	@rm -f	src/g3270/g3270.ico
	@rm -f	sources.lst
	@rm -f	g3270.tar.gz
	@rm -f $(PACKAGE).pot
	@rm -f $(PACKAGE).po
	@rm -fr autom4te.cache
	@rm -f config.status
	@find . -name "*.save" -exec mv -f {} $(TMPDIR) \;
	@find . -name "*~" -exec mv -f {} $(TMPDIR) \;
	@find . -name "*.bak" -exec mv -f {} $(TMPDIR) \;
	@find . -name "*.log" -exec mv -f {} $(TMPDIR) \;
	@find . -name "leak.out" -exec mv -f {} $(TMPDIR) \;
	@rm -f svn-commit.tmp
