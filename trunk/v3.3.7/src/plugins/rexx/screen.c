/*
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe.
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
 * Este programa está nomeado como screen.c e possui 337 linhas de código.
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

 #include "rx3270.h"

 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>
 #include <lib3270/hostc.h>

/*---[ Statics ]----------------------------------------------------------------------------------*/


/*---[ Implement ]--------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270InputString                                  */
/*                                                                            */
/* Description: "type" informed string.                                       */
/*                                                                            */
/* Rexx Args:	NONE to send an Enter                                         */
/*                                                                            */
/* Rexx Args:	String to input                                               */
/*                                                                            */
/* Rexx Args:	Buffer position                                               */
/*				String to input                                               */
/*                                                                            */
/* Rexx Args:	New Cursor row                                                */
/*				New Cursor col                                                */
/*				String to input                                               */
/*                                                                            */
/* Rexx Args:   String to input                                               */
/*                                                                            */
/* Returns:	    None                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270InputString(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	if(!PCONNECTED)
 	{
		return RetValue(Retstr,ENOTCONN);
 	}

	switch(Argc)
	{
	case 0:
		action_internal(Enter_action, IA_DEFAULT, CN, CN);
		break;

    case 1:
		Input_String((const unsigned char *) Argv[0].strptr);
		break;

	case 2:
        cursor_set_addr(atoi(Argv[0].strptr));
		Input_String((const unsigned char *) Argv[1].strptr);
        break;

    case 3:
        cursor_set_addr((atoi(Argv[0].strptr)-1) * ctlr_get_cols() + (atoi(Argv[1].strptr)-1));
		Input_String((const unsigned char *) Argv[2].strptr);
        break;

	default:
		return RXFUNC_BADCALL;
	}

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
/*              New cursor col                                             	  */
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
        rc = cursor_set_addr((atoi(Argv[0].strptr)-1) * ctlr_get_cols() + (atoi(Argv[1].strptr)-1));
        break;

    default:
		return RXFUNC_BADCALL;
    }

	ReturnValue(rc);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270QueryScreenAttribute                         */
/*                                                                            */
/* Description: Get Screen Atribute                                           */
/*                                                                            */
/* Rexx Args:   Screen attribute to get                                       */
/*              SCREEN_COLS to get the number of the colums in the screen     */
/*              SCREEN_ROWS to get the number of the rows in the screen       */
/*              CURSOR_ADDR to get the current cursor position                */
/*                                                                            */
/* Returns:	    Value of the selected attribute                               */
/*                                                                            */
/*----------------------------------------------------------------------------*/
ULONG APIENTRY rx3270QueryScreenAttribute(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
{
    static const struct _attr
    {
        const char *name;
        int (*call)(void);
    } attr[] =
    {
        {   "SCREEN_COLS",  ctlr_get_cols   },
        {   "SCREEN_ROWS",  ctlr_get_rows   },
        {   "CURSOR_ADDR",  cursor_get_addr }
    };

    int f;
    int sz;

    if(Argc != 1)
        return RXFUNC_BADCALL;

    sz = strlen(Argv[0].strptr);

    for(f=0;f < G_N_ELEMENTS(attr); f++)
    {
        if(!g_ascii_strncasecmp(Argv[0].strptr,attr[f].name,sz))
        {
            f = attr[f].call();
            ReturnValue(f);
        }
    }


    ReturnValue(-1);
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


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270SendENTERKey                                 */
/*                                                                            */
/* Description: Send and "ENTER"        .                                     */
/*                                                                            */
/* Rexx Args:	None                                                          */
/*                                                                            */
/* Returns:	    None                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270SendENTERKey(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int rc = 0;

	if(Argc != 0)
		return RXFUNC_BADCALL;

	if(PCONNECTED)
		action_internal(Enter_action, IA_DEFAULT, CN, CN);
	else
		rc = ENOTCONN;

	ReturnValue(rc);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270SendPFKey                                    */
/*                                                                            */
/* Description: Activate a PF-Key action.                                     */
/*                                                                            */
/* Rexx Args:	Number of the key to activate (1 for PF1, 2, PF2, etc).       */
/*                                                                            */
/* Returns:	    None                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270SendPFKey(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	char buffer[10];

	if(Argc != 1)
		return RXFUNC_BADCALL;

	g_snprintf(buffer,9,"%d",atoi(Argv[0].strptr));
	action_internal(PF_action, IA_DEFAULT, buffer, 0);

	ReturnValue(0);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270WaitForChanges                               */
/*                                                                            */
/* Description: Wait until the screen changes.                                */
/*                                                                            */
/* Rexx Args:	None                                                          */
/*                                                                            */
/* Returns:	    0 if ok or error code                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/
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
		RunPendingEvents(TRUE);
	}

	ReturnValue(0);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270SetVisible		                              */
/*                                                                            */
/* Description: Set visibility state of g3270 main window                     */
/*                                                                            */
/* Rexx Args:	0 to hide window, now zero to show		                      */
/*                                                                            */
/* Returns:	    0 if ok or error code                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270SetVisible(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int rc = ENOENT;

	if(Argc != 1)
		return RXFUNC_BADCALL;

	if(g3270_topwindow)
	{
		rc = 0;
		if(atoi(Argv[0].strptr))
			gtk_widget_show(g3270_topwindow);
		else
			gtk_widget_hide(g3270_topwindow);
	}

	ReturnValue(rc);
 }
