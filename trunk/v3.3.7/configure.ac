dnl
dnl Copyright 2008, Banco do Brasil S.A.
dnl
dnl This file is part of g3270
dnl
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
dnl http://www.gnu.org/software/autoconf/manual/autoconf-2.57/html_node/autoconf_toc.html
dnl

# https://stage.maemo.org/svn/maemo/projects/haf/branches/maemo-af-desktop/hildon-desktop/configure.ac
# http://svn.abisource.com/libabiword-builder/trunk/configure.in

dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.59)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([g3270], [3.3.7p5], [perry.werneck@gmail.com])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/include/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL

# Optional
AC_ARG_WITH([trace], 
	[AS_HELP_STRING([--with-trace], [enable trace functions])], 
[
	g3270_cv_trace="$withval"
],[
	g3270_cv_trace="$no"
])

# Checks for libraries.
PKG_CHECK_EXISTS
PKG_CHECK_MODULES( [GTK], [glib-2.0 gtk+-2.0 >= 2.10 gthread-2.0 gthread-2.0 gmodule-2.0], AC_DEFINE(HAVE_GTK) )
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)

# Check for GNOMEUI
PKG_CHECK_MODULES( [GNOMEUI], [libgnomeui-2.0],
				AC_DEFINE(HAVE_GNOMEUI),
				AC_MSG_NOTICE([gnomeui not present.]) )
AC_SUBST(GNOMEUI_LIBS)
AC_SUBST(GNOMEUI_CFLAGS)

# Check for libssl
PKG_CHECK_MODULES( [LIBSSL], [libssl],
				AC_DEFINE(HAVE_LIBSSL),
				AC_MSG_NOTICE([libssl not present.]) )
AC_SUBST(LIBSSL_LIBS)
AC_SUBST(LIBSSL_CFLAGS)

# Other options

if test "$g3270_cv_trace" == "yes"; then
	AC_DEFINE([X3270_TRACE])
fi

AC_DEFINE([X3270_TN3270E])
AC_DEFINE([X3270_ANSI])
AC_DEFINE([X3270_APL])
AC_DEFINE([X3270_DBCS])
AC_DEFINE([X3270_FT])
AC_DEFINE([X3270_LOCAL_PROCESS])
AC_DEFINE([X3270_PRINTER])
AC_DEFINE([X3270_SCRIPT])

# Fixed values
AC_SUBST(DLLEXT, "so")
AC_SUBST(DATAPATH, "$datarootdir/$PACKAGE")
AC_SUBST(EXTRA_FLAGS)
AC_SUBST(XCPPFLAGS)
AC_SUBST(EXTRA_TARGETS)
AC_SUBST(BINEXT)
AC_SUBST(DLLEXT)
AC_SUBST(WINDRES)
AC_SUBST(NATIVECC)
AC_SUBST(CROSS)
AC_SUBST(OS_LIBS)

#dnl Configure which files to generate.
AC_OUTPUT(Makefile)
AC_OUTPUT(g3270.spec)

dnl Output the generated config.status script.
AC_OUTPUT
