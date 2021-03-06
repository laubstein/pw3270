/*
 * "Software PW3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
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
 * Este programa está nomeado como macros.h e possui - linhas de código.
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

 #define DECLARE_LIB3270_MACRO( name )  				LIB3270_EXPORT char * lib3270_macro_ ## name (H3270 *hSession, int argc, const char **argv);
 #define LIB3270_MACRO( name )  						LIB3270_EXPORT char * lib3270_macro_ ## name (H3270 *hSession, int argc, const char **argv)

 typedef struct _lib3270_macro_list
 {
 	const char *name;
 	char *(*exec)(H3270 *session, int argc, const char **argv);
 } LIB3270_MACRO_LIST;


/*---[ Macro entries ]---------------------------------------------------------------------------------*/

 LIB3270_EXPORT const LIB3270_MACRO_LIST * get_3270_calls(void);

 DECLARE_LIB3270_MACRO( connect )
 DECLARE_LIB3270_MACRO( cstate )
 DECLARE_LIB3270_MACRO( disconnect )
 DECLARE_LIB3270_MACRO( encoding )
 DECLARE_LIB3270_MACRO( get )
 DECLARE_LIB3270_MACRO( luname )
 DECLARE_LIB3270_MACRO( set )
 DECLARE_LIB3270_MACRO( status )
 DECLARE_LIB3270_MACRO( pf )
 DECLARE_LIB3270_MACRO( pa )
 DECLARE_LIB3270_MACRO( enter )
 DECLARE_LIB3270_MACRO( connect )

