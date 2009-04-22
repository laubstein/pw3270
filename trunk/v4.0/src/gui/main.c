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
 * Este programa está nomeado como main.c e possui 502 linhas de código.
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

#include "gui.h"

#include <stdio.h>
#include <gtk/gtk.h>

#ifdef HAVE_LIBGNOME
	#include <gnome.h>
#endif

#include <glib/gstdio.h>

#include "globals.h"

#if !defined(_WIN32) /*[*/
#include <sys/wait.h>
#include <signal.h>
#endif /*]*/

#include <errno.h>
#include <lib3270/3270ds.h>
#include <lib3270/toggle.h>
#include "ftc.h"
#include "macrosc.h"
// #include "printerc.h"
#include "togglesc.h"
#include "utilc.h"
#include "xioc.h"

#if defined(_WIN32) /*[*/
#include <windows.h>
#include "winversc.h"
#include "windirsc.h"
#endif /*]*/

/* Globals */
#ifdef HAVE_LIBGNOME
GnomeClient *client = 0;
#endif

static const char	*cl_hostname	= NULL;
static const char	*startup_script = NULL;

/* Callback for connection state changes. */
#ifdef 	X3270_FT
static void connect_3270(int status)
{
	gtk_action_group_set_sensitive(ft_actions,status);
}
#endif

static void connect_main(int status)
{
	gboolean online = (CONNECTED) ? TRUE : FALSE;

	Trace("%s: status: %d Connected: %d",__FUNCTION__,status,(int) online);

	if(status)
	{
		cMode |= CURSOR_MODE_ENABLED;
	}
	else
	{
		SetStatusCode(STATUS_CODE_DISCONNECTED);
		cMode &= ~CURSOR_MODE_ENABLED;
		ctlr_erase(True);
		online = FALSE;
#ifdef 	X3270_FT
		connect_3270(status);
#endif
	}

	if(terminal)
	{
		gtk_widget_set_sensitive(terminal,online);
		DrawOIA(terminal,color,pixmap);
		gtk_widget_queue_draw(terminal);
		gtk_widget_grab_focus(terminal);
	}

	gtk_action_group_set_sensitive(online_actions,online);
	gtk_action_group_set_sensitive(offline_actions,!online);

	if(keypad)
		gtk_widget_set_sensitive(keypad,online);


}

static void log_callback(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer id)
{
	WriteLog(id, "%s %s", log_domain, message);
	Trace("%s %s", log_domain, message);
}

static void set_fullscreen(int value, int reason)
{
 	Trace("Fullscren mode toggled (value: %d",value);

 	if(value)
		gtk_window_fullscreen(GTK_WINDOW(topwindow));
	else
		gtk_window_unfullscreen(GTK_WINDOW(topwindow));

}

static gboolean trylog(gchar *path)
{
	gboolean rc = (Set3270Log(path) == 0);

#ifdef DEBUG
	if(rc)
	{
		Trace("Writing log in %s",path);
	}
	else
	{
		Trace("Log in %s failed",path);
	}
#endif

	g_free(path);
	return rc;
}

static int program_init(const gchar *program)
{
	static const gchar	*logname	= PROGRAM_NAME ".log";
	gboolean				has_log		= FALSE;
	const gchar			*msg		= gtk_check_version(GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);

	if(msg)
	{
		// Invalid GTK version, notify user and exit
		int rc;
		GtkWidget *dialog = gtk_message_dialog_new(	NULL,
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_WARNING,
													GTK_BUTTONS_OK_CANCEL,
													_( "This program requires GTK version %d.%d.%d" ),GTK_MAJOR_VERSION,GTK_MINOR_VERSION,GTK_MICRO_VERSION );


		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",msg);
		gtk_window_set_title(GTK_WINDOW(dialog),_( "GTK Version mismatch" ));
		gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

        rc = gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);

        if(rc != GTK_RESPONSE_OK)
			return EINVAL;
	}

	/* If running on win32 changes to program path */
#if defined(_WIN32) || defined( DEBUG ) /*[*/
	has_log = trylog(g_build_filename(G_DIR_SEPARATOR_S, g_get_current_dir(),logname,NULL));
#endif

	/* Init Log system */
	if(!has_log)
		has_log = trylog(g_build_filename(G_DIR_SEPARATOR_S, "var","log",logname,NULL));

	if(!has_log)
		has_log = trylog(g_build_filename(g_get_home_dir(),"var","log",logname,NULL));

	if(!has_log)
		has_log = trylog(g_build_filename(g_get_home_dir(),"log",logname,NULL));

	if(!has_log)
		has_log = trylog(g_build_filename(g_get_home_dir(),PROGRAM_NAME,logname,NULL));

	if(!has_log)
		has_log = trylog(g_build_filename(g_get_home_dir(),PACKAGE_NAME,logname,NULL));

	if(!has_log)
		has_log = trylog(g_build_filename(g_get_tmp_dir(),logname,NULL));


	g_log_set_default_handler(log_callback,"GLog");

	gtk_rc_parse("etc/gtk-2.0/gtkrc");

	/* Start */
	Trace("%s","Opening configuration file");
	OpenConfigFile();

	Trace("%s","Opening configuration file");
	if(Register3270IOCallbacks(&program_io_callbacks))
	{
		PopupAnError( N_( "Can't register into lib3270 I/O callback table." ) );
		return -1;
	}

	Trace("%s","Setting screen callbacks");
	if(Register3270ScreenCallbacks(&program_screen_callbacks))
	{
		PopupAnError( N_( "Can't register into lib3270 screen callback table." ) );
		return -1;
	}

#ifdef X3270_FT
	Trace("%s","Starting FT feature");
	if(initft())
	{
		PopupAnError( N_( "Can't register into lib3270 file-transfer callback table." ) );
		return -1;
	}
#endif

	Trace("%s completed!",__FUNCTION__);
	return 0;
}

#ifdef HAVE_LIBGNOME
static gint save_session (GnomeClient *client, gint phase, GnomeSaveStyle save_style,
              gint is_shutdown, GnomeInteractStyle interact_style,
              gint is_fast, gpointer client_data)
{
	gchar** argv;
	guint argc;

	action_Save();

	/* allocate 0-filled, so it will be NULL-terminated */
	argv = g_malloc0(sizeof(gchar*)*2);
	argc = 0;

	argv[argc++] = client_data;
	argv[argc] = 0;

	gnome_client_set_clone_command(client, argc, argv);
	gnome_client_set_restart_command(client, argc, argv);

	Trace("Session for \"%s\" saved (Argc=%d)",(char *) client_data,argc);

	return TRUE;
}

static gint session_die(GnomeClient* client, gpointer client_data)
{
	WriteLog("GNOME","Exiting by request");
	gtk_main_quit();
	return FALSE;
}

#else

static int parse_option_context(GOptionContext *context, int *argc, char ***argv)
{
	GError *error = NULL;

	if(!g_option_context_parse( context, argc, argv, &error ))
    {
		g_print ( _( "Option parsing failed: %s\n" ), error->message);
		return -1;
    }

	return 0;
}

#endif

static void load_options(GOptionContext *context)
{
	static GOptionEntry entries[] =
	{
		{ "config-file",	 	'c', 0, G_OPTION_ARG_FILENAME, 	&program_config_filename_and_path,	N_( "Fixed path and filename for load/save the configuration data" ), 	NULL },
		{ "config",				'C', 0, G_OPTION_ARG_STRING,	&program_config_file,				N_( "Name of configuration file (auto-search)" ),						PROGRAM_NAME ".conf" },
		{ "host",				'h', 0, G_OPTION_ARG_STRING,	&cl_hostname,						N_( "Host identifier" ),												NULL },
		{ "startup-script", 	's', 0, G_OPTION_ARG_FILENAME, 	&startup_script,					N_( "Run script on startup (if available)" ),							NULL },
		{ "program-data",	 	'd', 0, G_OPTION_ARG_STRING, 	&program_data,						N_( "Path to search for data and configuration files" ),				NULL },
		{ "icon",	 			'i', 0, G_OPTION_ARG_FILENAME, 	&program_logo,						N_( "Path to an image file for program icon" ),							NULL },
		{ "window-title",	 	't', 0, G_OPTION_ARG_STRING, 	&window_title,						N_( "Main window title" ),												PROGRAM_NAME },

#ifdef HAVE_PLUGINS
		{ "plugins",	 		'p', 0, G_OPTION_ARG_STRING, 	&plugin_list,						N_( "Full path of plugins to load" ),									NULL },
#endif

		{ NULL }
	};

	const struct lib3270_option	*opt = get_3270_option_table(sizeof(struct lib3270_option));

	int				f;
	GOptionEntry	entry[2];
	GOptionGroup 	*group;

	g_option_context_add_main_entries(context, entries, NULL);

	Trace("%s","Setting default options");

	group = g_option_group_new( "lib3270", N_( "3270 options" ), N_( "Show lib3270 options" ), NULL, NULL);

	for(f=0;opt[f].name;f++)
	{
		memset(entry,0,sizeof(GOptionEntry) *2);

		entry->long_name = opt[f].name;
//		entry->short_name;
//		entry->flags;

		switch(opt[f].type)
		{
		case OPT_BOOLEAN:		// FIXME (perry#1#): How can I set a boolean option?
			entry->long_name = NULL;
			break;

		case OPT_STRING:
			entry->arg = G_OPTION_ARG_STRING;
			entry->arg_description = *((const gchar **) opt[f].aoff);
			break;

		case OPT_INTEGER:
			entry->arg = G_OPTION_ARG_INT;
			break;

		default:	// Ignore other options.
			entry->long_name = NULL;
		}

		if(entry->long_name)
		{
			while(*entry->long_name && *entry->long_name == '-')
				entry->long_name++;

			entry->arg_data = opt[f].aoff;
			entry->description = opt[f].description;
			// entry->arg_description;
			g_option_group_add_entries(group, entry);

		}
	}
	g_option_context_add_group(context,group);

}

int main(int argc, char *argv[])
{
	static GOptionContext	*context;

#ifdef HAVE_LIBGNOME

	static GnomeProgram	*gnome_program;

#endif

#if defined(_WIN32)
	{
		gchar *ptr = g_strdup(argv[0]);
		g_chdir(g_path_get_dirname(ptr));
		g_free(ptr);
	}
#endif

	setlocale( LC_ALL, "" );

#if defined( LOCALEDIR )

	// http://bo.majewski.name/bluear/gnu/GTK/i18n/
	Trace("Localedir: %s",LOCALEDIR);
	bindtextdomain(PACKAGE_NAME, LOCALEDIR);

#else

	Trace("Localedir: %s",DATAROOTDIR G_DIR_SEPARATOR_S "locale");
	bindtextdomain(PACKAGE_NAME, DATAROOTDIR G_DIR_SEPARATOR_S "locale" );

#endif

	bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
	textdomain(PACKAGE_NAME);

#ifdef HAVE_LIBGNOME

	context = g_option_context_new (_("- 3270 Emulator for Gnome"));

	load_options(context);

	gnome_program = gnome_program_init (	PACKAGE_NAME,
											PACKAGE_VERSION,
											LIBGNOMEUI_MODULE,
											argc, argv,
											GNOME_PARAM_GOPTION_CONTEXT, 		context,
											GNOME_PARAM_HUMAN_READABLE_NAME,	_("3270 Emulator"),
											NULL
								);

	client = gnome_master_client();
	gtk_signal_connect(GTK_OBJECT(client), "save_yourself", GTK_SIGNAL_FUNC(save_session), argv[0]);
	gtk_signal_connect(GTK_OBJECT(client), "die", GTK_SIGNAL_FUNC(session_die), NULL);

	Trace("Gnome session setup for client %p finished",client);

#else

	context = g_option_context_new (_("- 3270 Emulator for GTK+"));
	load_options(context);

	g_thread_init(NULL);
	gtk_init(&argc, &argv);

	if(parse_option_context(context,&argc, &argv))
		return -1;

#endif

	if(program_init(argv[0]))
		return -1;

	Trace("%s","Loading plugins");
	LoadPlugins();

	Trace("Initializing library with %s...",argv[0]);
	if(lib3270_init(argv[0]))
		return -1;

	if(CreateTopWindow())
		return -1;

	connect_main(0);

	register_schange(ST_CONNECT, connect_main);

#ifdef 	X3270_FT
	connect_3270(0);
	register_schange(ST_3270_MODE, connect_3270);
#else
	gtk_action_group_set_sensitive(ft_actions,FALSE);
#endif

	Trace("Topwindow: %p (%d) terminal: %p (%d)",topwindow,GTK_IS_WIDGET(topwindow),terminal,GTK_IS_WIDGET(terminal));

	if(!startup_script)
	{
		// No startup script, show main window
		gtk_widget_show(topwindow);
		gtk_widget_grab_focus(terminal);
		gtk_widget_grab_default(terminal);
	}

	register_tchange(FULL_SCREEN,set_fullscreen);
	register_tchange(MONOCASE,set_monocase);

	while(gtk_events_pending())
		gtk_main_iteration();

	/* Connect to the host. */
	screen_suspend();

	if(cl_hostname != CN)
		SetHostname(cl_hostname);
	else
		cl_hostname = GetString("Network","Hostname",CN);

	if(cl_hostname == CN)
	{
		action_SetHostname();
	}
	else
	{
		DisableNetworkActions();
		gtk_widget_set_sensitive(topwindow,FALSE);
		RunPendingEvents(0);

		if(host_connect(cl_hostname,1) == ENOTCONN)
		{
			Warning( N_( "Negotiation with %s failed!" ),cl_hostname);
		}

		gtk_widget_set_sensitive(topwindow,TRUE);
		gtk_widget_grab_focus(terminal);

		RunPendingEvents(0);
	}

	if(topwindow)
	{
		screen_resume();
		screen_disp();
//		peer_script_init();

		// Start plugins after the creation of main loop
		g_timeout_add((guint) 10, (GSourceFunc) StartPlugins, (gpointer) startup_script);

		// Run main loop
		gtk_main();
	}

	UnloadPlugins();
	CloseConfigFile();
	return 0;
}



