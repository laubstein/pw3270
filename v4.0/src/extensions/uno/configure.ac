

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

# Target options

DLLEXT=".so"
OS_CFLAGS="-DUNX -DGCC -DLINUX=1"
OS_DLLFLAGS="-shared -Wl,-soname,\$(@F)"
OO_LIBS="-luno_cppu"
CPPUENV=gcc3

case "$host" in
    *-mingw32)
		DLLEXT=".dll"
		OS_CFLAGS="-DWNT -mms-bitfields"
		OS_DLLFLAGS="-Wl,--add-stdcall-alias -Wl,--enable-auto-import -shared -Wl,-soname,\$(@F)"
		OO_LIBS="-lisal -lcppu3"

esac

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

AC_ARG_WITH([idlc-program],
	[AS_HELP_STRING([--with-idlc-program], [Define IDLC program command-line])],
[
	o3270_cv_idlc="$withval"
],[
	o3270_cv_idlc="LD_LIBRARY_PATH=\$(OFFICE_HOME)/ure/lib/ \$(OO_SDK_HOME)/bin/idlc"
])

AC_ARG_WITH([cppumaker-program],
	[AS_HELP_STRING([--with-cppumaker-program], [Define cppumaker program command-line])],
[
	o3270_cv_cppumaker="$withval"
],[
	o3270_cv_cppumaker="LD_LIBRARY_PATH=\$(OFFICE_HOME)/ure/lib/ \$(OO_SDK_HOME)/bin/cppumaker"
])

AC_ARG_WITH([regmerge-program],
	[AS_HELP_STRING([--with-regmerge-program], [Define regmerge program command-line])],
[
	o3270_cv_regmerge="$withval"
],[
	o3270_cv_regmerge="LD_LIBRARY_PATH=\$(OFFICE_HOME)/ure/lib/ \$(OFFICE_HOME)/ure/bin/regmerge"
])

AC_ARG_WITH([regcomp-program],
	[AS_HELP_STRING([--with-regcomp-program], [Define regcomp program command-line])],
[
	o3270_cv_regcomp="$withval"
],[
	o3270_cv_regcomp="\$(OFFICE_HOME)/ure/bin/regcomp"
])

AC_ARG_WITH([types.rdb],
	[AS_HELP_STRING([--with-types.rdb], [Define types.rdb file full path])],
[
	o3270_cv_types_rdb="$withval"
],[
	o3270_cv_types_rdb="\$(OO_SDK_HOME)/bin/types.rdb"
])

AC_ARG_WITH([office-include-path],
	[AS_HELP_STRING([--with-office-include-path], [Path to the openoffice include files])],
[
	o3270_cv_ooinclude="$withval"
],[
	o3270_cv_ooinclude="\$(OO_SDK_HOME)/inc"
])

AC_ARG_WITH([office-ure-lib],
	[AS_HELP_STRING([--with-office-ure-lib], [Path to the openoffice runtime libraries])],
[
	o3270_cv_oourelib="$withval"
],[
	o3270_cv_oourelib="\$(OFFICE_HOME)/ure/lib"
])


# Checks for libraries.
PKG_CHECK_EXISTS
PKG_CHECK_MODULES( [G3270], [g3270 >= 4.0], AC_DEFINE(G3270) )
AC_SUBST(G3270_LIBS)
AC_SUBST(G3270_CFLAGS)
AC_SUBST(EXEEXT)

AC_SUBST(OFFICE_HOME,$o3270_cv_oohome)
AC_SUBST(OO_SDK_HOME,$o3270_cv_sdkhome)
AC_SUBST(OFFICE_PROGRAM,$o3270_cv_ooprogram)
AC_SUBST(OO_URE_LIB,$o3270_cv_oourelib)

AC_SUBST(IDLC,$o3270_cv_idlc)
AC_SUBST(CPPUMAKER,$o3270_cv_cppumaker)
AC_SUBST(REGMERGE,$o3270_cv_regmerge)
AC_SUBST(REGCOMP,$o3270_cv_regcomp)
AC_SUBST(TYPES_RDB,$o3270_cv_types_rdb)
AC_SUBST(OO_INCLUDE_PATH,$o3270_cv_ooinclude)

AC_DEFINE_UNQUOTED(SERVICENAME,"$o3270_cv_srvname")
AC_DEFINE_UNQUOTED(IMPLNAME,"$o3270_cv_implname")
AC_DEFINE_UNQUOTED(OFFICE_HOME,"$o3270_cv_oohome")
AC_DEFINE_UNQUOTED(OFFICE_PROGRAM,"$o3270_cv_ooprogram")

AC_SUBST(CXXFLAGS)
AC_SUBST(OS_CFLAGS)
AC_SUBST(DLLEXT)
AC_SUBST(OS_DLLFLAGS)
AC_SUBST(OO_LIBS)
AC_SUBST(CPPUENV)

#dnl Configure which files to generate.
AC_OUTPUT(Makefile)
AC_OUTPUT(OpenOffice-3270.spec)
AC_OUTPUT(manifest.xml)
AC_OUTPUT(description.xml)

dnl Output the generated config.status script.
AC_OUTPUT

