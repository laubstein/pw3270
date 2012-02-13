dnl Copyright 2008, Banco do Brasil S.A.
dnl
dnl This file is part of g3270
dnl This program file is free software; you can redistribute it and/or modify it
dnl under the terms of the GNU General Public License as published by the
dnl Free Software Foundation; version 3 of the License.
dnl
dnl This program is distributed in the hope that it will be useful, but WITHOUT
dnl ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
dnl FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
dnl for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program in a file named COPYING; if not, write to the
dnl Free Software Foundation, Inc.,
dnl 51 Franklin Street, Fifth Floor,
dnl Boston, MA 02110-1301 USA
dnl
dnl Authors:
dnl
dnl Perry Werneck<perry.werneck@gmail.com>
dnl

AC_PREREQ(2.60)
AC_INIT(pw3270,3.3.4)
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
