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
#include "oia.h"
#include "actions.h"

#include <stdio.h>

#if defined( HAVE_LIBGNOME )
	#include <gnome.h>
#endif

#include <glib/gstdio.h>

#include "globals.h"

#if !defined(_WIN32)
	#include <sys/wait.h>
	#include <signal.h>
#endif

#include <errno.h>
#include <lib3270/toggle.h>


/*---[ Globals ]------------------------------------------------------------------------------------------------*/

#if defined( HAVE_IGEMAC )

GtkOSXApplication	* osxapp = 0;

#elif defined( HAVE_LIBGNOME )

static GnomeClient	* client = 0;

#endif

const char			* on_lu_command		= NULL;
H3270				* hSession			= NULL;

static const char	* cl_hostname		= NULL;
static const char	* startup_script	= NULL;
static gchar		* gtk_theme 		= NULL;
static gchar		* log_filename		= NULL;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

/* Callback for connection state changes. */
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
		set_action_group_sensitive_state(ACTION_GROUP_FT,status);
#endif
	}

	if(terminal)
	{
		gtk_widget_set_sensitive(terminal,online);
		update_oia_element(OIA_ELEMENT_CONNECTION_STATUS);
		update_oia_element(OIA_ELEMENT_UNDERA);
		gtk_widget_grab_focus(terminal);
	}

	set_action_group_sensitive_state(ACTION_GROUP_ONLINE,online);
	set_action_group_sensitive_state(ACTION_GROUP_OFFLINE,!online);

	if(online)
		check_clipboard_contents();
	else
		set_action_group_sensitive_state(ACTION_GROUP_PASTE,FALSE);

	keypad_set_sensitive(topwindow,online);

	#ifdef HAVE_IGEMAC
		gtk_osxapplication_attention_request(osxapp,INFO_REQUEST);
	#endif

}

static void log_callback(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer id)
{
	WriteLog(id, "%s %s", log_domain, message);
	Trace("%s %s", log_domain, message);
}

static void set_fullscreen(int value, enum toggle_type reason)
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
	return rc == 0;
}

static int show_lib3270_register_error(const gchar *msg)
{
	int rc;
	GtkWidget *dialog = gtk_message_dialog_new(	NULL,
											   GTK_DIALOG_DESTROY_WITH_PARENT,
											   GTK_MESSAGE_ERROR,
											   GTK_BUTTONS_CANCEL,
											   "%s", msg);


	gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),_( "Please, check if lib3270 version is %s" ),program_version);
	gtk_window_set_title(GTK_WINDOW(dialog),_( "3270 library failed" ));

#if GTK_CHECK_VERSION(2,10,0)
	gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
#endif

	rc = gtk_dialog_run(GTK_DIALOG (dialog));
	gtk_widget_destroy(dialog);

	return -1;
}


static int program_init(void)
{
	static const gchar	*logname	= PROGRAM_NAME ".log";
	gboolean				has_log		= FALSE;
	const gchar			*msg		= gtk_check_version(GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
	gchar					*ptr;

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

#if GTK_CHECK_VERSION(2,10,0)
		gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);
#endif

        rc = gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);

        if(rc != GTK_RESPONSE_OK)
			return EINVAL;
	}

	/* Try command-line defined log */
	if(log_filename)
		has_log = (Set3270Log(log_filename) == 0);

	Trace("Log is %s",has_log ? "set" : "unset");

	/* If running on win32 try to save log in program path */
#if defined(_WIN32) || defined( DEBUG ) /*[*/
	if(!has_log)
		has_log = trylog(g_build_filename(program_data, G_DIR_SEPARATOR_S, logname, NULL));
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

	/* Start */
	Trace("%s","Opening configuration file");
	OpenConfigFile();

	if(gtk_theme)
		ptr = g_strdup(gtk_theme);
	else
	{
#if defined(WIN32)
		ptr = GetString( "gtk", "theme", "themes/MS-Windows/gtk-2.0/gtkrc");
#else
		ptr = GetString( "gtk", "theme", "");
#endif
	}

	if(ptr)
	{
		if(*ptr && g_file_test(ptr,G_FILE_TEST_IS_REGULAR))
			gtk_rc_parse(ptr);
		g_free(ptr);
	}

	Trace("%s","Opening configuration file");
	if(Register3270IOCallbacks(&program_io_callbacks))
		return show_lib3270_register_error(_( "Can't register as I/O manager." ));

	Trace("%s","Setting screen callbacks");
	if(Register3270ScreenCallbacks(&program_screen_callbacks))
		return show_lib3270_register_error(_( "Can't register as screen manager." ));


#ifdef X3270_FT
	Trace("%s","Starting FT feature");
	if(initft())
		return show_lib3270_register_error(_( "Can't register as file-transfer manager." ));
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
		GtkWidget *dialog = gtk_message_dialog_new(	NULL,
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CANCEL,
													"%s", _(  "Option parsing failed." ));

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Parse error" ));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);

		return -1;
    }

	return 0;
}

#endif

static void load_options(GOptionContext *context)
{
	static GOptionEntry entries[] =
	{
		{ "config-file",	 	'c', 0, G_OPTION_ARG_FILENAME, 	&program_config_filename_and_path,	N_( "Fixed path and filename for load/save the configuration data" ), 		NULL			},
		{ "config",				'C', 0, G_OPTION_ARG_STRING,	&program_config_file,				N_( "Name of configuration file (auto-search)" ),							PROGRAM_NAME ".conf" },
		{ "host",				'h', 0, G_OPTION_ARG_STRING,	&cl_hostname,						N_( "Host identifier" ),													NULL			},
		{ "startup-script", 	's', 0, G_OPTION_ARG_FILENAME, 	&startup_script,					N_( "Run scripts on startup (Example: script1(arg1,arg2);script2(arg)" ),	NULL			},
		{ "program-data",	 	'd', 0, G_OPTION_ARG_STRING, 	&program_data,						N_( "Path to search for data and configuration files" ),					NULL			},
		{ "icon",	 			'i', 0, G_OPTION_ARG_FILENAME, 	&program_logo,						N_( "Path to an image file for program icon" ),								NULL			},
		{ "window-title",	 	't', 0, G_OPTION_ARG_STRING, 	&window_title,						N_( "Main window title" ),													PROGRAM_NAME	},
		{ "theme",				'T', 0, G_OPTION_ARG_FILENAME,	&gtk_theme,							N_( "Theme file (gtkrc)" ),													NULL 			},
		{ "log",				'l', 0, G_OPTION_ARG_FILENAME,	&log_filename,						N_( "Log file" ),															NULL			},
		{ "on-lu",				'L', 0, G_OPTION_ARG_STRING,	&on_lu_command,						N_( "Command to run when LU name is available" ),							NULL			},

#ifdef HAVE_PLUGINS
		{ "plugins",	 		'p', 0, G_OPTION_ARG_STRING, 	&plugin_list,						N_( "Full path of plugins to load" ),										NULL			},
		{ "plugin-path",	 	'P', 0, G_OPTION_ARG_STRING, 	&plugin_path,						N_( "Path to search for plugins" ),											NULL			},
#endif

		{ NULL }
	};

	const struct lib3270_option	*opt = get_3270_option_table(sizeof(struct lib3270_option));

	int				f;
	GOptionEntry	entry[2];
	GOptionGroup 	*group;

	g_option_context_add_main_entries(context, entries, NULL);

//	Trace("%s","Setting default options");

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
			entry->description = gettext( opt[f].description );
			// entry->arg_description;
			g_option_group_add_entries(group, entry);

		}
	}
	g_option_context_add_group(context,group);

}

int main(int argc, char *argv[])
{
	static GOptionContext	*context;
	int rc = 0;

#ifdef HAVE_LIBGNOME

	static GnomeProgram	*gnome_program;

#endif

#ifdef LC_ALL
	setlocale( LC_ALL, "" );
#endif

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

#if defined( HAVE_IGEMAC )

	context = g_option_context_new (_("- 3270 Emulator for Gtk-OSX"));
	load_options(context);

	g_thread_init(NULL);
	gtk_init(&argc, &argv);
	osxapp = g_object_new(GTK_TYPE_OSX_APPLICATION, NULL);

	if(parse_option_context(context,&argc, &argv))
		return -1;

#elif defined( HAVE_LIBGNOME )

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

	if(!program_data)
	{
#if defined( HAVE_IGEMAC )

		program_data = gtk_osxapplication_get_bundle_path(osxapp);

#elif defined( WIN32 )

		gchar *ptr = g_path_get_dirname(argv[0]);
		g_chdir(ptr);
		g_free(ptr);
		program_data = g_get_current_dir();

#elif defined( DATAROOTDIR )

		program_data = DATAROOTDIR G_DIR_SEPARATOR_S PROGRAM_NAME;
		g_chdir(program_data);

#else

		#error DATAROOTDIR is undefined

#endif
	}

	Trace("Program data: \"%s\"",program_data);

	if(program_init())
		return -1;

	Trace("%s","Loading plugins");
	LoadPlugins();

	Trace("Initializing library with %s...",argv[0]);
	hSession = new_3270_session();

	if(rc)
	{
		GtkWidget *dialog = gtk_message_dialog_new(	NULL,
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CANCEL,
													"%s", _(  "3270 library failed to start" ));

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Can't start" ));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), _( "Return code was %d" ), rc);

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);

		return rc;
	}

	LoadColors();

	if(CreateTopWindow())
	{
		GtkWidget *dialog = gtk_message_dialog_new(	NULL,
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CANCEL,
													"%s", _(  "Top window failed to start" ));

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Can't start" ));

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
		return -1;
	}

	connect_main(0);

	register_schange(ST_CONNECT, connect_main);

//	Trace("Topwindow: %p (%d) terminal: %p (%d)",topwindow,GTK_IS_WIDGET(topwindow),terminal,GTK_IS_WIDGET(terminal));

	register_tchange(FULL_SCREEN,set_fullscreen);
	register_tchange(MONOCASE,set_monocase);

	update_toggle_actions();
	init_gui_toggles();

	if(!startup_script)
	{
		// No startup script, show main window
		gtk_widget_show(topwindow);
		gtk_widget_grab_focus(terminal);
		gtk_widget_grab_default(terminal);
	}

	while(gtk_events_pending())
		gtk_main_iteration();

	/* Connect to the host. */
	screen_suspend();

	if(cl_hostname != CN)
		SetString("Network","Hostname",cl_hostname);
	else
		cl_hostname = GetString("Network","Hostname",CN);

    if(TOGGLED_CONNECT_ON_STARTUP)
        action_Connect();

	if(topwindow)
	{
		screen_resume();
		screen_disp();
//		peer_script_init();

		// Start plugins after the creation of main loop
		g_timeout_add((guint) 10, (GSourceFunc) StartPlugins, (gpointer) startup_script);

		// Run main loop
#if defined( HAVE_IGEMAC )
		gtk_osxapplication_ready(osxapp);
#endif

		gtk_main();
		Trace("%s --- Loop de mensagems encerrou",__FUNCTION__);
	}
	else
	{
		GtkWidget *dialog = gtk_message_dialog_new(	NULL,
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_ERROR,
													GTK_BUTTONS_CANCEL,
													"%s", _(  "Program window was destroyed" ));

		gtk_window_set_title(GTK_WINDOW(dialog),_( "Program aborted" ));

        gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);
		return -1;
	}

	Trace("%s","Stopping");
	StopPlugins();

	Trace("%s","Unloading");
	UnloadPlugins();

	Trace("%s","Releasing config");
	CloseConfigFile();

	Trace("%s finished (rc=%d)",argv[0],rc);
	gtk_exit(rc);
	return rc;
}



