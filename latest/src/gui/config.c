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
 * Este programa está nomeado como config.c e possui 389 linhas de código.
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
 #include "gui.h"
 #include <ctype.h>
 #include <sys/stat.h>
 #include <string.h>
 #include <lib3270/toggle.h>

 #define CHECK_FILENAME(...)	filename = g_build_filename( __VA_ARGS__, NULL ); \
								if(g_file_test(filename,G_FILE_TEST_IS_REGULAR)) \
									return filename; \
								g_free(filename);


/*---[ Statics ]------------------------------------------------------------------------------------------------*/

 static const struct _WindowState
 {
	const char *name;
	GdkWindowState flag;
	void (*activate)(GtkWindow *);
 } WindowState[] =
 {
	{ "Maximized",  GDK_WINDOW_STATE_MAXIMIZED,		gtk_window_maximize             },
	{ "Iconified",  GDK_WINDOW_STATE_ICONIFIED,		gtk_window_iconify              },
	{ "Sticky",		GDK_WINDOW_STATE_STICKY,		gtk_window_stick                }
 };

 static GKeyFile	* program_config = NULL;
 gchar				* program_config_file = PROGRAM_NAME ".conf";
 gchar				* program_config_filename_and_path = NULL;
 gchar				* program_data = NULL;


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static gchar * FindConfigFile(void)
 {
// 	static const gchar 	*name = program_config_file;
 	gchar					*filename;

	// Check for pre-defined name
	if(program_config_filename_and_path)
		return program_config_filename_and_path;

	// Search for user's configuration file.
	filename = g_build_filename( g_get_user_config_dir(),program_config_file,NULL );
	Trace("Checking for %s",filename);
	if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		return program_config_filename_and_path = filename;
	g_free(filename);

	filename = g_strdup_printf("%s%c.%s", g_get_home_dir(), G_DIR_SEPARATOR, program_config_file);
	Trace("Checking for %s",filename);
	if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		return program_config_filename_and_path = filename;
	g_free(filename);


	// Can't find user's configuration file, check program_data for default one
	filename = g_build_filename(program_data, program_config_file,NULL);
	if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		return program_config_filename_and_path = filename;
	g_free(filename);


	return 0;
 }


 int OpenConfigFile(void)
 {
	int	 f;
	gchar *filename = FindConfigFile();

	Trace("Configuration file: %s",filename);

	if(program_config)
	{
		g_key_file_free(program_config);
		program_config = NULL;
	}

	program_config = g_key_file_new();

	Trace("Configuration file: %s - %p",filename,program_config);

	if(filename)
		g_key_file_load_from_file(program_config,filename,G_KEY_FILE_NONE,NULL);

	/* Load initial settings */
	Trace("Loading %d toggles",N_TOGGLES);

	for(f=0;f<N_TOGGLES;f++)
	{
 		const char *name = get_toggle_name(f);

 		Trace("Setting toggle(%d): %s",f,name);

		if(g_key_file_has_key(program_config,"Toggles",name,NULL))
			set_toggle(f,g_key_file_get_boolean(program_config,"Toggles",name,NULL));
	}

	Trace("%s loaded",filename);
 	return 0;
 }

 gboolean GetBoolean(const gchar *group, const gchar *key, const gboolean def)
 {
	if(!g_key_file_has_key(program_config,group,key,NULL))
		return def;
	return g_key_file_get_boolean(program_config,group,key,NULL);
 }

 void SetBoolean(const gchar *group, const gchar *key, const gboolean val)
 {
	g_key_file_set_boolean(program_config,group,key,val);
 }

 int SaveConfigFile(void)
 {
 	gchar	*ptr;
 	gchar	*filename;
 	int		f;

	for(f=0;f<N_TOGGLES;f++)
		g_key_file_set_boolean(program_config,"Toggles",get_toggle_name(f),Toggled(f));

 	ptr = g_key_file_to_data(program_config,NULL,NULL);

	Trace("Configuration data: %p",ptr);

	if(ptr)
	{
		gchar *buffer;

		for(buffer = ptr;*ptr && isspace(*ptr);ptr++);

		// Try pre-defined file
		if(program_config_filename_and_path)
		{
			if(g_file_set_contents(program_config_filename_and_path,ptr,-1,NULL))
			{
				Trace("Configuration data saved on %s",program_config_filename_and_path);
				g_free(buffer);
				return 0;
			}
		}

		g_free(program_config_filename_and_path);
		program_config_filename_and_path = NULL;

		// Try user configuration dir
		filename = g_build_filename( g_get_user_config_dir(), program_config_file, NULL);
		if(g_file_set_contents(filename,ptr,-1,NULL))
		{
			program_config_filename_and_path = filename;
			Trace("Configuration data saved on %s",program_config_filename_and_path);
			g_free(buffer);
			return 0;
		}

#ifndef WIN32
		Trace("Creating %s",g_get_user_config_dir());
		g_mkdir_with_parents(g_get_user_config_dir(),S_IRUSR|S_IWUSR);
		if(g_file_set_contents(filename,ptr,-1,NULL))
		{
			program_config_filename_and_path = filename;
			Trace("Configuration data saved on %s",program_config_filename_and_path);
			g_free(buffer);
			return 0;
		}
#endif
		g_free(filename);

		// Try user home dir
		filename = g_strdup_printf("%s%c.%s", g_get_home_dir(), G_DIR_SEPARATOR, program_config_file);
		if(g_file_set_contents(filename,ptr,-1,NULL))
		{
			program_config_filename_and_path = filename;
			Trace("Configuration data saved on %s",program_config_filename_and_path);
			g_free(buffer);
			return 0;
		}
		g_free(filename);

		g_free(buffer);

		Trace("Can't save, showing error dialog (topwindow: %p program_config_file: %p)",topwindow,program_config_file);

		if(topwindow)
		{
			GtkWidget *dialog;

			// Can't save configuration data, notify user
			dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
												GTK_DIALOG_DESTROY_WITH_PARENT,
												GTK_MESSAGE_ERROR,
												GTK_BUTTONS_OK,
												_(  "Can't save %s" ), program_config_file);

			gtk_window_set_title(GTK_WINDOW(dialog), _( "Error saving file" ) );
			gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), _( "Unable to save configuration data in %s" ), g_get_user_config_dir());

			gtk_dialog_run(GTK_DIALOG (dialog));
			gtk_widget_destroy(dialog);
		}
	}
	return -1;
 }

 int CloseConfigFile(void)
 {
 	SaveConfigFile();
	g_key_file_free(program_config);
	program_config = NULL;
	return 0;
 }

 void SaveWindowSize(const gchar *group, GtkWidget *widget)
 {
 	int 			pos[2];
	gtk_window_get_size(GTK_WINDOW(widget),&pos[0],&pos[1]);
	g_key_file_set_integer_list(program_config,group,"size",pos,2);
 }

 void action_Restore(void)
 {
    int f;

	if(!program_config)
		return;

 	/* Set window size */
	RestoreWindowSize("TopWindow",topwindow);

	for(f=0;f<(sizeof(WindowState)/sizeof(struct _WindowState));f++)
	{
		if(g_key_file_get_boolean(program_config,"TopWindow",WindowState[f].name,NULL))
		{
		    WindowState[f].activate(GTK_WINDOW(topwindow));
		}
	}
 }


 void action_Save(void)
 {
	GdkWindowState	CurrentState;
	int				f;

	if(!program_config)
		return;

	CurrentState = gdk_window_get_state(topwindow->window);

	if( !(CurrentState & (GDK_WINDOW_STATE_FULLSCREEN|GDK_WINDOW_STATE_MAXIMIZED|GDK_WINDOW_STATE_ICONIFIED)) )
		SaveWindowSize("TopWindow",topwindow);

	for(f=0;f<(sizeof(WindowState)/sizeof(struct _WindowState));f++)
		g_key_file_set_boolean(program_config,"TopWindow",WindowState[f].name, CurrentState & WindowState[f].flag);

	SaveConfigFile();
 }

/**
 * Get and string from configuration file.
 *
 * @param group	Configuration file section
 * @param key		Keyname
 * @param def		Default value
 *
 * @return String with the value read (release with g_free)
 *
 */
 gchar * GetString(const gchar *group, const gchar *key, const gchar *def)
 {
 	gchar *ret = NULL;

 	if(program_config)
		ret = g_key_file_get_string(program_config,group,key,NULL);

	if(!ret)
		return g_strdup(def);

	return ret;
 }

 void SetString(const gchar *group, const gchar *key, const gchar *val)
 {
 	if(program_config)
 	{
 		if(val)
			g_key_file_set_string(program_config,group,key,val);
		else
			g_key_file_remove_key(program_config,group,key,NULL);
 	}
 }

 gint GetInt(const gchar *group, const gchar *key, gint def)
 {
 	if(!(program_config && g_key_file_has_key(program_config,group,key,NULL)))
		return def;
	return g_key_file_get_integer(program_config,group,key,NULL);
 }

 void SetInt(const gchar *group, const gchar *key, gint val)
 {
 	if(program_config)
		g_key_file_set_integer(program_config,group,key,val);
 }

 void RestoreWindowSize(const gchar *group, GtkWidget *widget)
 {
	if(g_key_file_has_key(program_config,group,"size",NULL))
	{
		gsize 	sz		= 2;
		gint	*vlr	=  g_key_file_get_integer_list(program_config,group,"size",&sz,NULL);
		if(vlr)
		{
			gtk_window_resize(GTK_WINDOW(widget),vlr[0],vlr[1]);
			g_free(vlr);
		}
	}
 }

GKeyFile *GetConf(void)
{
	if(!program_config)
		OpenConfigFile();
	return program_config;
}
