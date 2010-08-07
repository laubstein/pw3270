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
 * Este programa está nomeado como script.c e possui - linhas de código.
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


 #include <errno.h>
 #include <glib.h>
 #include <lib3270/config.h>

#ifndef WIN32
 #include <unistd.h>
#endif

 #include "gui.h"

/*---[ Structs ]------------------------------------------------------------------------------------------------*/

 struct cnvt
 {
 	const gchar *arg;
 	gchar		*(*begin)(void);
 	void		 (*end)(gpointer arg);
 };

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

 static gchar * set_tempfile(const gchar *str);


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static gchar *screencontents(void)
 {
 	return set_tempfile(GetScreenContents(TRUE));
 }

 static gchar *selectedarea(void)
 {
	return set_tempfile(GetScreenContents(FALSE));
 }

 static gchar *clipboard(void)
 {
	return set_tempfile(GetClipboard());
 }

 static gchar *luname(void)
 {
 	gchar *lu = (gchar *) get_connected_lu(hSession);
	return lu ? lu : "";
 }

 static gchar *host(void)
 {
 	gchar *host = (gchar *) get_current_host(hSession);
	return host ? host : "";
 }

 static void dunno(gpointer data)
 {

 }

 static void remove_tempfile(gpointer filename)
 {
	remove(filename);
	g_free(filename);
 }

 static const struct cnvt *get_conversion_methods(const gchar *arg)
 {
	static const struct cnvt mtd[] =
	{
		{ "ScreenContents",	screencontents, remove_tempfile	},
		{ "SelectedArea",	selectedarea, 	remove_tempfile	},
		{ "Clipboard", 		clipboard, 		remove_tempfile	},
		{ "luname", 		luname, 		dunno			},
		{ "host",			host, 			dunno			},
	};

	int f;

	if(*arg == '{')
		arg++;

	for(f=0;f<G_N_ELEMENTS(mtd);f++)
	{
		if(!g_strncasecmp(arg,mtd[f].arg,strlen(mtd[f].arg)))
			return mtd+f;
	}

	return NULL;
 }

 struct child_cleanup
 {
 	void (*end)(gpointer arg);
 	gchar *arg;
 };

 static void child_ended(GPid pid,gint status,struct child_cleanup *cleanup)
 {
 	int f;

 	Trace("Process %d ended with status %d",(int) pid, status);

	for(f=0;cleanup[f].end;f++)
	{
		Trace("Cleaning %d: %p(%s)",f,cleanup[f].end,cleanup[f].arg);
		cleanup[f].end(cleanup[f].arg);
	}

	g_free(cleanup);
 	g_spawn_close_pid(pid);
 	exit(-1);
 }

 static int spawn_child(const gchar *script_name, int argc, const struct cnvt **convert, gchar **argv)
 {
	gchar *child_argv[argc+2];
	struct child_cleanup *cleanup;
	int 	f;
	GPid 	pid;
	GError	*err = NULL;
	int		qtd = 0;
	int		pos = 0;

	Trace("Running %s with %d arguments",script_name,argc);

	// Adjust arguments for spawn call
	child_argv[0] = (gchar *) script_name;
	for(f=0;f<argc;f++)
	{
		child_argv[f+1] = argv[f];
		if(convert[f] && g_file_test(argv[f],G_FILE_TEST_EXISTS))
			qtd++;
	}
	child_argv[f+1] = NULL;

#ifdef DEBUG
	for(f=0;child_argv[f];f++)
	{
		Trace("argv(%d)=%s",f,child_argv[f]);
	}
#endif

	if(!g_spawn_async(NULL,child_argv,NULL,G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD,NULL,NULL,&pid,&err))
	{
		GtkWidget *dialog;

		dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
											GTK_DIALOG_DESTROY_WITH_PARENT,
											GTK_MESSAGE_WARNING,
											GTK_BUTTONS_OK_CANCEL,
											_(  "Can't start external program \"%s\"" ), script_name);

		gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't start" ) );

		if(err)
		{
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", err->message);
			g_error_free(err);
		}

		if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_CANCEL)
			gtk_main_quit();
		gtk_widget_destroy(dialog);
		return -1;
	}

	// Keep tempfiles until child ends
	cleanup	= g_malloc0((sizeof(struct child_cleanup)*qtd));
	for(f=0;child_argv[f] && pos < qtd;f++)
	{
		if(convert[f] && g_file_test(argv[f],G_FILE_TEST_EXISTS))
		{
			cleanup[pos].end = convert[f]->end;
			cleanup[pos].arg = argv[f];
			convert[f] = NULL;
			pos++;
		}
	}

	g_child_watch_add(pid,(GChildWatchFunc) child_ended, cleanup);

	return 0;
 }

 int script_interpreter( const gchar *script_type, const gchar *script_name, const gchar *script_text, int argc, gchar **argv )
 {
 	int f;
 	int rc = 0;
 	const struct cnvt *convert[argc+1];
 	gchar *converted_argv[argc+1];

	// Convert arguments
	for(f=0;f<argc;f++)
	{
		convert[f] = 0;

		if(!argv[f])
		{
			converted_argv[f] = "";
		}
		else if(*argv[f] == '%')
		{
			convert[f] = get_conversion_methods(argv[f]+1);
			if(convert[f])
				converted_argv[f] = convert[f]->begin();
			else
				converted_argv[f] = argv[f];
		}
		else
		{
			converted_argv[f] = argv[f];
		}
	}

	// Run script
	if(script_type)
	{
		int (*interpret)( const gchar *script_name, const gchar *script_text, int argc, gchar **argv );

		if(get_symbol_by_name(NULL,(gpointer) &interpret,"pw3270_script_interpreter_%s",script_type))
		{
			rc = interpret(script_name,script_text,argc,converted_argv);
		}
		else if(script_name && g_file_test(script_name,G_FILE_TEST_IS_EXECUTABLE))
		{
			spawn_child(script_name,argc,convert,converted_argv);
		}
		else
		{
			GtkWidget *dialog;

			dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_WARNING,
												GTK_BUTTONS_OK_CANCEL,
												_(  "Can't find a valid interpreter for \"%s\"" ), script_type);

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't start script" ) );

			if(script_name)
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), _( "The script \"%s\" can't be started.\nDo you have the required plugin installed?" ),script_name);

			if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_CANCEL)
				gtk_main_quit();
			gtk_widget_destroy(dialog);
			rc = EINVAL;
		}
	}
	else if(script_name)
	{
		// No type
		PW3270_COMMAND_POINTER cmd = get_command_pointer(script_name);
		if(cmd)
		{
			gchar *rsp = cmd(argc,(const gchar **) argv);
			if(rsp)
				g_free(rsp);
			else
				rc = -1;
		}
		else if(g_file_test(script_name,G_FILE_TEST_IS_EXECUTABLE))
		{
			spawn_child(script_name,argc,convert,converted_argv);
		}
		else
		{
			GtkWidget *dialog;

			dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_WARNING,
												GTK_BUTTONS_OK_CANCEL,
												_(  "Can't start script \"%s\"" ), script_name);

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't start script" ) );

			if(script_name)
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s",  _( "The script file is invalid or nonexistent." ));

			if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_CANCEL)
				gtk_main_quit();
			gtk_widget_destroy(dialog);
			rc = ENOENT;
		}
	}

	// Release converted arguments
	for(f=0;f<argc;f++)
	{
		if(convert[f])
			convert[f]->end(converted_argv[f]);
	}

	return rc;
 }

/**
 * Run a single script.
 *
 * @param script Script line in the format script(arg1,arg2,...,argn)
 *
 * @return 0 if ok, error code if non ok
 */
 int run_script_command_line(const gchar *script)
 {
	gchar *begin_arg = g_strstr_len(script,-1,"(");
	gchar *type;
	int	   rc = 0;

	if(begin_arg)
	{
		// Arguments delimited by ()
		gchar	** argv  = 0;
		gchar	 * end_arg = g_strstr_len(begin_arg,-1,")");

		*(begin_arg++) = 0;

		type = g_strrstr(script,".");
		if(type)
			type++;

		if(end_arg)
			*(end_arg) = 0;

		Trace("Calling script %s(%s)",script,begin_arg);

		argv = g_strsplit(begin_arg,",",-1);
		script_interpreter(type,script,NULL,g_strv_length(argv),argv);

		g_strfreev(argv);
	}

	else
	{
		int 	   argc;
		gchar 	** argv;
		GError	*  error = NULL;

		if(!g_shell_parse_argv(script,&argc,&argv,&error))
		{
			GtkWidget *dialog;

			dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_WARNING,
												GTK_BUTTONS_OK_CANCEL,
												_(  "Can't start script \"%s\"" ), script);

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't start script" ) );

			rc = -1;
			if(error)
			{
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s",  error->message );
				rc = error->code;
				g_error_free(error);
			}

			if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_CANCEL)
				gtk_main_quit();

			gtk_widget_destroy(dialog);

			return rc;
		}


		type = g_strrstr(argv[0],".");
		if(type)
			type++;

		rc = script_interpreter(type,argv[0],NULL,argc-1,argv+1);

		g_strfreev(argv);
	}

	return rc;
 }

/**
 * Call a list of scripts.
 *
 * @param scripts List of scripts to run in the format script1(arg1,arg2);script2(arg);...
 *
 */
 void run_script_list( const gchar *scripts )
 {
	// Call scripts
	int str;
	gchar **script = g_strsplit(scripts,";",-1);

	for(str=0;script[str];str++)
		run_script_command_line(script[str]);

	g_strfreev(script);

 }

 static gchar * set_tempfile(const gchar *str)
 {
 	GError	* error = NULL;
 	gchar	* filename = NULL;
 	gint	  fd = g_file_open_tmp(NULL,&filename,&error);

 	if(fd < 0)
 	{
		if(error)
		{
			Warning( N_( "Can't create temporary file:\n%s" ), error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
 		return "";
 	}
	close(fd);

	if(!g_file_set_contents(filename,str,-1,&error))
	{
			if(error)
			{
				Warning( N_( "Can't create temporary file:\n%s" ), error->message ? error->message : N_( "Unexpected error" ));
				g_error_free(error);
			}
			remove(filename);
			g_free(filename);
			return "";
	}

	return filename;
 }

