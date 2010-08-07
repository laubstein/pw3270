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

 static int save_in_tempfile(gchar **filename, const gchar *str)
 {
#ifndef WIN32
 	GError	*error = NULL;
#endif

	gchar   tmpname[20];

	do
	{
			g_free(*filename);
			g_snprintf(tmpname,19,"%08lx.tmp",rand() ^ ((unsigned long) time(0)));
			*filename = g_build_filename(g_get_tmp_dir(),tmpname,NULL);
	} while(g_file_test(*filename,G_FILE_TEST_EXISTS));

	Trace("Temporary file: %s",*filename);

#ifdef WIN32

	{
		int fd = open(*filename,O_CREAT|O_TRUNC|O_TEXT|O_WRONLY,S_IRWXU);

		if(fd < 0)
		{
			Warning( N_( "Can't create temporary file %s\n" ), *filename);
			remove(*filename);
			g_free(*filename);
			return -1;
		}

		write(fd,str,strlen(str));

		close(fd);
	}

#else

	if(!g_file_set_contents(*filename,str,-1,&error))
	{
			if(error)
			{
				Warning( N_( "Can't create temporary file:\n%s" ), error->message ? error->message : N_( "Unexpected error" ));
				g_error_free(error);
			}
			remove(*filename);
			g_free(*filename);
			return -1;
	}

#endif

	return 0;
 }

 static gboolean parse_arguments(const gchar *command_line, gchar **tempfile, gint *argcp, gchar ***argvp, GError **error)
 {
	int		f;
	gchar	**argv;

	// Parse command-line
	if(!g_shell_parse_argv(command_line,argcp,argvp,error))
		return FALSE;

	argv = *argvp;

	// Convert arguments
	for(f=0;argv[f] && !*tempfile;f++)
	{
		gchar *ptr = NULL;

		if(!g_strcasecmp(argv[f],"%{ScreenContents}"))
		{
			ptr =  GetScreenContents(TRUE);

			save_in_tempfile(tempfile,ptr);
			g_free(argv[f]);
			argv[f] = g_strdup(*tempfile);

			g_free(ptr);

		}
		else if(!g_strcasecmp(argv[f],"%{SelectedArea}"))
		{
			ptr =  GetScreenContents(FALSE);

			save_in_tempfile(tempfile,ptr);
			g_free(argv[f]);
			argv[f] = g_strdup(*tempfile);

			g_free(ptr);
		}
		else if(!g_strcasecmp(argv[f],"%{Clipboard}"))
		{
			ptr =  GetClipboard();

			save_in_tempfile(tempfile,ptr);
			g_free(argv[f]);
			argv[f] = g_strdup(*tempfile);

			g_free(ptr);
		}
		else if(!g_strcasecmp(argv[f],"%{luname}"))
		{
			const gchar *ptr = get_connected_lu(hSession);
			if(ptr)
			{
				g_free(argv[f]);
				argv[f] = g_strdup(ptr);
			}
		}
		else if(!g_strcasecmp(argv[f],"%{host}"))
		{
			const gchar *ptr = get_current_host(hSession);
			if(ptr)
			{
				g_free(argv[f]);
				argv[f] = g_strdup(ptr);
			}
		}

		Trace("Argv[%d]=\"%s\"",f,argv[f]);
	}

	return TRUE;
 }

 static void process_ended(GPid pid,gint status,gchar *tempfile)
 {
 	Trace("Process %d ended with status %d",(int) pid, status);
 	if(tempfile)
 	{
		remove(tempfile);
		g_free(tempfile);
 	}
 	g_spawn_close_pid(pid);
 }

 int spawn_async_process(const gchar *line, GPid *pid, gchar **tempfile, GError **error)
 {
	gint 	argc	= 0;
	gchar	**argv	= NULL;

	if(!parse_arguments(line,tempfile,&argc,&argv,error))
	{
		Trace("%s: Cant parse arguments",__FUNCTION__);
		return -1;
	}
	else
	{
		// Spawn command
		if(!g_spawn_async(NULL,argv,NULL,G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD,NULL,NULL,pid,error))
		{
			Trace("%s: Cant spawn process \"%s\" with %d arguments",__FUNCTION__,argv[0],argc);
			g_strfreev(argv);
			if(*tempfile)
			{
				remove(*tempfile);
				g_free(*tempfile);
				*tempfile = NULL;
			}
			return -1;
		}
	}

	g_strfreev(argv);

	return 0;
 }

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

 int run_command(const gchar *line, GError **error)
 {
	PW3270_COMMAND_POINTER cmd;

	gchar	*tempfile	= NULL;
	GPid	pid;
	gint 	argc		= 0;
	gchar	**argv		= NULL;

	if(!parse_arguments(line,&tempfile,&argc,&argv,error))
		return -1;

	Trace("Command \"%s\" with %d arguments",argv[0],argc);

	// Check for internal commands
	cmd = get_command_pointer(argv[0]);
	if(cmd)
	{
		int rc = -1;
		gchar *rsp = cmd(argc-1,(const gchar **) argv+1);
		if(tempfile)
			remove(tempfile);
		if(rsp)
		{
			rc = atoi(rsp);
			g_free(rsp);
		}
		return rc;
	}

	// Check for action commands
	if(argc == 1)
	{
		 GtkAction *action =  get_action_by_name(argv[0]);
		 if(action)
		 {
		 	gtk_action_activate(action);
		 	return 0;
		 }
	}

	// Spawn external process
	if(!g_spawn_async(NULL,argv,NULL,G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD,NULL,NULL,&pid,error))
	{
		g_strfreev(argv);
		if(tempfile)
		{
			g_free(tempfile);
			remove(tempfile);
		}
		return -1;
	}

	g_child_watch_add(pid,(GChildWatchFunc) process_ended,tempfile);

	g_strfreev(argv);

	return 0;
 }


