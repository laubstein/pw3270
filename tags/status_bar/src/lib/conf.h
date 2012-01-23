/* conf.h.  Generated by configure.  */
/*
 * Copyright 2000, 2001, 2002 by Paul Mattes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * c3270 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 *
 */

/*
 *	conf.h
 *              System-specific #defines for libraries and library functions.
 *		Automatically generated from conf.h.in by configure.
 */


/* Libraries. */
#define HAVE_LIBNCURSES 1
/* #undef HAVE_LIBCURSES */
#define HAVE_LIBUTIL 1
/* #undef HAVE_LIBSOCKET */
/* #undef HAVE_LIBREADLINE */
/* #undef HAVE_LIBSSL */

/* Header files. */
#define HAVE_NCURSES_H 1
/* #undef HAVE_CURSES_H */
#define HAVE_SYS_SELECT_H 1
/* #undef HAVE_READLINE_HISTORY_H */
#define HAVE_PTY_H 1
/* #undef HAVE_LIBUTIL_H */
/* #undef HAVE_UTIL_H */
#define HAVE_GETOPT_H 1

/* Uncommon functions. */
#define HAVE_VASPRINTF 1
#define HAVE_FSEEKO 1

/* Default pager. */
#define LESSPATH "/usr/bin/less"
#define MOREPATH "/bin/more"

/* Broken stuff. */
/* #undef BROKEN_NEWTERM */

/* Optional parts. */
#define X3270_ANSI 1
#define X3270_APL 1
/* #undef X3270_DBCS */
#define X3270_FT 1
#define X3270_LOCAL_PROCESS 1
#define X3270_PRINTER 1
#define X3270_SCRIPT 1
#define X3270_TN3270E 1
#define X3270_TRACE 1