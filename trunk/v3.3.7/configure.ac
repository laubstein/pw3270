dnl 
dnl "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
dnl (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
dnl aplicativos mainframe.
dnl 
dnl Copyright (C) <2008> <Banco do Brasil S.A.>
dnl 
dnl Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
dnl os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
dnl Free Software Foundation.
dnl 
dnl Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
dnl GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
dnl A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
dnl obter mais detalhes.
dnl 
dnl Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
dnl programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
dnl Place, Suite 330, Boston, MA, 02111-1307, USA
dnl 
dnl Contatos: 
dnl 
dnl perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
dnl erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
dnl licinio@bb.com.br		(Licínio Luis Branco)
dnl kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
dnl macmiranda@bb.com.br	(Marco Aurélio Caldas Miranda)
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

AC_ARG_ENABLE([print-font-dialog],
	[AS_HELP_STRING([--disable-print-font-dialog], [disable font selection dialog on print jobs])],
[
	g3270_cv_print_font="$enableval"

],[
	g3270_cv_print_font="yes"
])


AC_ARG_WITH([ssl],
	[AS_HELP_STRING([--without-ssl], [do not use libssl])],
[
	g3270_cv_ssl="$withval"
],[
	g3270_cv_ssl="yes"
])

AC_ARG_ENABLE([mouse-pointer-change],
	[AS_HELP_STRING([--disable-mouse-pointer-change], [disable mouse pointer changes])],
[
	g3270_cv_pointer_change="$enableval"
],[
	g3270_cv_pointer_change="yes"
])

AC_ARG_ENABLE([selections],
	[AS_HELP_STRING([--disable-selections], [ignore selection events])],
[
	g3270_cv_use_selections="$enableval"
],[
	g3270_cv_use_selections="yes"
])

AC_ARG_ENABLE([primary-selection],
	[AS_HELP_STRING([--disable-primary-selection], [disable usage of GDK_SELECTION_PRIMARY])],
[
	g3270_cv_use_primary_selection="$enableval"
],[
	g3270_cv_use_primary_selection="yes"
])

AC_ARG_WITH([rexx],
	[AS_HELP_STRING([--without-rexx], [disable rexx plugin])],
[
	g3270_cv_rexx="$withval"
],[
	AC_CHECK_PROGS(REXX_CONFIG, oorexx-config)

	if test -n "$REXX_CONFIG"; then
		g3270_cv_rexx="yes"
	else
		g3270_cv_rexx="no"
	fi
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

AC_ARG_WITH([release],
	[AS_HELP_STRING([--with-release], [set program release])],
[
	g3270_cv_release="$withval"
],[
	g3270_cv_release="`date +%y%m%d`"
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

AC_SUBST(LIBSSL_LIBS)
AC_SUBST(LIBSSL_CFLAGS)

# Check for rexx devel
if test "$g3270_cv_rexx" == "yes"; then
	AC_DEFINE(HAVE_REXX)
	REXX_CFLAGS="`$REXX_CONFIG --cflags`"
	REXX_LIBS="$PROGRAM_LIBS `$REXX_CONFIG --libs`"
	SELECTED_PLUGINS="rx3270 $SELECTED_PLUGINS"
fi

AC_SUBST(REXX_LIBS)
AC_SUBST(REXX_CFLAGS)
AC_SUBST(REXX_CONFIG)

# Other options

if test "$g3270_cv_trace" == "yes"; then
	AC_DEFINE([X3270_TRACE])
fi

if test "$g3270_cv_plugins" == "yes"; then
	AC_DEFINE([HAVE_PLUGINS])
fi

if test "$g3270_cv_print_font" == "yes"; then
	AC_DEFINE([HAVE_PRINT_FONT_DIALOG])
fi

if test "$g3270_cv_pointer_change" == "yes"; then
	AC_DEFINE([MOUSE_POINTER_CHANGE])
fi

if test "$g3270_cv_use_selections" == "yes"; then
	AC_DEFINE([USE_SELECTIONS])
fi

if test "$g3270_cv_use_primary_selection" == "yes"; then
	AC_DEFINE([USE_PRIMARY_SELECTION])
fi



AC_DEFINE([X3270_TN3270E])
AC_DEFINE([X3270_ANSI])
AC_DEFINE([X3270_APL])
AC_DEFINE([X3270_FT])
AC_DEFINE([X3270_PRINTER])

AC_SUBST(PACKAGE_RELEASE,$g3270_cv_release)
AC_SUBST(ICONFILE,$g3270_cv_icon)
AC_SUBST(LOGOFILE,$g3270_cv_logo)

# Fixed values
AC_SUBST(LOGOEXT, "png")
AC_SUBST(DLLEXT, "so")
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
AC_SUBST(SELECTED_PLUGINS)
AC_SUBST(libdir)

#dnl Configure which files to generate.
AC_OUTPUT(Makefile)
AC_OUTPUT(g3270.spec)
AC_OUTPUT(g3270.pc)
AC_OUTPUT(src/g3270/version.c)

if test "$g3270_cv_rexx" == "yes"; then
	AC_OUTPUT(src/plugins/rexx/Makefile)
fi

dnl Output the generated config.status script.
AC_OUTPUT
