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
AC_CONFIG_HEADER([src/g3270/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL

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

#dnl Configure which files to generate.
AC_OUTPUT(Makefile)

dnl Output the generated config.status script.
AC_OUTPUT
