/*
* Copyright 2008, Banco do Brasil S.A.
*
* This file is part of g3270
*
* This program file is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program in a file named COPYING; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301 USA
*
* Authors:
*
* Perry Werneck<perry.werneck@gmail.com>
*
*/

#ifndef RX3270_H_INCLUDED

	#define RX3270_H_INCLUDED 1

	#include <lib3270/config.h>
	#define ENABLE_NLS
	#define GETTEXT_PACKAGE PACKAGE_NAME

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
	#define G3270_MODULE_NAME "rexx"
	#include <lib3270/config.h>
	#include <lib3270/api.h>
	#include <lib3270/plugins.h>
	#include <lib3270/localdefs.h>
	#include <lib3270/statusc.h>
	#include <lib3270/toggle.h>

	/* Include GTK stuff */
	#include <glib/gi18n-lib.h>

	#define CONFIG_GROUP "Rexx"

	/* Tools */
	int		call_rexx(const gchar *prg, const gchar *arg);
	ULONG	RetString(PRXSTRING Retstr, const char *value);
	ULONG	RetValue(PRXSTRING Retstr, int value);

	#define ReturnValue(x)  return RetValue(Retstr,x)
	#define ReturnString(x) return RetString(Retstr,x)
	#define ReturnOk()  	strcpy(Retstr->strptr,"0"); Retstr->strlength = 1; return RXFUNC_OK;

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
	ULONG APIENTRY rx3270Popup(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270InputString(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270QueryCState(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270UpdateScreen(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270Sleep(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270FindFieldAttribute(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270FindFieldLength(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270MoveCursor(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270ReadScreen(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270SendPFKey(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270WaitForChanges(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270GetCursorPosition(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270QueryScreenAttribute(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);
	ULONG APIENTRY rx3270Connect(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr);

	/* Globals */
	extern GtkWidget *g3270_topwindow;

#endif // RX3270_H_INCLUDED

