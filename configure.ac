dnl Copyright 2008 By Perry Werneck
dnl  Permission to use, copy, modify, and distribute this software and its
dnl  documentation for any purpose and without fee is hereby granted,
dnl  provided that the above copyright notice appear in all copies and that
dnl  both that copyright notice and this permission notice appear in
dnl  supporting documentation.
dnl
dnl MusicCenter is distributed in the hope that it will be useful, but WITHOUT ANY
dnl WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
dnl FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
dnl
dnl http://www.gnu.org/software/autoconf/manual/autoconf-2.57/html_node/autoconf_toc.html
dnl

AC_PREREQ(2.60)
AC_INIT(g3270,3.3.4)
AC_CONFIG_HEADER(src/config.h)

# Checks for programs.
AC_PROG_CC
AC_PROG_MAKE_SET

dnl pkg-config
AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test "x$PKG_CONFIG" = "xno"; then
        AC_MSG_ERROR([You need to install pkg-config])
fi

PKG_CHECK_MODULES(GTK, gtk+-2.0 gdk-2.0 glib-2.0 gthread-2.0 libgnomeui-2.0)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([stdlib.h string.h sys/file.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_REPLACE_FNMATCH
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRFTIME
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([memset strchr strdup strerror])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_SUBDIRS([src/lib])
AC_OUTPUT
