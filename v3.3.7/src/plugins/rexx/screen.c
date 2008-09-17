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

 #include "rx3270.h"
 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>

/*---[ Statics ]----------------------------------------------------------------------------------*/


/*---[ Implement ]--------------------------------------------------------------------------------*/

 ULONG APIENTRY rx3270InputString(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	Input_String((const unsigned char *) Argv[0].strptr);

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270FindFieldAttribute(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	return RetValue(Retstr,find_field_attribute(atoi(Argv[0].strptr)));
 }

 ULONG APIENTRY rx3270FindFieldLength(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	return RetValue(Retstr,find_field_length(atoi(Argv[0].strptr)));
 }

 ULONG APIENTRY rx3270MoveCursor(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	cursor_move(atoi(Argv[0].strptr));

	return RetValue(Retstr,0);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270ReadScreen                                   */
/*                                                                            */
/* Description: Read screen contents.                                         */
/*                                                                            */
/* Rexx Args:   Start position                                                */
/*              Number of chars to read                                       */
/*                                                                            */
/* Returns:	    Screen contents                                               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270ReadScreen(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int start;
 	int qtd;

	if(Argc != 2)
		return RXFUNC_BADCALL;

	start	= atoi(Argv[0].strptr);
	qtd 	= atoi(Argv[1].strptr);

	if(qtd < 1)
		return RXFUNC_BADCALL;

	if(qtd > (RXAUTOBUFLEN-1))
 		Retstr->strptr = RexxAllocateMemory(qtd+1);

	screen_read(Retstr->strptr, start, qtd);

    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }

 ULONG APIENTRY rx3270SendPFKey(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	char buffer[10];

	if(Argc != 1)
		return RXFUNC_BADCALL;

	g_snprintf(buffer,9,"%d",atoi(Argv[0].strptr));
	action_internal(PF_action, IA_DEFAULT, buffer, 0);

	ReturnValue(0);
 }

