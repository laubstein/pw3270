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
AC_CONFIG_HEADER([src/include/lib3270/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL

# Optional
AC_ARG_ENABLE([trace],
	[AS_HELP_STRING([--disable-trace], [disable trace])],
[
	g3270_cv_trace="$enableval"
],[
	g3270_cv_trace="yes"
])

AC_ARG_ENABLE([plugins],
	[AS_HELP_STRING([--disable-plugins], [disable plugin support])],
[
	g3270_cv_plugins="$enableval"

],[
	g3270_cv_plugins="yes"
])

AC_ARG_WITH([ssl],
	[AS_HELP_STRING([--without-ssl], [do not use libssl])],
[
	g3270_cv_ssl="$withval"
],[
	g3270_cv_ssl="yes"
])

AC_ARG_WITH([rexx],
	[AS_HELP_STRING([--without-rexx], [do not use rexx])],
[
	g3270_cv_rexx="$withval"
],[
	g3270_cv_rexx="yes"
])

AC_ARG_WITH([gnome],
	[AS_HELP_STRING([--without-gnome], [do not use libgnome])],
[
	g3270_cv_gnome="$withval"
],[
	g3270_cv_gnome="yes"
])

AC_ARG_WITH([icon],
	[AS_HELP_STRING([--with-icon], [set icon filename])],
[
	g3270_cv_icon="$withval"
],[
	g3270_cv_icon="image/default.ico"
])

AC_ARG_WITH([logo],
	[AS_HELP_STRING([--with-logo], [set logo filename])],
[
	g3270_cv_logo="$withval"
],[
	g3270_cv_logo="image/default.ico"
])


# Checks for libraries.
PKG_CHECK_EXISTS
PKG_CHECK_MODULES( [GTK], [glib-2.0 gtk+-2.0 >= 2.10 gthread-2.0 gthread-2.0 gmodule-2.0], AC_DEFINE(HAVE_GTK) )
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)

# Check for GNOME
if test "$g3270_cv_gnome" == "yes"; then
	PKG_CHECK_MODULES( [LIBGNOME], [libgnome-2.0 libgnomeui-2.0],
					AC_DEFINE(HAVE_LIBGNOME),
					AC_MSG_NOTICE([gnomeui not present.]) )
fi
AC_SUBST(LIBGNOME_LIBS)
AC_SUBST(LIBGNOME_CFLAGS)

# Check for libssl
if test "$g3270_cv_ssl" == "yes"; then
	PKG_CHECK_MODULES( [LIBSSL], [libssl],
					AC_DEFINE(HAVE_LIBSSL),
					AC_MSG_NOTICE([libssl not present.]) )
fi

# Check for rexx devel
if test "$g3270_cv_rexx" == "yes"; then
	AC_CHECK_PROGS(REXX_CONFIG, oorexx-config)

	if test -n "$REXX_CONFIG"; then
		AC_DEFINE(HAVE_REXX)
		PROGRAM_FLAGS="$PROGRAM_FLAGS `$REXX_CONFIG --cflags`"
		PROGRAM_LIBS="$PROGRAM_LIBS `$REXX_CONFIG --libs`"
	else
		AC_MSG_NOTICE([rexx not present.])
	fi

fi

AC_SUBST(LIBSSL_LIBS)
AC_SUBST(LIBSSL_CFLAGS)

# Other options

if test "$g3270_cv_trace" == "yes"; then
	AC_DEFINE([X3270_TRACE])
fi

if test "$g3270_cv_plugins" == "yes"; then
	AC_DEFINE([HAVE_PLUGINS])
fi

AC_DEFINE([X3270_TN3270E])
AC_DEFINE([X3270_ANSI])
AC_DEFINE([X3270_APL])
AC_DEFINE([X3270_FT])
AC_DEFINE([X3270_LOCAL_PROCESS])
AC_DEFINE([X3270_PRINTER])

AC_SUBST(ICONFILE,$g3270_cv_icon)
AC_SUBST(LOGOFILE,$g3270_cv_logo)

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
AC_SUBST(PROGRAM_LIBS)
AC_SUBST(PROGRAM_FLAGS)

#dnl Configure which files to generate.
AC_OUTPUT(Makefile)
AC_OUTPUT(g3270.spec)
AC_OUTPUT(g3270.pc)

dnl Output the generated config.status script.
AC_OUTPUT
