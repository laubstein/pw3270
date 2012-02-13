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
 * Este programa está nomeado como screen.c e possui 710 linhas de código.
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

 #include <errno.h>
 /*
 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>
 */

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
 RexxReturnCode REXXENTRY rx3270InputString(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	char *str = 0;

	Trace("Status: %d",query_3270_terminal_status());

 	if(query_3270_terminal_status() != STATUS_CODE_BLANK)
		return RetValue(Retstr,EINVAL);

	switch(Argc)
	{
	case 0:
		action_Enter();
		break;

    case 1:
		str = GetStringArg(Argv);
		break;

	case 2:
        cursor_set_addr(atoi(Argv[0].strptr));
		str = GetStringArg(Argv+1);
        break;

    case 3:
        cursor_set_addr((atoi(Argv[0].strptr)-1) * ctlr_get_cols() + (atoi(Argv[1].strptr)-1));
		str = GetStringArg(Argv+2);
        break;

	default:
		return RXFUNC_BADCALL;
	}

	if(str)
	{
		Input_String((unsigned char *) str);
		ReleaseStringArg(str);
	}

	Trace("Status: %d",query_3270_terminal_status());

	return RetValue(Retstr,query_3270_terminal_status() == STATUS_CODE_BLANK ? 0 : EINVAL);
 }

 RexxReturnCode REXXENTRY rx3270FindFieldAttribute(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	return RetValue(Retstr,find_field_attribute(atoi(Argv[0].strptr)));
 }

 RexxReturnCode REXXENTRY rx3270FindFieldLength(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	return RetValue(Retstr,find_field_length(atoi(Argv[0].strptr)));
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270SetCursorPosition                            */
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
RexxReturnCode REXXENTRY rx3270SetCursorPosition(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
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
RexxReturnCode REXXENTRY rx3270QueryScreenAttribute(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
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
RexxReturnCode REXXENTRY rx3270GetCursorPosition(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
{
    if(Argc)
		return RXFUNC_BADCALL;

	ReturnValue(cursor_get_addr());
}

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270WaitForStringAt                              */
/*                                                                            */
/* Description: Wait for desired string at position                           */
/*                                                                            */
/* Rexx Args:	NONE to read the entire screen                                */
/*                                                                            */
/* Rexx Args:   Number of chars to read from cursor position                  */
/*                                                                            */
/* Rexx Args:   row                                                           */
/*				Col													  		  */
/*              string                                       				  */
/*				timeout (seconds)											  */
/*                                                                            */
/* Rexx Args:   row                                                           */
/*				Col													  		  */
/*              string                                       				  */
/*                                                                            */
/* Rexx Args:   string                                                        */
/*                                                                            */
/* Rexx Args:   string                                                        */
/*              timeout                                                       */
/*                                                                            */
/* Returns:	    0 if the string is in the screen or error code                */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 RexxReturnCode REXXENTRY rx3270WaitForStringAt(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int start, sz, rows, cols, row, col, rc;
 	time_t end = time(0);
 	const char *key;
 	char *buffer;

	screen_size(&rows,&cols);

	switch(Argc)
	{
	case 1:	// 1 parameter, search for string in the entire screen with 1 second timeout
		row = col = 0;
		key = Argv[0].strptr;
		end++;
		sz = (rows*cols)+1;
		break;

	case 2:	// 2 parameters, search for string in the entire screen with supplied timeout
		row = col = 0;
		key = Argv[0].strptr;
		end += atoi(Argv[1].strptr);
		sz = (rows*cols)+1;
		break;

	case 3:	// 3 parameters assume default timeout
		row = atoi(Argv[0].strptr)-1;
		col = atoi(Argv[1].strptr)-1;
		key = Argv[2].strptr;
		sz = strlen(key);
		end += RX3270_DEFAULT_TIMEOUT;
		break;

	case 4:	// All parameters
		row = atoi(Argv[0].strptr)-1;
		col = atoi(Argv[1].strptr)-1;
		key = Argv[2].strptr;
		sz = strlen(key);
		end += atoi(Argv[3].strptr);
		Trace("Timeout: %d",atoi(Argv[3].strptr));
		break;

	default:
		return RXFUNC_BADCALL;
	}

	if(row < 0 || row > rows || col < 0 || col > cols)
	{
		ReturnValue(EINVAL);
	}

	start = ((row) * cols) + col;
	rc = ETIMEDOUT;

	buffer = malloc(sz+2);

	Trace("Waiting for %ld seconds (Default: %d)", (end-time(0)),RX3270_DEFAULT_TIMEOUT);

	RunPendingEvents(0);

	while( (rc == ETIMEDOUT) && (time(0) <= end) )
	{
		if(!CONNECTED)
		{
			Trace("Disconnected when waiting for \"%s\"",key);
			rc = ENOTCONN;
		}
		else if(IsHalted())
		{
			rc = ECANCELED;
		}
		else if(query_3270_terminal_status() == STATUS_CODE_BLANK)
		{
			screen_read(buffer,start,sz);
			*(buffer+sz) = 0;

			if(strstr(buffer,key))
				rc = 0;
		}

		RunPendingEvents(1);

	}

	Trace("Waitforstring(%s) exits with %d (%s) (Status: %d)",key,rc,rc == ETIMEDOUT ? "TIMEOUT" : strerror(rc),query_3270_terminal_status());

	free(buffer);

	ReturnValue(rc);

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
 RexxReturnCode REXXENTRY rx3270ReadScreen(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int start, qtd, rows, cols, row, col;
 	char *buffer;

	switch(Argc)
	{
	case 0:	// Get entire screen
		screen_size(&rows,&cols);
		qtd = (rows*(cols+1)+1);
		buffer = malloc(qtd+2);

		Trace("Screen buffer size: %d (%dx%d)",qtd,rows,cols);

		memset(buffer,0,qtd+1);
		start = qtd = 0;
		for(row = 0; row < rows;row++)
		{
			screen_read(buffer+qtd,start,cols);
			qtd += cols;
			start += cols;
			Retstr->strptr[qtd++] = '\n';
		}
		buffer[qtd] = 0;

		Trace("Bytes read: %d",qtd);

		RetConvertedString(Retstr,buffer);

		free(buffer);
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

	buffer = malloc(qtd+1);
	screen_read(buffer, start, qtd);

	RetConvertedString(Retstr,buffer);

	free(buffer);

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
 RexxReturnCode REXXENTRY rx3270SendENTERKey(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int rc = 0;

	if(Argc != 0)
		return RXFUNC_BADCALL;

	if(!PCONNECTED)
		rc = ENOTCONN;
	else if(query_3270_terminal_status() != STATUS_CODE_BLANK)
		rc = EINVAL;
	else
		rc = action_Enter();

	if(!rc)
	{
		// Enter was sent, wait for screen changes
		int last = query_screen_change_counter();
		time_t tm = time(0)+1;

		while(last == query_screen_change_counter() && time(0) < tm)
			RunPendingEvents(1);
	}

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
 RexxReturnCode REXXENTRY rx3270SendPFKey(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int rc = 0;

	if(Argc != 1)
		return RXFUNC_BADCALL;

	Trace("%s: Status: %d Key: %s",__FUNCTION__,query_3270_terminal_status(),Argv[0].strptr);

	if(!PCONNECTED)
		rc = ENOTCONN;
	else if(query_3270_terminal_status() != STATUS_CODE_BLANK)
		rc = EINVAL;
	else
		rc = action_PFKey(atoi(Argv[0].strptr));

	if(!rc)
	{
		// Key was sent, wait for screen changes
		int last = query_screen_change_counter();
		time_t tm = time(0)+1;

		while(last == query_screen_change_counter() && time(0) < tm)
			RunPendingEvents(1);
	}

	ReturnValue(rc);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270WaitForChanges                               */
/*                                                                            */
/* Description: Wait until the screen changes.                                */
/*                                                                            */
/* Rexx Args:	Timeout in seconds  (None to default to 60 seconds)           */
/*                                                                            */
/* Returns:	    0 if ok or error code                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/

 RexxReturnCode REXXENTRY rx3270WaitForChanges(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int 			last;
 	int				rc  = 0;
 	int 			end = time(0);

	switch(Argc)
	{
	case 0:
		end += 60;
		break;

	case 1:
		end += atoi(Argv[0].strptr);
		break;

	default:
		return RXFUNC_BADCALL;
	}

	last = query_screen_change_counter();

	while(!rc && last == query_screen_change_counter())
	{
		if(!CONNECTED)
			rc = ENOTCONN;
		else if(IsHalted())
			rc = ECANCELED;
		else if(time(0) > end)
			rc = ETIMEDOUT;
		RunPendingEvents(1);
	}

	ReturnValue(rc);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270SetVisible		                              */
/*                                                                            */
/* Description: Set visibility state of pw3270 main window                     */
/*                                                                            */
/* Rexx Args:	0 to hide window, now zero to show		                      */
/*                                                                            */
/* Returns:	    0 if ok or error code                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 RexxReturnCode REXXENTRY rx3270SetVisible(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int rc = ENOENT;

	if(Argc != 1)
		return RXFUNC_BADCALL;

	if(program_window)
	{
		rc = 0;
		if(atoi(Argv[0].strptr))
			gtk_widget_show(program_window);
		else
			gtk_widget_hide(program_window);
	}

	ReturnValue(rc);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270WaitForString	                              */
/*                                                                            */
/* Description: Wait for desired string at position                           */
/*                                                                            */
/* Rexx Args:	String to search                                              */
/*                                                                            */
/* Rexx Args:	Buffer position                                               */
/*				String to search                                              */
/*                                                                            */
/* Rexx Args:	Start row                                                     */
/*				Start col                                                     */
/*				String to search                                              */
/*                                                                            */
/* Returns:	    0 if ok or error code                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 RexxReturnCode REXXENTRY rx3270WaitForString(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int			pos = -1;
 	char		*str;
 	int			sz;
 	int			rc = ETIMEDOUT;
 	int			rows,cols,row,col;
 	char		*buffer;
	time_t 	end = time(0)+RX3270_DEFAULT_TIMEOUT;

	screen_size(&rows,&cols);

 	switch(Argc)
 	{
	case 1:	// Only string in any position
		str = GetStringArg(Argv);
		sz = (rows*cols);
		break;

	case 2:	// Just buffer position
		pos = atoi(Argv[0].strptr);
		str = GetStringArg(Argv+1);
		sz  = strlen(str);
		break;

	case 3:	// String att row/col

		row = atoi(Argv[0].strptr)-1;
		col = atoi(Argv[1].strptr)-1;

		if(row < 0 || row > rows || col < 0 || col > cols)
		{
			ReturnValue(EINVAL);
		}

		pos = (row * cols) + col;
		str = GetStringArg(Argv+2);
		sz	= strlen(str);
		break;

	default:
		return RXFUNC_BADCALL;
 	}

	buffer = malloc(sz+2);
	if(!buffer)
	{
		ReturnValue(ENOMEM);
	}

	RunPendingEvents(0);

	Trace("%s waiting for \"%s\"",__FUNCTION__,str);

	while(rc == ETIMEDOUT  && (time(0) <= end))
	{
		if(!CONNECTED)
		{
            rc = ENOTCONN;
		}
		else if(IsHalted())
		{
			rc = ECANCELED;
		}
		else if(query_3270_terminal_status() == STATUS_CODE_BLANK)
		{
			screen_read(buffer,pos,sz);
			*(buffer+(sz+1)) = 0;
			if(strstr(buffer,str))
				rc = 0;
		}

		RunPendingEvents(1);

	}

	Trace("%s returns %d",__FUNCTION__,rc);

	ReleaseStringArg(str);
	free(buffer);

	ReturnValue(rc);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270IsTerminalReady                              */
/*                                                                            */
/* Description: Check if the terminal is ready                                */
/*                                                                            */
/* Rexx Args:   None                                                          */
/*                                                                            */
/* Returns:	    Non zero if terminal is ready                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
RexxReturnCode REXXENTRY rx3270IsTerminalReady(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
{
    if(Argc)
		return RXFUNC_BADCALL;

	if(!CONNECTED || query_3270_terminal_status() != STATUS_CODE_BLANK)
	{
		ReturnValue(0);
	}

	ReturnValue(1);
}

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270WaitForTerminalReady                         */
/*                                                                            */
/* Description: Wait for terminal status is ready                             */
/*                                                                            */
/* Rexx Args:	timeout                                                       */
/*                                                                            */
/* Rexx Args:	None for 1 second timeout.                                    */
/*                                                                            */
/* Returns:	    0 if ok or error code                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 RexxReturnCode REXXENTRY rx3270WaitForTerminalReady(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int end = time(0);
 	int rc = ETIMEDOUT;

 	switch(Argc)
 	{
	case 0:
		end++;
		break;

	case 1:
		end += atoi(Argv[0].strptr);
		break;

	default:
		return RXFUNC_BADCALL;
 	}

	Trace("%s waiting for %d seconds",__FUNCTION__,(int) (end - time(0)));

	while( (rc == ETIMEDOUT) && (time(0) <= end) )
	{
		RunPendingEvents(TRUE);

		if(!CONNECTED)
            rc = ENOTCONN;
		else if(IsHalted())
			rc = ECANCELED;
		else if(query_3270_terminal_status() == STATUS_CODE_BLANK)
			rc = 0;
	}

	Trace("%s exits with rc=%d",__FUNCTION__,rc);

	ReturnValue(rc);
 }

