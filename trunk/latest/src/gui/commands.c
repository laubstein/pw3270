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
 * Este programa está nomeado como commands.c e possui 1088 linhas de código.
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

 #include <lib3270/config.h>
 #include <lib3270/plugins.h>
 #include <stdlib.h>

#ifdef WIN32
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
#else
	#include <glib.h>
	#include <glib/gstdio.h>
#endif

 #include "gui.h"

// TODO (perry#1#): DEPRECATED - Replace with the new command/macro engine on script.c

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 // Internal commands
 PW3270_COMMAND_ENTRY(connect)
 {
	const gchar *host;

	switch(argc)
	{
	case 1:
		host = GetString("Network","Hostname",NULL);
		if(!host)
		{
			action_SetHostname();
			return 0;
		}
		break;

	case 2:
		host = argv[1];
		break;

	default:
		return NULL;
	}

 	Trace("%s Connected:%d",__FUNCTION__,PCONNECTED);

 	if(PCONNECTED)
 		return g_strdup("BUSY");

 	action_ClearSelection();

	DisableNetworkActions();

	gtk_widget_set_sensitive(topwindow,FALSE);

	RunPendingEvents(0);

	Trace("Connect to \"%s\"",host);

	if(host_connect(host,1) == ENOTCONN)
	{
		Warning( N_( "Negotiation with %s failed!" ),host);
	}

	Trace("Topwindow: %p Terminal: %p",topwindow,terminal);

	if(topwindow)
		gtk_widget_set_sensitive(topwindow,TRUE);

	if(terminal)
		gtk_widget_grab_focus(terminal);

	return 0;

 }

 // Command interpreter
 #define INTERNAL_COMMAND_ENTRY(x) { #x, pw3270_command_entry_ ## x }
 static const struct _internal_command
 {
 	const gchar	*cmd;
 	PW3270_COMMAND_POINTER call;
 } internal_command[] =
 {
	INTERNAL_COMMAND_ENTRY(connect),
 };

 PW3270_COMMAND_POINTER get_command_pointer(const gchar *cmd)
 {
 	int f;

	for(f=0;f<G_N_ELEMENTS(internal_command);f++)
	{
		if(!g_ascii_strcasecmp(cmd,internal_command[f].cmd))
			return internal_command[f].call;
	}

	return NULL;
 }

