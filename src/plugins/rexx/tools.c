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
 * Este programa está nomeado como tools.c e possui 172 linhas de código.
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

/*---[ Statics ]----------------------------------------------------------------------------------*/


/*---[ Implement ]--------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270Log                                          */
/*                                                                            */
/* Description: Send string to program's log file                             */
/*                                                                            */
/* Rexx Args:   String to log                                                 */
/*                                                                            */
/* Returns:	    None                                                      */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 RexxReturnCode REXXENTRY rx3270Log(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	Log("%s",Argv[0].strptr);

	return RetValue(Retstr,0);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270Version                                      */
/*                                                                            */
/* Description: Query rx3270 current version                                  */
/*                                                                            */
/* Rexx Args:   None                                                          */
/*                                                                            */
/* Returns:	    String with the version information                           */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 RexxReturnCode REXXENTRY rx3270Version(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	strncpy(Retstr->strptr,PACKAGE_VERSION,RXAUTOBUFLEN-1);
    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }


/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270WaitForEvents                                */
/*                                                                            */
/* Description: Wait for network/screen events                                */
/*                                                                            */
/* Rexx Args:   non zero to wait.                                             */
/*                                                                            */
/* Returns:	    None                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 RexxReturnCode REXXENTRY rx3270WaitForEvents(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	switch(Argc)
 	{
	case 0:
		RunPendingEvents(0);
		break;

	case 1:
		RunPendingEvents(atoi(Argv[0].strptr));
		break;

	default:
		return RXFUNC_BADCALL;
 	}

	return RetValue(Retstr,0);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270Sleep		                                  */
/*                                                                            */
/* Description: Wait for "N" seconds                                          */
/*                                                                            */
/* Rexx Args:   Number of seconds to wait                                     */
/*                                                                            */
/* Returns:	    0 if ok, error code if not ok                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 RexxReturnCode REXXENTRY rx3270Sleep(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	time_t end = time(0);

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

	Trace("Wait from %ld to %ld (CPS %ld)",(long) clock(),(long) end, (long) CLOCKS_PER_SEC);

	while(time(0) < end)
	{
		RunPendingEvents(1);
		if(IsHalted())
			return RetValue(Retstr,ECANCELED);
	}

	Trace("Sleep ended (clock: %ld)",(long) time(0));

	return RetValue(Retstr,0);
 }

 RexxReturnCode REXXENTRY rx3270Dunno(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	return RetValue(Retstr,0);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270Quit		                                  */
/*                                                                            */
/* Description: Quit main application (gtk_main_quit)                         */
/*                                                                            */
/* Rexx Args:   None                                                          */
/*                                                                            */
/* Returns:	    None                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 RexxReturnCode REXXENTRY rx3270Quit(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	gtk_main_quit();
	return RetValue(Retstr,0);
 }
