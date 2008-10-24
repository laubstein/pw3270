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
 #include <errno.h>
 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>

#ifndef ENOTCONN
    #define ENOTCONN -1
#endif

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

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270MoveCursor                                   */
/*                                                                            */
/* Description: Set cursor position.                                          */
/*                                                                            */
/* Rexx Args:   New cursor position                                           */
/*                                                                            */
/* Rexx Args:   New cursor row                                                */
/*              New cursor col                                                */
/*                                                                            */
/* Returns:	    Original cursor position                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/
ULONG APIENTRY rx3270MoveCursor(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
{
    int rc = -1;

    switch(Argc)
    {
    case 0:
        rc = cursor_get_addr();
        break;

    case 1:
        rc = cursor_set_addr(atoi(Argv[0].strptr));
        break;

    case 2:
        rc = cursor_set_addr((atoi(Argv[0].strptr) * ctlr_get_cols()) + atoi(Argv[1].strptr));
        break;

    default:
		return RXFUNC_BADCALL;
    }

	ReturnValue(rc);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270GetCursorPosition                            */
/*                                                                            */
/* Description: Get cursor position.                                          */
/*                                                                            */
/* Rexx Args:   None                                                          */
/*                                                                            */
/* Returns:	    Current cursor position                                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/
ULONG APIENTRY rx3270GetCursorPosition(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
{
    if(Argc)
		return RXFUNC_BADCALL;

	ReturnValue(cursor_get_addr());
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270ReadScreen                                   */
/*                                                                            */
/* Description: Read screen contents.                                         */
/*                                                                            */
/* Rexx Args:	NONE to read the entire screen                                */
/*                                                                            */
/* Rexx Args:   Number of chars to read from cursor position                  */
/*                                                                            */
/* Rexx Args:   Start position                                                */
/*              Number of chars to read                                       */
/*                                                                            */
/* Rexx Args:   Start Row                                                     */
/*				Start Col													  */
/*              Number of chars to read                                       */
/*                                                                            */
/* Returns:	    Screen contents                                               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270ReadScreen(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int start, qtd, rows, cols, row, col;

	switch(Argc)
	{
	case 0:	// Get entire screen
		screen_size(&rows,&cols);
		qtd = (rows*(cols+1)+1);
		Retstr->strptr = RexxAllocateMemory(qtd);

		Trace("Screen buffer size: %d (%dx%d)",qtd,rows,cols);

		memset(Retstr->strptr,0,qtd);
		start = qtd = 0;
		for(row = 0; row < rows;row++)
		{
			screen_read(Retstr->strptr+qtd,start,cols);
			qtd += cols;
			start += cols;
			Retstr->strptr[qtd++] = '\n';
		}
		Retstr->strptr[qtd] = 0;

		Trace("Bytes read: %d",qtd);

		Retstr->strlength = strlen(Retstr->strptr);

		return RXFUNC_OK;

	case 1:	// Just size, get current cursor position
		start	= 0;
		qtd 	= atoi(Argv[0].strptr);
		break;

	case 2:	// Use start position
		start	= atoi(Argv[0].strptr);
		qtd 	= atoi(Argv[1].strptr);
		break;

	case 3:	// Get start position from row/col
		screen_size(&rows,&cols);

		row = atoi(Argv[0].strptr)-1;
		col = atoi(Argv[1].strptr)-1;

		if(row < 0 || row > rows || col < 0 || col > cols)
		{
			ReturnString("");
		}

		start 	= (row * cols) + col;
		qtd 	= atoi(Argv[2].strptr);

		break;

	default:
		return RXFUNC_BADCALL;
	}

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

 ULONG APIENTRY rx3270WaitForChanges(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int last;

	if(Argc)
		return RXFUNC_BADCALL;

	last = query_screen_change_counter();

	while(last == query_screen_change_counter())
	{
		if(!CONNECTED)
		{
            ReturnValue( ENOTCONN );
		}
		gtk_main_iteration();
	}

	ReturnValue(0);
 }
