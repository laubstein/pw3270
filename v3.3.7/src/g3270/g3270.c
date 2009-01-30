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
 * Este programa está nomeado como g3270.c e possui 552 linhas de código.
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

#include "g3270.h"

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
#include "printerc.h"
#include "togglesc.h"
#include "utilc.h"
#include "xioc.h"

#if defined(_WIN32) /*[*/
#include <windows.h>
#include "winversc.h"
#include "windirsc.h"
#endif /*]*/

#include <lib3270/hostc.h>

#if !defined(_WIN32) /*[*/
/* Base keymap. */
static char *base_keymap1 =
"Ctrl<Key>]: Escape\n"
"Ctrl<Key>a Ctrl<Key>a: Key(0x01)\n"
"Ctrl<Key>a Ctrl<Key>]: Key(0x1d)\n"
"Ctrl<Key>a <Key>c: Clear\n"
"Ctrl<Key>a <Key>e: Escape\n"
"Ctrl<Key>a <Key>i: Insert\n"
"Ctrl<Key>a <Key>r: Reset\n"
"Ctrl<Key>a <Key>l: Redraw\n"
"Ctrl<Key>a <Key>m: Compose\n"
"Ctrl<Key>a <Key>^: Key(notsign)\n"
"<Key>DC: Delete\n"
"<Key>UP: Up\n"
"<Key>DOWN: Down\n"
"<Key>LEFT: Left\n"
"<Key>RIGHT: Right\n"
"<Key>HOME: Home\n"
"Ctrl<Key>a <Key>1: PA(1)\n"
"Ctrl<Key>a <Key>2: PA(2)\n"
"Ctrl<Key>a <Key>3: PA(3)\n";
static char *base_keymap2 =
"<Key>F1: PF(1)\n"
"Ctrl<Key>a <Key>F1: PF(13)\n"
"<Key>F2: PF(2)\n"
"Ctrl<Key>a <Key>F2: PF(14)\n"
"<Key>F3: PF(3)\n"
"Ctrl<Key>a <Key>F3: PF(15)\n"
"<Key>F4: PF(4)\n"
"Ctrl<Key>a <Key>F4: PF(16)\n"
"<Key>F5: PF(5)\n"
"Ctrl<Key>a <Key>F5: PF(17)\n"
"<Key>F6: PF(6)\n"
"Ctrl<Key>a <Key>F6: PF(18)\n";
static char *base_keymap3 =
"<Key>F7: PF(7)\n"
"Ctrl<Key>a <Key>F7: PF(19)\n"
"<Key>F8: PF(8)\n"
"Ctrl<Key>a <Key>F8: PF(20)\n"
"<Key>F9: PF(9)\n"
"Ctrl<Key>a <Key>F9: PF(21)\n"
"<Key>F10: PF(10)\n"
"Ctrl<Key>a <Key>F10: PF(22)\n"
"<Key>F11: PF(11)\n"
"Ctrl<Key>a <Key>F11: PF(23)\n"
"<Key>F12: PF(12)\n"
"Ctrl<Key>a <Key>F12: PF(24)\n";

/* Base keymap, 3270 mode. */
static char *base_3270_keymap =
"Ctrl<Key>a <Key>a: Attn\n"
"Ctrl<Key>c: Clear\n"
"Ctrl<Key>d: Dup\n"
"Ctrl<Key>f: FieldMark\n"
"Ctrl<Key>h: Erase\n"
"Ctrl<Key>i: Tab\n"
"Ctrl<Key>j: Newline\n"
"Ctrl<Key>l: Redraw\n"
"Ctrl<Key>m: Enter\n"
"Ctrl<Key>r: Reset\n"
"Ctrl<Key>u: DeleteField\n"
"<Key>IC: ToggleInsert\n"
"<Key>DC: Delete\n"
"<Key>BACKSPACE: Erase\n"
"<Key>HOME: Home\n"
"<Key>END: FieldEnd\n";

#else /*][*/

/* Base keymap. */
static char *base_keymap =
       "Alt <Key>1:      PA(1)\n"
       "Alt <Key>2:      PA(2)\n"
       "Alt <Key>3:      PA(3)\n"
  "Alt Ctrl <Key>]:      Key(0x1d)\n"
      "Ctrl <Key>]:      Escape\n"
       "Alt <Key>^:      Key(notsign)\n"
       "Alt <Key>c:      Clear\n"
       "Alt <Key>l:      Redraw\n"
       "Alt <Key>m:      Compose\n"
     "Shift <Key>F1:     PF(13)\n"
     "Shift <Key>F2:     PF(14)\n"
     "Shift <Key>F3:     PF(15)\n"
     "Shift <Key>F4:     PF(16)\n"
     "Shift <Key>F5:     PF(17)\n"
     "Shift <Key>F6:     PF(18)\n"
     "Shift <Key>F7:     PF(19)\n"
     "Shift <Key>F8:     PF(20)\n"
     "Shift <Key>F9:     PF(21)\n"
     "Shift <Key>F10:    PF(22)\n"
     "Shift <Key>F11:    PF(23)\n"
     "Shift <Key>F12:    PF(24)\n";

/* Base keymap, 3270 mode. */
static char *base_3270_keymap =
      "Ctrl <Key>a:      Attn\n"
       "Alt <Key>a:      Attn\n"
      "Ctrl <Key>c:      Clear\n"
      "Ctrl <Key>d:      Dup\n"
       "Alt <Key>d:      Dup\n"
      "Ctrl <Key>f:      FieldMark\n"
       "Alt <Key>f:      FieldMark\n"
      "Ctrl <Key>h:      Erase\n"
       "Alt <Key>i:      Insert\n"
"Shift Ctrl <Key>i:      BackTab\n"
      "Ctrl <Key>i:      Tab\n"
      "Ctrl <Key>j:      Newline\n"
      "Ctrl <Key>l:      Redraw\n"
      "Ctrl <Key>m:      Enter\n"
      "Ctrl <Key>r:      Reset\n"
       "Alt <Key>r:      Reset\n"
      "Ctrl <Key>u:      DeleteField\n"
      "Ctrl <Key>v:      Paste\n"
           "<Key>INSERT: ToggleInsert\n"
     "Shift <Key>TAB:    BackTab\n"
           "<Key>BACK:   Erase\n"
     "Shift <Key>END:    EraseEOF\n"
           "<Key>END:    FieldEnd\n"
     "Shift <Key>LEFT:   PreviousWord\n"
     "Shift <Key>RIGHT:  NextWord\n";
#endif /*]*/

/* Globals */
#ifdef HAVE_LIBGNOME
GnomeClient *client = 0;
#endif


/* Callback for connection state changes. */
#ifdef 	X3270_FT
static void connect_3270(Boolean status)
{
	gtk_action_group_set_sensitive(ft_actions,status);
}
#endif

static void connect_main(Boolean status)
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

static int g3270_init(int *argc, char ***argv)
{
	static const gchar	*logname	= PACKAGE_NAME ".log";
	gboolean				has_log		= FALSE;

	/* If running on win32 changes to program path */
#if defined(_WIN32) || defined( DEBUG ) /*[*/
	gchar *ptr = g_strdup(*argv[0]);
	g_chdir(g_path_get_dirname(ptr));
	g_free(ptr);
	Trace("Current dir: %s",g_get_current_dir());
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
		has_log = trylog(g_build_filename(g_get_home_dir(),PACKAGE_NAME,logname,NULL));

	if(!has_log)
		has_log = trylog(g_build_filename(g_get_tmp_dir(),logname,NULL));


	g_log_set_default_handler(log_callback,"GLog");

	gtk_rc_parse("etc/gtk-2.0/gtkrc");

	/* Start */
	OpenConfigFile();

	if(Register3270IOCallbacks(&g3270_io_callbacks))
	{
		PopupAnError( N_( "Can't register into lib3270 I/O callback table." ) );
		return -1;
	}

	if(Register3270ScreenCallbacks(&g3270_screen_callbacks))
	{
		PopupAnError( N_( "Can't register into lib3270 screen callback table." ) );
		return -1;
	}

#ifdef X3270_FT
	if(initft())
	{
		PopupAnError( N_( "Can't register into lib3270 file-transfer callback table." ) );
		return -1;
	}
#endif

	Trace("%s completed!",__FUNCTION__);
	return 0;
}

int wait4negotiations(const char *cl_hostname)
{
	Trace("Waiting for negotiations with %s to complete or fail",cl_hostname);

	gtk_widget_set_sensitive(topwindow,FALSE);

	while(!IN_ANSI && !IN_3270)
	{
		while(gtk_events_pending())
			gtk_main_iteration();

		if(!topwindow)
		{
			Log("Connection with %s aborted by request",cl_hostname);
			return EINTR;
		}

		if(!PCONNECTED)
		{
			PopupAnError( N_( "Negotiation with %s failed!" ),cl_hostname);
			gtk_widget_set_sensitive(topwindow,TRUE);
			return EINVAL;
		}
	}

	Trace("Negotiations with %s completed",cl_hostname);

	gtk_widget_set_sensitive(topwindow,TRUE);
	gtk_widget_grab_focus(terminal);
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
#endif

static void init_locale(void)
{
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

}

int main(int argc, char *argv[])
{
#ifdef HAVE_LIBGNOME

/*
	static GOptionEntry entries[] =
	{
		{ "title", 't', 0, G_OPTION_ARG_STRING, &window_title, N_( "Window title" ), PACKAGE_NAME },

		{ NULL }
	};
*/

	static GnomeProgram	*gnome_program;
	static GOptionContext	*context;

#endif

	const char	*cl_hostname = CN;

	init_locale();

	initialize_toggles();

#ifdef HAVE_LIBGNOME

	context = g_option_context_new (_("- 3270 Emulator for Gnome"));

//	g_option_context_add_main_entries(context, entries, PACKAGE_NAME);
//	g_option_context_add_group (context, gtk_get_option_group (TRUE));

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

	g_thread_init(NULL);
	gtk_init(&argc, &argv);

#endif

	if(g3270_init(&argc,&argv))
		return -1;

#ifdef DEBUG
	{
		int f;

		Trace("%s started with %d arguments",argv[0],argc);

		for(f = 0; f<argc;f++)
		{
			Trace("Arg(%d) = \"%s\"",f,argv[f]);
		}
	}
#endif

	LoadPlugins();

	add_resource("keymap.base",
#if defined(_WIN32) /*[*/
	    base_keymap
#else /*][*/
	    xs_buffer("%s%s%s", base_keymap1, base_keymap2, base_keymap3)
#endif /*]*/
	    );
	add_resource("keymap.base.3270", NewString(base_3270_keymap));

	cl_hostname = lib3270_init(&argc, (const char **)argv);

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

//	register_schange(ST_EXITING, main_exiting);

#if defined(X3270_PRINTER) /*[*/
	printer_init();
#endif /*]*/

	Trace("Topwindow: %p (%d) terminal: %p (%d)",topwindow,GTK_IS_WIDGET(topwindow),terminal,GTK_IS_WIDGET(terminal));
	gtk_widget_show(topwindow);

	gtk_widget_grab_focus(terminal);
	gtk_widget_grab_default(terminal);

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
		if(host_connect(cl_hostname) >= 0)
			wait4negotiations(cl_hostname);
	}

	if(topwindow)
	{
		screen_resume();
		screen_disp();
		peer_script_init();
		gtk_main();
	}

	UnloadPlugins();
	CloseConfigFile();
	return 0;
}


