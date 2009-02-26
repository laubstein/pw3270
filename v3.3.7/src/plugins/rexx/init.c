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
 * Este programa está nomeado como init.c e possui n linhas de código.
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
/* Rexx External Function: rx3270Init		                                  */
/*                                                                            */
/* Description: Initialize lib3270 - Load default settings                    */
/*                                                                            */
/* Rexx Args:   Program path.                                                 */
/*                                                                            */
/* Returns:	    0 if ok, error code if not ok                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/
 ULONG APIENTRY rx3270Init(PSZ Name, LONG Argc, RXSTRING Argv[],PSZ Queuename, PRXSTRING Retstr)
 {
	char	*rx3270_argv[] = {"", ""};
	char 	**argv = (char **)&rx3270_argv;

	int		rx3270_argc = 1;
 	int 	rc = 0;

	if(Argc < 1)
		return RXFUNC_BADCALL;

	initialize_toggles();

	rx3270_argv[0] = Argv[0].strptr;

	rc = lib3270_init(Argv[0].strptr);

	g_thread_init(NULL);
	gtk_init(&rx3270_argc, &argv);

	return RetValue(Retstr,rc);
 }



