# Copyright 1999, 2000, 2001, 2002, 2004 by Paul Mattes.
#  Permission to use, copy, modify, and distribute this software and its
#  documentation for any purpose and without fee is hereby granted,
#  provided that the above copyright notice appear in all copies and that
#  both that copyright notice and this permission notice appear in
#  supporting documentation.
#
# c3270 is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.

# Makefile for c3270 (console-based 3270 emulator)

RM = rm -f
CC = gcc

all:: c3270 x3270if

SRCS = actions.c ansi.c apl.c c3270.c charset.c child.c ctlr.c \
	ft.c ft_cut.c ft_dft.c glue.c help.c host.c idle.c keymap.c kybd.c \
	macros.c print.c printer.c resources.c rpq.c screen.c see.c sf.c \
	tables.c telnet.c toggles.c trace_ds.c util.c xio.c XtGlue.c
VOBJS = actions.o ansi.o apl.o c3270.o charset.o child.o ctlr.o fallbacks.o \
	ft.o ft_cut.o ft_dft.o glue.o help.o host.o idle.o keymap.o kybd.o \
	macros.o print.o printer.o resources.o rpq.o screen.o see.o sf.o \
	tables.o telnet.o toggles.o trace_ds.o util.o xio.o XtGlue.o
OBJS1 = $(VOBJS) version.o

LIBDIR = ${exec_prefix}/lib
prefix = /usr/local
exec_prefix = ${prefix}
sysconfdir = ${prefix}/etc
LIBX3270DIR = ${sysconfdir}/x3270
MANDIR = ${prefix}/man
BINDIR = ${exec_prefix}/bin
INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

XCPPFLAGS = -I. -DLIBX3270DIR=\"$(LIBX3270DIR)\"  -D_POSIX_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE
CFLAGS = -g -O2 $(XCPPFLAGS) $(CDEBUGFLAGS)
LDFLAGS =
LIBS = -lnsl -lncurses -lutil

version.o: $(VOBJS) version.txt mkversion.sh
	@chmod +x mkversion.sh version.txt
	sh ./mkversion.sh $(CC) c3270

fallbacks.c: mkfb X3270.xad
	$(RM) $@
	./mkfb -c X3270.xad $@

lib3270.so: $(OBJS1)
	$(CC) -shared $(LIBS) -Wl,-soname,$(@F) -o $@ $^

c3270: $(OBJS1)
	$(CC) -o $@ $(OBJS1) $(LDFLAGS) $(LIBS)

x3270if: x3270if.o
	$(CC) -o $@ x3270if.o

clean:
	@rm -f $(OBJS1)
	@rm -f lib3270.so
	@rm -f c3270
	@rm -f x3270if


