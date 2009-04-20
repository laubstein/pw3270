/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

#ifndef RX3270_H_INCLUDED

	#define RX3270_H_INCLUDED 1

	#include <lib3270/config.h>
	#define ENABLE_NLS
	#define GETTEXT_PACKAGE PACKAGE_NAME

	#include <libintl.h>
	#include <glib/gi18n.h>

	#define RX3270_DEFAULT_TIMEOUT 60

	/* include the REXX stuff */
	#define RXFUNC_BADCALL 40
	// #define INCL_REXXSAA	/* Complete Rexx support */
	// #define INCL_RXSUBCOM	/* Rexx subcommand handler support */
	// #define INCL_RXSHV		/* Rexx shared variable pool support */
	#define INCL_RXFUNC	/* Rexx external function support */
	// #define INCL_RXSYSEXIT	/* Rexx system exit support */
	// #define INCL_RXARI		/* Rexx asynchronous Trace/Halt support */
	#include <rexx.h>

	/* include the "C" stuff */
	#include <string.h>
	#include <sys/time.h>                   /* System time-related data types */
	#include <stdlib.h>
	#include <unistd.h>
	#include <errno.h>

	/* include the lib3270 stuff */
	#define LIB3270_MODULE_NAME "rexx"
	#include <lib3270/config.h>
	#include <lib3270/api.h>
	#include <lib3270/plugins.h>
	#include <lib3270/localdefs.h>
	#include <lib3270/statusc.h>
	#include <lib3270/toggle.h>

	#define CONFIG_GROUP "Rexx"

	#ifndef ENOTCONN
		#define ENOTCONN -1
	#endif

	/* Tools */
	int		call_rexx(const gchar *prg, const gchar *arg);
	ULONG	RetString(PRXSTRING Retstr, const char *value);
	ULONG	RetValue(PRXSTRING Retstr, int value);
	ULONG 	RetPointer(PRXSTRING Retstr, gpointer value);

	#define ReturnValue(x)  	return RetValue(Retstr,x)
	#define ReturnString(x)		return RetString(Retstr,x)
	#define ReturnPointer(x)	return RetPointer(Retstr,x)
	#define ReturnOk()			strcpy(Retstr->strptr,"0"); Retstr->strlength = 1; return RXFUNC_OK;

	/* Rexx entry points */

	ULONG APIENTRY rx3270Version(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270LoadFuncs(PSZ Name, LONG Argc, RXSTRING Argv[], PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270Init(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270Actions(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270ToggleON(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270ToggleOFF(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270Toggle(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270Toggled(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270Log(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270QueryCState(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270WaitForEvents(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270Sleep(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270WaitForChanges(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270Quit(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270SetVisible(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);

	ULONG APIENTRY rx3270Dunno(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);

	// Network
	ULONG APIENTRY rx3270Connect(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270Disconnect(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);

	// Screen
	ULONG APIENTRY rx3270GetCursorPosition(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270SetCursorPosition(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);

	ULONG APIENTRY rx3270QueryScreenAttribute(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270FindFieldAttribute(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270FindFieldLength(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270ReadScreen(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270WaitForString(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270WaitForStringAt(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270IsTerminalReady(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270WaitForTerminalReady(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);

	// Keyboard
	ULONG APIENTRY rx3270InputString(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270SendPFKey(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270SendENTERKey(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);

	// GUI
	ULONG APIENTRY rx3270Popup(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270runDialog(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270DestroyDialog(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270FileChooserNew(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270FileChooserGetFilename(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270SetDialogTitle(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);

	/* Globals */
	extern GtkWidget *program_window;

#endif // RX3270_H_INCLUDED

