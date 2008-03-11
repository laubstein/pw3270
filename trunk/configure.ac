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


dnl Process this file with autoconf to produce a configure script.
AC_INIT(g3270,3.3.4)

AC_PREREQ(2.50)
AC_CONFIG_HEADER(src/config.h)

dnl Checks for programs.
AC_PROG_CC
AC_CHECK_PROG(PKGCONFIG, pkg-config, `which pkg-config`)

dnl Generating library
AC_CONFIG_SUBDIRS(src/lib)

dnl Generate the Makefile.
AC_OUTPUT(Makefile $pr3mf)
