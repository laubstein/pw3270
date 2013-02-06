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
 * Este programa está nomeado como commands.c e possui - linhas de código.
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

 #include "pipectl.h"

/*---[ Statics ]----------------------------------------------------------------------------------*/

#define DECLARE_XLAT_STATE( x ) { x, #x }

/*---[ Implement ]--------------------------------------------------------------------------------*/

/*

 static const gchar * get_state(void)
 {
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

	int f;

 	enum cstate state = QueryCstate();

	for(f=0;f < G_N_ELEMENTS(xlat_state); f++)
	{
		if(state == xlat_state[f].state)
			return xlat_state[f].ret;
	}

	return "Unexpected";
 }

 static void status(GString *rsp, gint argc, gchar **argv)
 {
 	const gchar	* luname	= get_connected_lu(0);
 	const gchar	* cstate	= get_state();
 	const gchar	* host		= get_current_host(0);

	g_string_append_printf(rsp,"%s %s %s",cstate,luname ? luname : "None",host ? host : "-");

 }

 static void cstate(GString *rsp, gint argc, gchar **argv)
 {
	g_string_append_printf(rsp,"%s",get_state());
 }

 static void luname(GString *rsp, gint argc, gchar **argv)
 {
 	const char *luname = get_connected_lu(0);
	g_string_append_printf(rsp,"%s",luname ? luname : "None");
 }
*/

/*---[ Command Interpreter ]----------------------------------------------------------------------*/

/*
 #define COMMAND_ENTRY(x) { #x, x }

 static const struct _entry
 {
 	const gchar *cmd;
 	void (*call)(GString *rsp, gint argc, gchar **argv);
 } entry[] =
 {
	COMMAND_ENTRY( luname ),
	COMMAND_ENTRY( cstate ),
	COMMAND_ENTRY( status ),
 };

 static int call(GString *rsp, gint argc, gchar **argv)
 {
	int f;

	for(f=0;f<G_N_ELEMENTS(entry);f++)
	{
		if(!g_ascii_strcasecmp(entry[f].cmd,argv[0]))
		{
			entry[f].call(rsp,argc,argv);
			return 0;
		}
	}
	return ENOENT;
 }

 gchar *run_commands(const gchar *str)
 {
 	GString *rsp	= g_string_new("");
	gchar	**line	= g_strsplit(str,"\n",-1);
	int		ln;

	for(ln = 0; line[ln]; ln++)
	{
		gint 	argc	= 0;
		gchar	**argv	= NULL;
		GError	*error	= NULL;

		if(ln > 0)
			g_string_append_printf(rsp,"\n");

		if(!g_shell_parse_argv(line[ln],&argc,&argv,&error))
		{
			g_string_append_printf(rsp,"Error in \"%s\": %s",line[ln],error->message);
			g_error_free(error);
		}
		else
		{
			if(call(rsp,argc,argv))
				g_string_append_printf(rsp,"Error in \"%s\": Unexpected command",line[ln]);
			g_strfreev(argv);
		}
	}
	g_strfreev(line);

	return g_string_free(rsp,FALSE);

 }

*/
