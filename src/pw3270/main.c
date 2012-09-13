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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como main.c e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 *
 */

#include <glib.h>
#include <glib/gstdio.h>
#include "globals.h"

#ifdef HAVE_GTKMAC
 #include <gtkosxapplication.h>
#endif // HAVE_GTKMAC

#include <v3270.h>
#include "v3270/accessible.h"
#include <stdlib.h>

/*--[ Statics ]--------------------------------------------------------------------------------------*/

 static GtkWidget *toplevel = NULL;

#ifdef HAVE_GTKMAC
 GtkOSXApplication	* osxapp	= NULL;
#endif // HAVE_GTKMAC

/*--[ Implement ]------------------------------------------------------------------------------------*/

static int initialize(void)
{
	const gchar * msg = gtk_check_version(GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);

	if(msg)
	{
		// Invalid GTK version, notify user
		int rc;

		GtkWidget *dialog = gtk_message_dialog_new(	NULL,
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_WARNING,
													GTK_BUTTONS_OK_CANCEL,
													_( "%s requires GTK version %d.%d.%d" ),PACKAGE_NAME,GTK_MAJOR_VERSION,GTK_MINOR_VERSION,GTK_MICRO_VERSION );


		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog),"%s",msg);
		gtk_window_set_title(GTK_WINDOW(dialog),_( "GTK Version mismatch" ));
		gtk_window_set_deletable(GTK_WINDOW(dialog),FALSE);

        rc = gtk_dialog_run(GTK_DIALOG (dialog));
        gtk_widget_destroy(dialog);

        if(rc != GTK_RESPONSE_OK)
			return EINVAL;
	}

	return 0;
}

static void toplevel_setup(GtkWindow *window)
{
 	gchar * name		= g_strdup_printf("%s.png",g_get_application_name());
	gchar * filename 	= pw3270_build_filename(GTK_WIDGET(window),name,NULL);
 	gchar * role		= g_strdup_printf("%s_top",g_get_application_name());

	gtk_window_set_type_hint(window,GDK_WINDOW_TYPE_HINT_NORMAL);
	gtk_window_set_position(window,GTK_WIN_POS_CENTER);
	gtk_window_set_role(window,role);

	g_free(role);

	// Set default icon
	if(g_file_test(filename,G_FILE_TEST_EXISTS))
	{
		GError * error = NULL;

		trace("Loading default icon from %s",filename);

		if(!gtk_window_set_default_icon_from_file(filename,&error))
		{
			g_warning("Error %s loading default icon from %s",error->message,filename);
			g_error_free(error);
		}
	}

	g_free(filename);
	g_free(name);
}

#if ! defined( WIN32 )
static gboolean appname(const gchar *option_name, const gchar *value, gpointer data,GError **error)
{
	g_set_application_name(value);
	return TRUE;
}
#endif // !win32

int main(int argc, char *argv[])
{
#if defined( WIN32 )
	static const gchar	* appname		= PACKAGE_NAME;
#endif // WIN32

	static const gchar	* session_name	= PACKAGE_NAME;
	static const gchar	* host			= NULL;
	int 				  rc 			= 0;

#if ! GLIB_CHECK_VERSION(2,32,0)
	g_thread_init(NULL);
#endif // !GLIB(2,32)

	gtk_init(&argc, &argv);

	// Setup locale
#ifdef LC_ALL
	setlocale( LC_ALL, "" );
#endif

#if defined( WIN32 )
	{
		gchar * appdir = g_win32_get_package_installation_directory_of_module(NULL);
		gchar * locdir = g_build_filename(appdir,"locale",NULL);

		g_chdir(appdir);
		bindtextdomain( PACKAGE_NAME, locdir );

		g_free(locdir);
		g_free(appdir);

	}
#elif defined(HAVE_GTKMAC)
	{
		GtkMacBundle * macbundle = gtk_mac_bundle_get_default();

		g_chdir(gtk_mac_bundle_get_datadir(macbundle));
		bindtextdomain(PACKAGE_NAME,gtk_mac_bundle_get_localedir(macbundle));

		osxapp = GTK_OSX_APPLICATION(g_object_new(GTK_TYPE_OSX_APPLICATION,NULL));

	}
#elif defined( DATAROOTDIR )
	{
		gchar * appdir = g_build_filename(DATAROOTDIR,PACKAGE_NAME,NULL);
		gchar * locdir = g_build_filename(DATAROOTDIR,"locale",NULL);

		g_chdir(appdir);
		bindtextdomain( PACKAGE_NAME, locdir);

		g_free(locdir);
		g_free(appdir);

	}
#endif // DATAROOTDIR

	bind_textdomain_codeset(PACKAGE_NAME, "UTF-8");
	textdomain(PACKAGE_NAME);

	// Process command-line options
	{
		static const GOptionEntry app_options[] =
		{
#if ! defined( WIN32 )
			{ "appname",		'a', 0, G_OPTION_ARG_CALLBACK,	appname,		N_( "Application name" ),		PACKAGE_NAME	},
#else
			{ "appname",		'a', 0, G_OPTION_ARG_STRING,	&appname,		N_( "Application name" ),		PACKAGE_NAME	},
#endif // WIN32
			{ "session",		's', 0, G_OPTION_ARG_STRING,	&session_name,	N_( "Session name" ),			PACKAGE_NAME	},
			{ "host",			'h', 0, G_OPTION_ARG_STRING,	&host,			N_( "Host to connect"),			NULL			},
			{ NULL }
		};

		GOptionContext	* options	= g_option_context_new (_("- 3270 Emulator for Gtk"));
		GError			* error		= NULL;


		g_option_context_add_main_entries(options, app_options, NULL);

		if(!g_option_context_parse( options, &argc, &argv, &error ))
		{
			int f;
			GString 	* str;
			GtkWidget 	* dialog = gtk_message_dialog_new(	NULL,
														GTK_DIALOG_DESTROY_WITH_PARENT,
														GTK_MESSAGE_ERROR,
														GTK_BUTTONS_CANCEL,
														"%s", error->message);

			gtk_window_set_title(GTK_WINDOW(dialog),_( "Parse error" ));

			str = g_string_new( _( "<b>Valid options:</b>\n\n" ) );

			for(f=0;app_options[f].description;f++)
				g_string_append_printf(str,"--%-20s\t%s\n",app_options[f].long_name,gettext(app_options[f].description));

			gtk_message_dialog_format_secondary_markup(GTK_MESSAGE_DIALOG(dialog), "%s", str->str);

			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);

			g_error_free(error);
			g_string_free(str,TRUE);

			return -1;
		}
	}

#if defined(WIN32)
	g_set_application_name(appname);
#endif // WIN32

	rc = initialize();
	if(!rc)
	{
		GtkSettings *settings = gtk_settings_get_default();

		if(settings)
		{
			// http://developer.gnome.org/gtk/2.24/GtkSettings.html
			gtk_settings_set_string_property(settings,"gtk-menu-bar-accel","Menu","");
		}

		toplevel = pw3270_new(host);
		pw3270_set_session_name(toplevel,session_name);

		toplevel_setup(GTK_WINDOW(toplevel));

		if(pw3270_get_toggle(toplevel,LIB3270_TOGGLE_FULL_SCREEN))
			gtk_window_fullscreen(GTK_WINDOW(toplevel));
		else
			pw3270_restore_window(toplevel,"toplevel");

		init_plugins(toplevel);
		gtk_window_present(GTK_WINDOW(toplevel));

#ifdef HAVE_GTKMAC
		gtk_osxapplication_ready(osxapp);
#endif // HAVE_GTKMAC

		gtk_main();

		deinit_plugins(toplevel);

	}

	return rc;
}
