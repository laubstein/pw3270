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
 * Este programa está nomeado como tools.c e possui 180 linhas de código.
 * 
 * Contatos: 
 * 
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
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
/* Description: Send string to g3270's log file                               */
/*                                                                            */
/* Rexx Args:   String to log                                                 */
/*                                                                            */
/* Returns:	    None                                                          */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270Log(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc != 1)
		return RXFUNC_BADCALL;

	Log("%s",Argv[0].strptr);

	return RetValue(Retstr,0);
 }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270QueryCState                                  */
/*                                                                            */
/* Description: Query 3270 connection state                                   */
/*                                                                            */
/* Rexx Args:   None                                                          */
/*                                                                            */
/* Returns:	    Connection state                                              */
/*              NOT_CONNECTED                                                 */
/*              RESOLVING                                                     */
/*              PENDING                                                       */
/*              CONNECTED_INITIAL                                             */
/*              CONNECTED_ANSI                                                */
/*              CONNECTED_3270                                                */
/*              CONNECTED_INITIAL_E                                           */
/*              CONNECTED_NVT                                                 */
/*              CONNECTED_SSCP                                                */
/*              CONNECTED_TN3270E                                             */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270QueryCState(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	#define DECLARE_XLAT_STATE( x ) { x, #x }

 	static const struct _xlat_state
 	{
 		enum cstate	state;
 		const gchar	*ret;
 	} xlat_state[] =
 	{
			DECLARE_XLAT_STATE( NOT_CONNECTED 		),
			DECLARE_XLAT_STATE( RESOLVING			),
			DECLARE_XLAT_STATE( PENDING				),
			DECLARE_XLAT_STATE( CONNECTED_INITIAL	),
			DECLARE_XLAT_STATE( CONNECTED_ANSI		),
			DECLARE_XLAT_STATE( CONNECTED_3270		),
			DECLARE_XLAT_STATE( CONNECTED_INITIAL_E	),
			DECLARE_XLAT_STATE( CONNECTED_NVT		),
			DECLARE_XLAT_STATE( CONNECTED_SSCP		),
			DECLARE_XLAT_STATE( CONNECTED_TN3270E	)
 	};

 	int				f;
 	enum cstate 	state = QueryCstate();

	if(Argc != 0)
		return RXFUNC_BADCALL;

	for(f=0;f < G_N_ELEMENTS(xlat_state); f++)
	{
		if(state == xlat_state[f].state)
		{
			return RetString(Retstr,xlat_state[f].ret);
		}
	}

	return RetString(Retstr,"UNEXPECTED");
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
ULONG APIENTRY rx3270Version(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	strncpy(Retstr->strptr,PACKAGE_VERSION,RXAUTOBUFLEN-1);
    Retstr->strlength = strlen(Retstr->strptr);
    return RXFUNC_OK;
 }


 ULONG APIENTRY rx3270UpdateScreen(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(Argc)
		return RXFUNC_BADCALL;

	while(gtk_events_pending())
		gtk_main_iteration();

	return RetValue(Retstr,0);
 }

 ULONG APIENTRY rx3270Sleep(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
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
		gtk_main_iteration();

	Trace("Sleep ended (clock: %ld)",(long) time(0));

	return RetValue(Retstr,0);
 }


