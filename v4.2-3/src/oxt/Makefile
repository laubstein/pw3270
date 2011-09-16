#
# "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
# (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
# aplicativos mainframe. Registro no INPI sob o nome G3270.
#
# Copyright (C) <2008> <Banco do Brasil S.A.>
#
# Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
# os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
# Free Software Foundation.
#
# Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
# GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
# A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
# obter mais detalhes.
#
# Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
# programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
# Place, Suite 330, Boston, MA, 02111-1307, USA
#
# Contatos:
#
# perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
# erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
# licinio@bb.com.br		(Licínio Luis Branco)
# kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
# macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
#
#
# Makefile to build only the pw3270.oxt file with Microsoft Visual C++
#

PACKAGE ?= pw3270
OOO_SRCS = main.cxx connection.cxx screen.cxx actions.cxx status.cxx

EXEEXT = .exe
DLLEXT = .dll

#---[ pw3270 SDK paths ]-------------------------------------------------------------------------------------
PW3270_ICON ?= ..\..\image\default.png
PW3270_SDK_LIB ?= ..\..\bin\Release
PW3270_SDK_INCLUDE ?= ..\include

#---[ MSVC paths ]-------------------------------------------------------------------------------------------
VCINSTALLDIR ?= C:\Arquivos de programas\Microsoft Visual Studio 9.0\VC

#---[ openOffice paths ]-------------------------------------------------------------------------------------
OFFICE_HOME ?= C:\Arquivos de programas\OpenOffice.org 3
OO_SDK_HOME ?= C:\Arquivos de programas\OpenOffice.org_3.1_SDK\sdk
OO_SDK_URE_HOME ?= $(OFFICE_HOME)\ure
OO_SDK_ZIP_HOME ?= C:\Arquivos de programas\GnuWin32\bin

#---[ Tools ]------------------------------------------------------------------------------------------------
ZIP="$(OO_SDK_ZIP_HOME)\zip.exe"
RMDIR=rd /s /q

#---[ Compiler and linker ]----------------------------------------------------------------------------------
CXX ?= cl.exe

#---[ from settings.mk ]-------------------------------------------------------------------------------------
# TODO (perry#2#): Include settings.mk directly
CC_FLAGS=-c -MT -Zm500 -Zc:forScope,wchar_t- -wd4251 -wd4275 -wd4290 -wd4675 -wd4786 -wd4800 -Zc:forScope -GR -EHa
CC_DEFINES=-DWIN32 -DWNT -DCPPU_ENV=msci
LIBRARY_LINK_FLAGS=/DLL /DEBUGTYPE:cv
# /NODEFAULTLIB

SALLIB=isal.lib
CPPULIB=icppu.lib
CPPUHELPERLIB=icppuhelper.lib
SALHELPERLIB=isalhelper.lib
REGLIB=ireg.lib
STORELIB=istore.lib
STLPORTLIB=stlport_vc71.lib

#---[ Build options ]----------------------------------------------------------------------------------------

CXXFLAGS = /nologo $(CC_FLAGS) $(CC_DEFINES) /I"$(OO_SDK_HOME)\include" /I"$(PW3270_SDK_INCLUDE)" /I.

#---[ Tools ]------------------------------------------------------------------------------------------------
OO_SDK_URE_HOME ?= $(OFFICE_HOME)\ure

IDLC ?= "$(OO_SDK_HOME)\bin\idlc"
REGMERGE ?= "$(OO_SDK_URE_HOME)\bin\regmerge"
CPPUMAKER ?= "$(OO_SDK_HOME)\bin\cppumaker"

TYPES_RDB ?= "$(OFFICE_HOME)\ure\misc\types.rdb"

####################################################################################################
# Rules
####################################################################################################

%.obj: %.cxx *.hpp
	@cl.exe $(CXXFLAGS) $<

%.urd: %.idl
	@echo $< ...
	@$(IDLC) -C -I"$(OO_SDK_HOME)\idl" -O. $<

%.uno.rdb : %.urd
	@echo $< ...
	$(REGMERGE) --verbose $@ / $(TYPES_RDB)
	$(REGMERGE) --verbose $@ /UCR $<
	$(REGMERGE) --verbose $@ / $<

%.hpp: %.uno.rdb
	@echo $< ...
	@$(CPPUMAKER) -O. -Gc -L -BUCR $<

%.lib: %.def
	@echo $< ...
	@lib /def:$< /out:$@

####################################################################################################
# Targets
####################################################################################################

all: $(PACKAGE).oxt

clean:
	@del $(PACKAGE).oxt > NUL: 2>&1
	@del $(PACKAGE).uno$(DLLEXT) > NUL: 2>&1
	@del $(PACKAGE).uno.lnk > NUL: 2>&1
	@del $(PACKAGE).uno.map > NUL: 2>&1
	@del $(PACKAGE).uno.rdb > NUL: 2>&1
	@del $(PACKAGE).uno.lib > NUL: 2>&1
	@del $(PACKAGE).uno.exp > NUL: 2>&1
	@del $(PACKAGE).urd  > NUL: 2>&1
	@del *.obj  > NUL: 2>&1

%.uno.lnk: Makefile
	@echo $@ ...

	@echo /LIBPATH:"$(OO_SDK_HOME)\lib" > $@
	@echo /LIBPATH:"$(VCINSTALLDIR)\lib" >> $@

	@echo /DEFAULTLIB:$(CPPULIB) >> $@
	@echo /DEFAULTLIB:$(CPPUHELPERLIB) >> $@
	@echo /DEFAULTLIB:$(REGLIB) >> $@
	@echo /DEFAULTLIB:irmcxt.lib >> $@
	@echo /DEFAULTLIB:$(SALLIB) >> $@
	@echo /DEFAULTLIB:$(SALHELPERLIB) >> $@
	@echo /DEFAULTLIB:$(STORELIB) >> $@
	@echo /DEFAULTLIB:$(STLPORTLIB) >> $@
	@echo /DEFAULTLIB:$(PW3270_SDK_LIB)\lib3270.lib >> $@

	@echo $(foreach SRC, $(basename $(OOO_SRCS)), $(SRC).obj) >> $@

run: testprogram$(EXEEXT)
	@.\testprogram$(EXEEXT)

testprogram$(EXEEXT): testprogram.obj $(PACKAGE).uno$(DLLEXT)
	@echo $@ ...
	@link /WX /NOLOGO /LIBPATH:"$(OO_SDK_HOME)\lib" /LIBPATH:"$(VCINSTALLDIR)\lib" /DEFAULTLIB:icppu.lib /DEFAULTLIB:icppuhelper.lib /DEFAULTLIB:ireg.lib /DEFAULTLIB:irmcxt.lib /DEFAULTLIB:isal.lib /DEFAULTLIB:isalhelper.lib /DEFAULTLIB:istore.lib /DEFAULTLIB:stlport_vc71.lib /out:$@ testprogram.obj

mkwinfiles$(EXEEXT): mkwinfiles.obj
	@echo $@ ...
	@link /WX /NOLOGO /out:$@ mkwinfiles.obj

$(PACKAGE).uno$(DLLEXT): $(PW3270_SDK_LIB)\lib3270.lib $(PACKAGE).uno.lnk $(PACKAGE).hpp $(PACKAGE).uno.rdb $(foreach SRC, $(basename $(OOO_SRCS)), $(SRC).obj)
	@echo $< ...
	@link $(LIBRARY_LINK_FLAGS) /NOLOGO /out:$@ /def:"$(OO_SDK_HOME)\settings\component.uno.def" /map:$(basename $@).map @$(PACKAGE).uno.lnk

$(PACKAGE).oxt: $(PACKAGE).uno$(DLLEXT) mkwinfiles$(EXEEXT) $(PACKAGE).uno.rdb description.xml manifest.xml description.txt
	@echo $@ ...
	@md oxt.tmp
	@md oxt.tmp\META-INF
	@mkwinfiles$(EXEEXT) "oxt.tmp\META-INF\manifest.xml" "oxt.tmp\description.xml"
	@copy description.txt oxt.tmp
	@copy $(PW3270_ICON) oxt.tmp\pw3270.png
	@mkdir oxt.tmp\windows
	@copy pw3270.uno$(DLLEXT) oxt.tmp\windows
	@copy pw3270.uno.rdb oxt.tmp
	@cd oxt.tmp & $(ZIP) -m -r ..\$@ .
	@$(RMDIR) oxt.tmp
	@echo $@ Ok.

install: $(PACKAGE).oxt
	"$(OFFICE_HOME)\program\unopkg" add $(PACKAGE).oxt

uninstall: $(PACKAGE).oxt
	"$(OFFICE_HOME)\program\unopkg remove" $(PACKAGE).oxt
