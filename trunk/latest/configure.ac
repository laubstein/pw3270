dnl
dnl Software pw3270, desenvolvido com base nos códigos fontes do  C3270  e  X3270
dnl (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
dnl aplicativos mainframe. Registro no INPI sob o nome G3270.
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

dnl Process this file with autoconf to produce a configure script.

dnl The minimum version of autoconf required.
AC_PREREQ(2.59)

dnl Load svn revision file info
m4_include([revision.m4])

dnl Initialise automake with the package name, version and
dnl bug-reporting address.
AC_INIT([pw3270], [4.2], [perry.werneck@gmail.com])

dnl Place auxilliary scripts here.
AC_CONFIG_AUX_DIR([scripts])

dnl Put macro definitions here (though they aren't used).
AC_CONFIG_HEADER([src/include/lib3270/config.h])

dnl Initialise automake stuff.
AM_INIT_AUTOMAKE

#--[ Check for tools & Libraries ]----------------------------------------------------------------------------------------------------------------------------

AC_PROG_CXX
AC_PROG_CC

AC_PATH_TOOL([REXXCONFIG], [oorexx-config], [no])
AC_PATH_TOOL([STRIP], [strip])
AC_PATH_TOOL([MAKENSIS], [makensis], [no])
AC_PATH_TOOL([RPMBUILD], [rpmbuild], [no])

AC_PATH_TOOL([JAVAC], [javac], [no])
AC_PATH_TOOL([JAVAH], [javah], [no])

# Check for iconv
AM_ICONV

# Headers
AC_CHECK_HEADER(malloc.h, AC_DEFINE(HAVE_MALLOC_H,,[do we have malloc.h?]))

#--[ Set default options ]------------------------------------------------------------------------------------------------------------------------------------

LD=$CC
DLLEXT=".so"
OBJEXT=".o"
IMGEXT=".svg"
DLLPREFIX="lib"

OSTYPE="linux"

DBG_LDFLAGS="-L.bin/Debug/lib"
RLS_LDFLAGS="-L.bin/Release/lib -Wl,--strip-all"

DLL_CFLAGS=""
DLL_LDFLAGS="-shared -Wl,-soname,\$(@F)"

PLUGINS=""
LIBRARIES=""

app_cv_srcgui=""
app_cv_srclib=""
app_cv_imgext=".svg"

#--[ Set host based options ]---------------------------------------------------------------------------------------------------------------------------------

if test $host != $build; then
	AC_CHECK_PROGS(HOST_CC, gcc cc)
else
	HOST_CC=$CC 
fi

case "$host" in
	*-mingw32)
		DLLEXT=".dll"
		IMGEXT=".gif"
		ICOEXT=".ico"
		localedir="locale"
		app_cv_srclib="$app_cv_srclib w3misc.c winvers.c"
		app_cv_srcgui="$app_cv_srcgui resources.rc"
		;;

	*-apple-*)
		ICOEXT=".icns"
		;;

	*)
		ICOEXT=".svg"
		java_cflags="-I\$(JDK_HOME)/include/ -I\$(JDK_HOME)/include/linux"
		java_ldflags="-L\$(JAVA_LIBJVM) -Wl,-rpath,\$(JAVA_LIBJVM) -ljvm"

esac
#--[ Compiling options ]--------------------------------------------------------------------------------------------------------------------------------------

# Check for ELF symbol visibility support
# Try to compile PROGRAM, check for warnings
m4_define([GLIB_CHECK_COMPILE_WARNINGS],
[m4_ifvaln([$1], [AC_LANG_CONFTEST([$1])])dnl
rm -f conftest.$ac_objext
glib_ac_compile_save="$ac_compile"
ac_compile='$CC -c $CFLAGS $CPPFLAGS conftest.$ac_ext'
AS_IF([_AC_EVAL_STDERR($ac_compile) &&
         AC_TRY_COMMAND([(if test -s conftest.err; then false ; else true; fi)])],
      [$2],
      [echo "$as_me: failed program was:" >&AS_MESSAGE_LOG_FD
cat conftest.$ac_ext >&AS_MESSAGE_LOG_FD
m4_ifvaln([$3],[$3])dnl])
ac_compile="$glib_ac_compile_save"
rm -f conftest.$ac_objext conftest.err m4_ifval([$1], [conftest.$ac_ext])[]dnl
])# GLIB_CHECK_COMPILE_WARNINGS

AC_MSG_CHECKING(for GNUC visibility attribute)
GLIB_CHECK_COMPILE_WARNINGS([
void
__attribute__ ((visibility ("hidden")))
     f_hidden (void)
{
}
void
__attribute__ ((visibility ("internal")))
     f_internal (void)
{
}
void
__attribute__ ((visibility ("protected")))
     f_protected (void)
{
}
void
__attribute__ ((visibility ("default")))
     f_default (void)
{
}
int main (int argc, char **argv)
{
        f_hidden();
        f_internal();
        f_protected();
        f_default();
        return 0;
}
],g_have_gnuc_visibility=yes,g_have_gnuc_visibility=no)
AC_MSG_RESULT($g_have_gnuc_visibility)
if test x$g_have_gnuc_visibility = xyes; then
  AC_DEFINE(HAVE_GNUC_VISIBILITY, 1, [supports GCC visibility attributes])
fi

# Check for compiler visibility flag
AC_MSG_CHECKING(whether $CC accepts -fvisibility=hidden)
AC_CACHE_VAL(app_cv_fvisibility_ok,
[ac_save_cc="$CC"
CC="$CC -fvisibility=hidden"
AC_TRY_RUN([int main() { return 0; }],
  app_cv_fvisibility_ok=yes,
  app_cv_fvisibility_ok=no,
  app_cv_fvisibility_ok=no)
CC="$ac_save_cc"])
AC_MSG_RESULT($app_cv_fvisibility_ok)
if test $app_cv_fvisibility_ok = yes; then
  CFLAGS="$CFLAGS -fvisibility=hidden"
fi

#--[ Command-line arguments & options ]-----------------------------------------------------------------------------------------------------------------------

# Branding
AC_ARG_WITH([program-name], [AS_HELP_STRING([--with-program-name], [Program name])], [ app_cv_progname="$withval" ],[ app_cv_progname="$PACKAGE_NAME" ])
AC_ARG_WITH([logo],[AS_HELP_STRING([--with-logo], [Change default application logo])], [ app_cv_logo=$app_cv_progname$app_cv_imgext ],[ app_cv_logo=$app_cv_progname$app_cv_imgext ])

# Optional libraryes and packets
AC_ARG_WITH([ssl],   [AS_HELP_STRING( [--without-ssl],	 [do not use libssl])],	[ app_cv_ssl="$withval" ],	[ app_cv_ssl="yes" ])
AC_ARG_WITH([gnome], [AS_HELP_STRING( [--without-gnome], [do not use gnome])],	[ app_cv_gnome="$withval" ],	[ app_cv_gnome="yes" ])
AC_ARG_ENABLE([macui],[AS_HELP_STRING([--disable-macui], [Disable MAC OSX compatible UI manager ])],[ app_cv_macui="$withval" ], [ app_cv_macui="yes" ])

AC_ARG_ENABLE([rexx],
	[AS_HELP_STRING([--disable-rexx], [Build rexx extension & plugin])],
[
	app_cv_rexx="$withval"
],[

	if test $REXXCONFIG = no; then
		app_cv_rexx="no"
	else
		app_cv_rexx="yes"
	fi
])

AC_ARG_ENABLE([java],
	[AS_HELP_STRING([--disable-java], [Build java extension & plugin])],
[
	app_cv_java="$withval"
],[

	if test $JAVAC = no; then
		app_cv_java="no"
	else
		app_cv_java="yes"
	fi
])


AC_ARG_ENABLE([pic],
	[AS_HELP_STRING([--enable-pic], [enable support for PIC libraries (required to compile PW3270 on some systems)])],
[
	app_cv_pic="$withval"
],[

	if test "$host_cpu" = "x86_64"; then
		app_cv_pic="yes"
	else
		app_cv_pic="no"
	fi
])

AC_ARG_ENABLE([altscreen], [AS_HELP_STRING([--disable-altscreen], [Disable alternate screen support])],[ app_cv_altscreen="$enableval" ],[ app_cv_altscreen="yes" ])
AC_ARG_ENABLE([plugins],[AS_HELP_STRING([--disable-plugins], [disable plugin support])], [ app_cv_plugins="$enableval" ],[ app_cv_plugins="yes" ])

#--[ Check for java ]-----------------------------------------------------------------------------------------------------------------------------------------

# Check for JRE_HOME
if test "_$JRE_HOME" = "_" ; then
	JRE_HOME="$JDK_HOME/jre"
	AC_CHECK_FILE("$JDK_HOME/jre/lib/$JVM_ARCH/client/libjvm.so",JRE_HOME="/usr/lib/jvm/java/jre",)
	AC_CHECK_FILE("/usr/lib/jvm/java/jre/lib/$JVM_ARCH/client/libjvm.so",JRE_HOME="/usr/lib/jvm/java/jre",)
	AC_CHECK_FILE("/etc/alternatives/jre/lib/$JVM_ARCH/client/libjvm.so",JRE_HOME="/etc/alternatives/jre",)
	AC_CHECK_FILE("/usr/lib/jvm/java-6-sun/jre/lib/$JVM_ARCH/client/libjvm.so",JRE_HOME="/usr/lib/jvm/java-6-sun/jre",)
fi

# Check for libjvm
if test "_$JAVA_LIBJVM" = "_" ; then

	case "$host" in
		*-mingw32)
			JAVA_LIBJVM="$JRE_HOME/bin/client"
			;;

		*-apple-*)
			AC_CHECK_FILE("$JRE_HOME/lib/$JVM_ARCH/server/libjvm.so",JAVA_LIBJVM="$JRE_HOME/lib/$JVM_ARCH/server",)
			AC_CHECK_FILE("$JRE_HOME/lib/$JVM_ARCH/client/libjvm.so",JAVA_LIBJVM="$JRE_HOME/lib/$JVM_ARCH/client",)
			;;

		*)
			AC_CHECK_FILE("$JRE_HOME/lib/$JVM_ARCH/server/libjvm.so",JAVA_LIBJVM="$JRE_HOME/lib/$JVM_ARCH/server",)
			AC_CHECK_FILE("$JRE_HOME/lib/$JVM_ARCH/client/libjvm.so",JAVA_LIBJVM="$JRE_HOME/lib/$JVM_ARCH/client",)
	esac


fi

if test "_$JAVA_LIBJVM" = "_" ; then
	JAVA_LIBJVM_SO=`find $JRE_HOME/lib -name libjvm.so | head -n 1`
	JAVA_LIBJVM=`dirname "${JAVA_LIBJVM_SO}"`
fi

case $host_cpu in
     x86_64)
	JVM_ARCH=amd64
	;;
     i?86)
	JVM_ARCH=i386
	;;
     *)
	JVM_ARCH=$host_cpu
	;;
esac

#--[ Check for libs ]-----------------------------------------------------------------------------------------------------------------------------------------

PKG_CHECK_EXISTS
PKG_CHECK_MODULES( [GLIB], [glib-2.0 gthread-2.0 gmodule-2.0], AC_DEFINE(HAVE_GLIB) )
PKG_CHECK_MODULES( [GTK], [gtk+-2.0 >= 2.8], AC_DEFINE(HAVE_GTK) )
PKG_CHECK_MODULES( [CAIRO], [cairo >= 1.8], AC_DEFINE(HAVE_CAIRO) )
PKG_CHECK_MODULES( [IGEMAC], [ige-mac-integration], AC_DEFINE(HAVE_IGEMAC), AC_MSG_NOTICE([ige-mac-integration not present.]) )

if test "$app_cv_gnome" == "yes"; then
	PKG_CHECK_MODULES( [GNOME], [libgnome-2.0 libgnomeui-2.0],AC_DEFINE(HAVE_GNOME),AC_MSG_NOTICE([gnomeui not present.]) )
fi

# Check for libssl
if test "$app_cv_ssl" == "yes"; then
	PKG_CHECK_MODULES( [SSL], [libssl],AC_DEFINE(HAVE_SSL),AC_MSG_NOTICE([libssl not present.]) )
fi

#--[ Process options ]----------------------------------------------------------------------------------------------------------------------------------------

# Check for pic
if test "$app_cv_pic" == "yes"; then
	DLL_CFLAGS="$DLL_CFLAGS -fPIC"
fi

if test "$app_cv_rexx" == "yes"; then
	AC_DEFINE(HAVE_REXX)
	PLUGINS="$PLUGINS rx3270"
	LIBRARIES="$LIBRARIES librx3270"
fi

if test "$app_cv_java" == "yes"; then
	AC_DEFINE(HAVE_JAVA)
	PLUGINS="$PLUGINS j3270plugin"
	LIBRARIES="$LIBRARIES libpw3270_jni"
fi

if test "$app_cv_plugins" == "yes"; then
	AC_DEFINE([HAVE_PLUGINS])
fi

if test "$app_cv_altscreen" == "yes"; then
	AC_DEFINE([HAVE_ALTSCREEN])
fi

if test "$app_cv_macui" == "yes"; then
	app_cv_srcgui="$app_cv_srcgui actions.c toggle.c ui_parse.c topwindow.c"
	AC_DEFINE([HAVE_MACUI])
else
	app_cv_srcgui="$app_cv_srcgui topwindow1.c uiparser1.c actions1.c"
fi


#--[ Substs & Defines ]---------------------------------------------------------------------------------------------------------------------------------------

# Branding
AC_DEFINE_UNQUOTED(PROGRAM_NAME,"$app_cv_progname")
AC_SUBST(PROGRAM_NAME,"$app_cv_progname")

AC_DEFINE_UNQUOTED(PROGRAM_LOGO,"$app_cv_logo")
AC_SUBST(PROGRAM_LOGO,"$app_cv_logo")

# Define revisions
AC_DEFINE(PACKAGE_REVISION,"SVN_REV")
AC_SUBST(PACKAGE_REVISION, "SVN_REV")

AC_DEFINE(PACKAGE_RELEASE,"SVN_RELEASE")
AC_SUBST(PACKAGE_RELEASE, "SVN_RELEASE")

AC_SUBST(CC)
AC_SUBST(LD)

AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(CAIRO_LIBS)
AC_SUBST(CAIRO_CFLAGS)
AC_SUBST(GNOME_LIBS)
AC_SUBST(GNOME_CFLAGS)
AC_SUBST(SSL_LIBS)
AC_SUBST(SSL_CFLAGS)
AC_SUBST(IGEMAC_LIBS)
AC_SUBST(IGEMAC_CFLAGS)
AC_SUBST(DLL_CFLAGS)
AC_SUBST(DLL_LDFLAGS)
AC_SUBST(DLLEXT)
AC_SUBST(OBJEXT)
AC_SUBST(GUI_SRCS,"$app_cv_srcgui")
AC_SUBST(LIB_SRCS,"$app_cv_srclib")

AC_SUBST(OBJDIR,"$ac_pwd/.obj/$host")
AC_SUBST(BINDIR,"$ac_pwd/.bin")
AC_SUBST(SRCDIR,"$ac_pwd/src")

AC_SUBST(PLUGINS)
AC_SUBST(LIBRARIES)

AC_SUBST(JDK_HOME,$JDK_HOME)
AC_SUBST(JRE_HOME,$JRE_HOME)
AC_SUBST(JAVA_LIBJVM,$JAVA_LIBJVM)
AC_SUBST(JVM_ARCH,$JVM_ARCH)
AC_SUBST(JAVA_CFLAGS,$java_cflags)
AC_SUBST(JAVA_LDFLAGS,$java_ldflags)

#--[ Output files ]-------------------------------------------------------------------------------------------------------------------------------------------

AC_OUTPUT(Makefile)
AC_OUTPUT(version)
AC_OUTPUT(lib3270.pc)
AC_OUTPUT(src/include/lib3270/plugins.h)

AC_OUTPUT(src/gtk2/Makefile)
AC_OUTPUT(src/gtk2/version.c)

AC_OUTPUT(src/plugins/rexx/Makefile)
AC_OUTPUT(src/plugins/java/Makefile)

AC_OUTPUT(src/java/Makefile)

dnl Output the generated config.status script.
AC_OUTPUT

