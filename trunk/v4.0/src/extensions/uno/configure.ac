

# https://stage.maemo.org/svn/maemo/projects/haf/branches/maemo-af-desktop/hildon-desktop/configure.ac
# http://svn.abisource.com/libabiword-builder/trunk/configure.in

dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.59)

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([OpenOffice_org-3270], [0.1], [perry.werneck@gmail.com])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/config.hpp])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Optional
AC_ARG_WITH([service-name],
	[AS_HELP_STRING([--with-service-name], [Define UNO service name])],
[
	o3270_cv_srvname="$withval"
],[
	o3270_cv_srvname="br.com.bb.g3270"
])

AC_ARG_WITH([interface-name],
	[AS_HELP_STRING([--with-service-name], [Define UNO interface name])],
[
	o3270_cv_implname="$withval"
],[
	o3270_cv_implname="br.com.bb.I3270"
])

AC_ARG_WITH([office-home],
	[AS_HELP_STRING([--with-office-home], [Define Open-Office program home])],
[
	o3270_cv_oohome="$withval"
],[
	o3270_cv_oohome="/usr/lib/ooo3"
])

AC_ARG_WITH([office-sdk-home],
	[AS_HELP_STRING([--with-office-sdk-home], [Define Open-Office sdk home])],
[
	o3270_cv_sdkhome="$withval"
],[
	o3270_cv_sdkhome="/usr/lib/ooo3/solver"
])

AC_ARG_WITH([office-program],
	[AS_HELP_STRING([--with-office-program], [Define Open-Office main program])],
[
	o3270_cv_ooprogram="$withval"
],[
	o3270_cv_ooprogram="/usr/lib/ooo3/program/soffice.bin"
])


# Checks for libraries.
PKG_CHECK_EXISTS
PKG_CHECK_MODULES( [G3270], [g3270 >= 4.0], AC_DEFINE(G3270) )
AC_SUBST(G3270_LIBS)
AC_SUBST(G3270_CFLAGS)

AC_SUBST(OFFICE_HOME,$o3270_cv_oohome)
AC_SUBST(OO_SDK_HOME,$o3270_cv_sdkhome)
AC_SUBST(OFFICE_PROGRAM,$o3270_cv_ooprogram)

AC_DEFINE_UNQUOTED(SERVICENAME,"$o3270_cv_srvname")
AC_DEFINE_UNQUOTED(IMPLNAME,"$o3270_cv_implname")
AC_DEFINE_UNQUOTED(OFFICE_HOME,"$o3270_cv_oohome")
AC_DEFINE_UNQUOTED(OFFICE_PROGRAM,"$o3270_cv_ooprogram")


# Fixed values
AC_SUBST(CXXFLAGS)

#dnl Configure which files to generate.
AC_OUTPUT(Makefile)
AC_OUTPUT(OpenOffice-3270.spec)

dnl Output the generated config.status script.
AC_OUTPUT

