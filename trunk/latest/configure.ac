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


# https://stage.maemo.org/svn/maemo/projects/haf/branches/maemo-af-desktop/hildon-desktop/configure.ac
# http://svn.abisource.com/libabiword-builder/trunk/configure.in

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

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

# Check for iconv
AM_ICONV

# Target options
DLLEXT=".so"

OS_CFLAGS=""
DLL_CFLAGS=""
API_CFLAGS="-DLIB3270=1 -Isrc/lib -Isrc/include/lib3270"
APP_CFLAGS="-Isrc/lib"

OS_DLLFLAGS="-shared -Wl,-soname,\$(@F)"
OS_LFLAGS=""
OS_SRCS=""
OS_APPSRCS=""
LOGOEXT="png"
LD_APIFLAGS=""
LD_RLSFLAGS="-Wl,--strip-all"
LD=$CC
PLUGINS=""
TARGETS=""
PLUGINS_UI=""
REXX_SYMBOL_HACK=""
EXTENSIONS=""
CPPUENV="gcc3"
INSTALL_TARGETS=""
JAVAC="\$(JDK_HOME)/bin/javac"
JAVAH="\$(JDK_HOME)/bin/javah"

AC_CHECK_PROG(PACKAGE_MANAGER, "rpmbuild", "rpm")
AC_CHECK_PROG(PACKAGE_MANAGER, "dpkg-buildpackage","deb")

case "$host" in
	*-mingw32)
		DLLEXT=".dll"
		PACKAGE_MANAGER="nsi"
		PLUGINS="pipectl $PLUGINS"
		OS_CFLAGS="-mno-cygwin -mms-bitfields -D_WIN32 -DWC3270 -D_WIN32_WINNT=0x0500"
		OS_DLLFLAGS="-mno-cygwin -shared -Wl,--add-stdcall-alias,--enable-auto-import,-soname,\$(@F)"
		REXX_SYMBOL_HACK="-Wl,--exclude-symbols,REXXAPI_NULL_THUNK_DATA -Wl,--exclude-symbols,rexx_NULL_THUNK_DATA"

		# http://oldwiki.mingw.org/index.php/MSVC-MinGW-DLL
		LD_APIFLAGS="-Wl,--warn-duplicate-exports,--output-def,\$(basename \$(@F)).def,--out-implib,\$(basename \$(@F)).a"

		LD_RLSFLAGS="-mwindows $LD_RLSFLAGS"
		OS_LFLAGS="-lws2_32"
		OS_APISRCS="w3misc.c winvers.c"
		OS_APPSRCS="resources.rc"
		LOGOEXT="gif"
		localedir="locale"

		java_cflags="-I\$(JDK_HOME)/include/ -I\$(JDK_HOME)/include/win32"
		java_ldflags="-Wl,--exclude-symbols,jvm_NULL_THUNK_DATA -L\$(JDK_HOME)/lib -ljvm"

		# Check for native javac
		JAVAC="javac"

		# Check for native javah
		JAVAH="javah"

		;;

	*-apple-*)
		OS_DLLFLAGS="-arch i386 -shared"
		OS_LFLAGS="-arch i386"

	;;

	*)
		# Check for OpenOffice
		AC_CHECK_FILE("/opt/openoffice.org/basis3.0/program/unorc",oo_home="/opt/openoffice.org",oo_home="none")
		AC_CHECK_FILE("/opt/broffice.org3/program/soffice.bin",oo_home="/opt/broffice.org3",)
		AC_CHECK_FILE("/usr/lib/ooo3/program/soffice.bin",oo_home="/usr/lib/ooo3",)
		AC_CHECK_FILE("/usr/lib/openoffice/program/soffice.bin",oo_home="/usr/lib/openoffice/",)

		# Check for OpenOffice URE
		AC_CHECK_FILE("/opt/openoffice.org/ure/lib/unorc",oo_ure_home="/opt/openoffice.org/ure",oo_ure_home="none")
		AC_CHECK_FILE("/usr/lib/ure/lib/unorc",oo_ure_home="/usr/lib/ure",)

		# Check for types.rdb
		AC_CHECK_FILE("$oo_ure_home/share/misc/types.rdb",types_rdb=$oo_ure_home/share/misc/types.rdb,types_rdb="no")
		AC_CHECK_FILE("$oo_home/solver/bin/types.rdb", types_rdb=$oo_home/solver/bin/types.rdb,)

		# Check for OpenOffice SDK
		AC_CHECK_FILE("/opt/openoffice.org/basis3.0/sdk/bin/idlc",oo_sdk_home="/opt/openoffice.org/basis3.0/sdk",oo_sdk_home="none")
		AC_CHECK_FILE("/opt/openoffice.org/basis3.1/sdk/bin/idlc",oo_sdk_home="/opt/openoffice.org/basis3.1/sdk",)
		AC_CHECK_FILE("/usr/lib/openoffice/basis3.2/sdk/bin/idlc",oo_sdk_home="/usr/lib/openoffice/basis3.2/sdk",)

		# Check for Java
		java_cflags="-I\$(JDK_HOME)/include/ -I\$(JDK_HOME)/include/linux"
		java_ldflags="-L\$(JRE_HOME)/lib/i386/client -Wl,-rpath,\$(JRE_HOME)/lib/i386/client -ljvm"

		# Check for ImageMagick


esac

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
AC_CACHE_VAL(pw3270_cv_fvisibility_ok,
[ac_save_cc="$CC"
CC="$CC -fvisibility=hidden"
AC_TRY_RUN([int main() { return 0; }],
  pw3270_cv_fvisibility_ok=yes,
  pw3270_cv_fvisibility_ok=no,
  pw3270_cv_fvisibility_ok=no)
CC="$ac_save_cc"])
AC_MSG_RESULT($pw3270_cv_fvisibility_ok)
if test $pw3270_cv_fvisibility_ok = yes; then
  APP_CFLAGS="$APP_CFLAGS -fvisibility=hidden"
fi

# Optional
AC_ARG_ENABLE([trace],
	[AS_HELP_STRING([--disable-trace], [disable trace])],
[
	pw3270_cv_trace="$enableval"
],[
	pw3270_cv_trace="yes"
])

AC_ARG_ENABLE([plugins],
	[AS_HELP_STRING([--disable-plugins], [disable plugin support])],
[
	pw3270_cv_plugins="$enableval"

],[
	pw3270_cv_plugins="yes"
])

AC_ARG_ENABLE([print-font-dialog],
	[AS_HELP_STRING([--disable-print-font-dialog], [disable font selection dialog on print jobs])],
[
	pw3270_cv_print_font="$enableval"

],[
	pw3270_cv_print_font="yes"
])


AC_ARG_WITH([ssl],
	[AS_HELP_STRING([--without-ssl], [do not use libssl])],
[
	pw3270_cv_ssl="$withval"
],[
	pw3270_cv_ssl="yes"
])

AC_ARG_ENABLE([mouse-pointer-change],
	[AS_HELP_STRING([--disable-mouse-pointer-change], [disable mouse pointer changes])],
[
	pw3270_cv_pointer_change="$enableval"
],[
	pw3270_cv_pointer_change="yes"
])

AC_ARG_WITH([rexx-sdk],
	[AS_HELP_STRING([--with-rexx-sdk], [Set Rexx SDK root path])],
[
	pw3270_cv_rexx="$withval"
],[
	AC_CHECK_PROGS(REXX_CONFIG, oorexx-config)

	if test -n "$REXX_CONFIG"; then
		pw3270_cv_rexx="yes"
	else
		pw3270_cv_rexx="no"
	fi
])

AC_ARG_WITH([gnome],
	[AS_HELP_STRING([--without-gnome], [do not use libgnome])],
[
	pw3270_cv_gnome="$withval"
],[
	pw3270_cv_gnome="yes"
])

AC_ARG_WITH([icon],
	[AS_HELP_STRING([--with-icon], [set icon filename])],
[
	pw3270_cv_icon="$withval"
	AC_DEFINE_UNQUOTED(PROGRAM_ICON,"$pw3270_cv_icon")
],[
	pw3270_cv_icon="image/default.ico"
])

AC_ARG_WITH([logo],
	[AS_HELP_STRING([--with-logo], [set logo filename])],
[
	pw3270_cv_logo="$withval"
	AC_DEFINE_UNQUOTED(PROGRAM_LOGO,"$pw3270_cv_logo")

],[
	pw3270_cv_logo="image/default.png"
])

AC_ARG_WITH([gtk-runtime],
	[AS_HELP_STRING([--with-gtk-runtime], [GTK runtime path (For windows installer)])],
[
	pw3270_cv_gtkruntime="$withval"
],[
	pw3270_cv_gtkruntime="no"
])

AC_ARG_WITH([program-name],
	[AS_HELP_STRING([--with-program-name], [Program name (For windows installer)])],
[
	pw3270_cv_progname="$withval"
],[
	pw3270_cv_progname="$PACKAGE_NAME"
])

AC_ARG_WITH([default-config],
	[AS_HELP_STRING([--with-default-config], [Default configuration file])],
[
	pw3270_cv_config="$withval"
],[
	pw3270_cv_config="default.conf"
])

AC_ARG_WITH([ui-files],
	[AS_HELP_STRING([--with-ui-files], [User interface files])],
[
	pw3270_cv_ui="$withval"
],[
	pw3270_cv_ui="ui/default.xml ui/functions.xml ui/keypad.xml ui/debug.xml"
])

AC_ARG_WITH([pic],
	[AS_HELP_STRING([--with-pic], [enable support for PIC libraries (required to compile PW3270 on some systems)])],
[
	pw3270_cv_pic="$withval"
],[

	if test "$host_cpu" = "x86_64"; then
		pw3270_cv_pic="yes"
	else
		pw3270_cv_pic="no"
	fi


])

#
# Check for java options
#
AC_ARG_WITH([jdk-home],
	[AS_HELP_STRING([--with-jdk-home], [Define java sdk home])],
[
	JDK_HOME="$withval"
],[
	AC_CHECK_FILE("/usr/lib/jvm/java-6-sun/bin/javac",JDK_HOME="/usr/lib/jvm/java-6-sun",)
] )

AC_ARG_WITH([jre-home],
	[AS_HELP_STRING([--with-jre-home], [Define java runtime home])],
[
	JRE_HOME="$withval"
], )

AC_ARG_ENABLE([java],
	[AS_HELP_STRING([--disable-java], [disable building of java plugin])],
[
	pw3270_cv_java="$enableval"
],[
	AC_CHECK_FILE("$JDK_HOME/include/jni.h",pw3270_cv_java="yes",pw3270_cv_java="no")
])

AC_ARG_WITH([jnidir],
	[AS_HELP_STRING([--with-jnidir], [jni libraries path])],
[
	jnidir="$withval"
],[
	jnidir="\$(LIBDIR)/java"
] )

AC_ARG_WITH([jvmjardir],
	[AS_HELP_STRING([--with-jvmjardir], [JAR files installation path])],
[
	jardir="$withval"
],[
	jardir="\$(LIBDIR)/jvm-exports"
] )


#
# Check for OpenOffice.org
#
AC_ARG_WITH([office-home],
	[AS_HELP_STRING([--with-office-home], [Define OpenOffice.org program home])],
[
	oo_home="$withval"
],)

AC_ARG_WITH([office-program],
	[AS_HELP_STRING([--with-office-program], [Define OpenOffice.org main program])],
[
	oo_program="$withval"
],[
	oo_program="$oo_home/program/soffice.bin"
])

AC_ARG_WITH([office-sdk-home],
	[AS_HELP_STRING([--with-office-sdk-home], [Define Open-Office sdk home])],
[
	oo_sdk_home="$withval"
], )

AC_ARG_WITH([office-ure-home],
	[AS_HELP_STRING([--with-office-ure-home], [Define Open-Office ure home])],
[
	oo_ure_home="$withval"
] )


AC_ARG_WITH([office-types],
	[AS_HELP_STRING([--with-office-types], [Define Open-Office types.rdb file])],
[
	types_rdb="$withval"
] )

if test "$oo_sdk_home" != "none"; then

	AC_PATH_PROG([UNOPKG], unopkg, [$oo_home/program/unopkg] )
	AC_PATH_PROG([IDLC], idlc, "no", [$PATH$PATH_SEPARATOR$oo_sdk_home/bin])
	AC_PATH_PROG([CPPUMAKER], cppumaker, "no", [$PATH$PATH_SEPARATOR$oo_sdk_home/bin])
	AC_PATH_PROG([REGMERGE], regmerge, "no", [$PATH$PATH_SEPARATOR$oo_home/ure/bin$PATH_SEPARATOR$oo_ure_home/bin])
	AC_PATH_PROG([REGCOMP], regcomp, "no", [$PATH$PATH_SEPARATOR$oo_home/ure/bin$PATH_SEPARATOR$oo_ure_home/bin])

	AC_SUBST(OO_SDK_HOME,$oo_sdk_home)
	AC_SUBST(OO_URE_HOME,$oo_ure_home)
	AC_SUBST(OO_HOME,$oo_home)
	AC_SUBST(TYPES_RDB,$types_rdb)
	AC_SUBST(CPPUENV)
	AC_SUBST(PACKAGE_MANAGER)

	AC_DEFINE_UNQUOTED(OFFICE_HOME,"$oo_home")
	AC_DEFINE_UNQUOTED(OFFICE_PROGRAM,"$oo_program")

	INSTALL_TARGETS="$INSTALL_TARGETS install-oxt"

fi

# Checks for libraries.
PKG_CHECK_EXISTS
PKG_CHECK_MODULES( [GTK], [glib-2.0 gtk+-2.0 >= 2.8 gthread-2.0 gthread-2.0 gmodule-2.0], AC_DEFINE(HAVE_GTK) )
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_CFLAGS)

# Check for GNOME
if test "$pw3270_cv_gnome" == "yes"; then
	PKG_CHECK_MODULES( [LIBGNOME], [libgnome-2.0 libgnomeui-2.0],
					AC_DEFINE(HAVE_LIBGNOME),
					AC_MSG_NOTICE([gnomeui not present.]) )
fi
AC_SUBST(LIBGNOME_LIBS)
AC_SUBST(LIBGNOME_CFLAGS)

# Check for pic
if test "$pw3270_cv_pic" == "yes"; then
	DLL_CFLAGS="$DLL_CFLAGS -fPIC"
fi
AC_SUBST(DLL_CFLAGS)

# Check for libssl
if test "$pw3270_cv_ssl" == "yes"; then
	PKG_CHECK_MODULES( [LIBSSL], [libssl],
					AC_DEFINE(HAVE_LIBSSL),
					AC_MSG_NOTICE([libssl not present.]) )
fi

AC_SUBST(LIBSSL_LIBS)
AC_SUBST(LIBSSL_CFLAGS)

# Check for rexx devel
if test "$pw3270_cv_rexx" == "yes"; then
	AC_DEFINE(HAVE_REXX)
	REXX_CFLAGS="`$REXX_CONFIG --cflags`"
	REXX_LIBS="`$REXX_CONFIG --libs`"
	PLUGINS="rx3270 $PLUGINS"
	PLUGINS_UI="rexx.xml $PLUGINS_UI"
	EXTENSIONS="librx3270 $EXTENSIONS"
	INSTALL_TARGETS="$INSTALL_TARGETS install-rexx-plugin"
elif test "$pw3270_cv_rexx" != "no"; then
	AC_DEFINE(HAVE_REXX)
	REXX_CFLAGS="-I$pw3270_cv_rexx/api"
	REXX_LIBS="-L$pw3270_cv_rexx/api -lrexx -lrexxapi"
	PLUGINS="rx3270 $PLUGINS"
	PLUGINS_UI="rexx.xml $PLUGINS_UI"
	EXTENSIONS="librx3270 $EXTENSIONS"
	INSTALL_TARGETS="$INSTALL_TARGETS install-rexx-plugin"
fi

AC_SUBST(REXX_LIBS)
AC_SUBST(REXX_CFLAGS)
AC_SUBST(REXX_CONFIG)
AC_SUBST(REXX_SYMBOL_HACK)

# Other options

if test "$pw3270_cv_trace" == "yes"; then
	AC_DEFINE([X3270_TRACE])
fi

if test "$pw3270_cv_plugins" == "yes"; then
	AC_DEFINE([HAVE_PLUGINS])
	AC_SUBST(PLUGINS)
	AC_SUBST(PLUGINS_UI)
fi

if test "$pw3270_cv_print_font" == "yes"; then
	AC_DEFINE([HAVE_PRINT_FONT_DIALOG])
fi

if test "$pw3270_cv_pointer_change" == "yes"; then
	AC_DEFINE([MOUSE_POINTER_CHANGE])
fi

if test "$pw3270_cv_use_selections" == "yes"; then
	AC_DEFINE([USE_SELECTIONS])
fi

if test "$pw3270_cv_use_primary_selection" == "yes"; then
	AC_DEFINE([USE_PRIMARY_SELECTION])
fi

if test "$pw3270_cv_java" == "yes"; then
	AC_DEFINE([HAVE_JAVA])
	PLUGINS="j3270plugin $PLUGINS"
	TARGETS="$TARGETS libpw3270_jni\$(DLLEXT) pw3270.jar"
	INSTALL_TARGETS="$INSTALL_TARGETS install-java-plugin"
fi

if test "_$HOST_CC" = "_" ; then
	HOST_CC="\$(CC)"
fi

# Check for ImageMagick
AC_CHECK_PROG(CONVERT, convert, convert, scripts/convert-sh)
AC_SUBST(CONVERT)

# Define program name
AC_DEFINE_UNQUOTED(PROGRAM_NAME,"$pw3270_cv_progname")

AC_DEFINE([X3270_TN3270E])
AC_DEFINE([X3270_ANSI])
AC_DEFINE([X3270_APL])
AC_DEFINE([X3270_FT])
AC_DEFINE([X3270_PRINTER])

AC_DEFINE(PACKAGE_REVISION,"SVN_REV")
AC_SUBST(PACKAGE_REVISION, "SVN_REV")

AC_DEFINE(PACKAGE_RELEASE,"SVN_RELEASE")
AC_SUBST(PACKAGE_RELEASE, "SVN_RELEASE")

AC_SUBST(ICONFILE,$pw3270_cv_icon)
AC_SUBST(LOGOFILE,$pw3270_cv_logo)

AC_SUBST(PROGRAM_NAME,"$pw3270_cv_progname")
AC_SUBST(DEFAULT_CONFIG,"$pw3270_cv_config")
AC_SUBST(UI_XML_FILES,"$pw3270_cv_ui")

AC_SUBST(LD)
AC_SUBST(HOST_CC)
AC_SUBST(OS_LFLAGS)

AC_SUBST(OS_CFLAGS)
AC_SUBST(API_CFLAGS)
AC_SUBST(APP_CFLAGS)

AC_SUBST(OS_DLLFLAGS)
AC_SUBST(LD_APIFLAGS)
AC_SUBST(LD_RLSFLAGS)
AC_SUBST(OS_SRCS)
AC_SUBST(DLLEXT)
AC_SUBST(EXEEXT)
AC_SUBST(TARGETS)

AC_DEFINE_UNQUOTED(LOGOEXT,"$LOGOEXT")
AC_SUBST(LOGOEXT)

AC_SUBST(JDK_HOME,$JDK_HOME)

if test "_$JRE_HOME" = "_" ; then
	AC_CHECK_FILE("$JDK_HOME/jre/lib/i386/client/libjvm.so",JRE_HOME="/usr/lib/jvm/java/jre",)
	AC_CHECK_FILE("/usr/lib/jvm/java/jre/lib/i386/client/libjvm.so",JRE_HOME="/usr/lib/jvm/java/jre",)
	AC_CHECK_FILE("/etc/alternatives/jre/lib/i386/client/libjvm.so",JRE_HOME="/etc/alternatives/jre",)
	AC_CHECK_FILE("/usr/lib/jvm/java-6-sun/jre/lib/i386/client/libjvm.so",JRE_HOME="/usr/lib/jvm/java-6-sun/jre",)
fi

AC_SUBST(JRE_HOME,$JRE_HOME)

AC_SUBST(JAVA_CFLAGS,$java_cflags)
AC_SUBST(JAVA_LDFLAGS,$java_ldflags)

AC_SUBST(EXTENSIONS)
AC_SUBST(WINDRES)
AC_SUBST(libdir)
AC_SUBST(jnidir)
AC_SUBST(jardir)
AC_SUBST(JAVAC)
AC_SUBST(JAVAH)
AC_SUBST(OS_APPSRCS)
AC_SUBST(OS_APISRCS)
AC_SUBST(GTKRUNTIME,$pw3270_cv_gtkruntime)
AC_SUBST(INSTALL_TARGETS)

#dnl Configure which files to generate.
AC_OUTPUT(Makefile)
AC_OUTPUT(src/gui/version.c)
AC_OUTPUT(src/oxt/description.xml)
AC_OUTPUT(src/oxt/manifest.xml)
AC_OUTPUT(src/include/lib3270/plugins.h)
AC_OUTPUT(version)

case "$host" in
	*-mingw32)
		AC_OUTPUT(pw3270.nsi)
		AC_OUTPUT(src/gui/resources.rc)
	;;

	*)
		AC_OUTPUT(pw3270.spec)
		AC_OUTPUT(debian/control)
		AC_OUTPUT(debian/changelog)
		AC_OUTPUT(lib3270.pc)

	;;
esac

dnl Output the generated config.status script.
AC_OUTPUT
