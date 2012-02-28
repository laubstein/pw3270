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
 * Este programa está nomeado como init.c e possui 151 linhas de código.
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

/*---[ Implement ]--------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/*                                                                            */
/* Rexx External Function: rx3270Connect                                      */
/*                                                                            */
/* Description: Connect with the supplied host                                */
/*                                                                            */
/* Rexx Args:   Host path                                                     */
/*				Non zero to wait for connection to be ok.					  */
/*                                                                            */
/* Returns:	    0 if ok, error code if not ok                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 RexxReturnCode REXXENTRY rx3270Connect(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	int rc = 0;
 	int wait = 1;

	if(Argc < 1)
		return RXFUNC_BADCALL;

	if(Argc > 1)
		wait = atoi(Argv[1].strptr);

	Trace("%s(%s,%d)",__FUNCTION__,Argv[0].strptr,wait);
	rc = host_connect(Argv[0].strptr,wait);
	Trace("%s exits with %d",__FUNCTION__,rc);

	return RetValue(Retstr,rc);
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
 RexxReturnCode REXXENTRY rx3270QueryCState(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
 	#define DECLARE_XLAT_STATE( x ) { x, #x }

 	static const struct _xlat_state
 	{
 		LIB3270_CSTATE	  state;
 		const gchar		* ret;
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
 	LIB3270_CSTATE	state;

	if(Argc != 0)
		return RXFUNC_BADCALL;

	RunPendingEvents(0);

	state = QueryCstate();

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
/* Rexx External Function: rx3270Disconnect                                   */
/*                                                                            */
/* Description: Disconnect from 3270 host                                     */
/*                                                                            */
/* Rexx Args:   None                                                          */
/*                                                                            */
/* Returns:	    0 if ok, error code if not ok                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 RexxReturnCode REXXENTRY rx3270Disconnect(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	if(!PCONNECTED)
	{
		return RetValue(Retstr,ENOTCONN);
	}

	lib3270_disconnect(hSession);
	return RetValue(Retstr,0);
 }

