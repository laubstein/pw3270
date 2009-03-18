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
 #include "g3270.h"
 #include <ctype.h>
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

 static GKeyFile	* g3270_config	= NULL;

 gchar				* g3270_config_filename = NULL;
 gchar				* program_data = NULL;


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static gchar * FindConfigFile(void)
 {
 	static const gchar 	*name = PROGRAM_NAME ".conf";
	const gchar * const	*list;
 	gchar					*filename;
 	const gchar			*fixed[] = { g_get_user_config_dir(), g_get_home_dir()  };
 	int						f;

	// Check for pre-defined name
	if(g3270_config_filename)
	{
		return g3270_config_filename;
	}
	else
	{
		// Search user config dir; if found set fixed filename to value found

		filename = g_build_filename( g_get_user_config_dir(),name,NULL );
		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		{
			g3270_config_filename = filename;
			return filename;
		}
		g_free(filename);

		filename = g_build_filename( g_get_user_config_dir(),PROGRAM_NAME,name,NULL );
		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		{
			g3270_config_filename = filename;
			return filename;
		}
		g_free(filename);

		filename = g_build_filename( g_get_user_config_dir(),PACKAGE_NAME,name,NULL );
		if(g_file_test(filename,G_FILE_TEST_IS_REGULAR))
		{
			g3270_config_filename = filename;
			return filename;
		}
		g_free(filename);

	}

	/*
	 * Can't find user's defined configuration file, search on system paths
	 */
	for(f=0; f < (sizeof(fixed)/sizeof(const gchar *)); f++)
	{
		CHECK_FILENAME(fixed[f],name);
		CHECK_FILENAME(fixed[f],PROGRAM_NAME,name);
		CHECK_FILENAME(fixed[f],PACKAGE_NAME,name);
	}

	// Search system config path
	list =  g_get_system_config_dirs();
 	for(f=0;list[f];f++)
 	{
		CHECK_FILENAME(list[f],PROGRAM_NAME,name);
		CHECK_FILENAME(list[f],PACKAGE_NAME,name);
 	}

#ifdef DATAROOTDIR
	// Search DATADIR
	CHECK_FILENAME(DATAROOTDIR,PROGRAM_NAME,name);
	CHECK_FILENAME(DATAROOTDIR,PACKAGE_NAME,name);
#endif

	// Check if the file is available in current directory
	if(g_file_test(name,G_FILE_TEST_IS_REGULAR))
		return g_strdup(name);

	return 0;
 }


 int OpenConfigFile(void)
 {
	int	 f;
	gchar *filename = FindConfigFile();

	Trace("Configuration file: %s",filename);

	if(g3270_config)
	{
		g_key_file_free(g3270_config);
		g3270_config = NULL;
	}

	g3270_config = g_key_file_new();

	Trace("Configuration file: %s - %p",filename,g3270_config);

	if(filename)
		g_key_file_load_from_file(g3270_config,filename,G_KEY_FILE_NONE,NULL);

	/* Load initial settings */
	Trace("Loading %d toggles",N_TOGGLES);

	for(f=0;f<N_TOGGLES;f++)
	{
 		const char *name = get_toggle_name(f);

 		Trace("Setting toggle(%d): %s",f,name);

		if(g_key_file_has_key(g3270_config,"Toggles",name,NULL))
			set_toggle(f,g_key_file_get_boolean(g3270_config,"Toggles",name,NULL));
	}

	Trace("%s loaded",filename);
 	return 0;
 }

 gboolean GetBoolean(const gchar *group, const gchar *key, const gboolean def)
 {
	if(!g_key_file_has_key(g3270_config,group,key,NULL))
		return def;
	return g_key_file_get_boolean(g3270_config,group,key,NULL);
 }

 void SetBoolean(const gchar *group, const gchar *key, const gboolean val)
 {
	g_key_file_set_boolean(g3270_config,group,key,val);
 }

 int SaveConfigFile(void)
 {
 	gchar	*ptr;
 	gchar	*filename;
 	int		f;

	for(f=0;f<N_TOGGLES;f++)
		g_key_file_set_boolean(g3270_config,"Toggles",get_toggle_name(f),Toggled(f));

 	ptr = g_key_file_to_data(g3270_config,NULL,NULL);

	if(ptr)
	{
		gchar *buffer;

		for(buffer = ptr;*ptr && isspace(*ptr);ptr++);

		if(g3270_config_filename)
		{
			// Save on fixed file
			g_file_set_contents(g3270_config_filename,ptr,-1,NULL);
		}
		else
		{
			// Save the buffer contents
			filename = g_build_filename( g_get_user_config_dir(), PACKAGE_NAME ".conf", NULL);
			Trace("Saving %s...",filename);
			g_file_set_contents(filename,ptr,-1,NULL);
			g_free(filename);
		}

		// Release buffer
		g_free(buffer);
	}
	return 0;
 }

 int CloseConfigFile(void)
 {
 	SaveConfigFile();
	g_key_file_free(g3270_config);
	g3270_config = NULL;
	return 0;
 }

 void SaveWindowSize(const gchar *group, GtkWidget *widget)
 {
 	int 			pos[2];
	gtk_window_get_size(GTK_WINDOW(widget),&pos[0],&pos[1]);
	g_key_file_set_integer_list(g3270_config,group,"size",pos,2);
 }

 void action_Restore(void)
 {
    int f;

	if(!g3270_config)
		return;

 	/* Set window size */
	RestoreWindowSize("TopWindow",topwindow);

	for(f=0;f<(sizeof(WindowState)/sizeof(struct _WindowState));f++)
	{
		if(g_key_file_get_boolean(g3270_config,"TopWindow",WindowState[f].name,NULL))
		{
		    WindowState[f].activate(GTK_WINDOW(topwindow));
		}
	}
 }


 void action_Save(void)
 {
	GdkWindowState	CurrentState;
	int				f;

	if(!g3270_config)
		return;

	CurrentState = gdk_window_get_state(topwindow->window);

	if( !(CurrentState & (GDK_WINDOW_STATE_FULLSCREEN|GDK_WINDOW_STATE_MAXIMIZED|GDK_WINDOW_STATE_ICONIFIED)) )
		SaveWindowSize("TopWindow",topwindow);

	for(f=0;f<(sizeof(WindowState)/sizeof(struct _WindowState));f++)
		g_key_file_set_boolean(g3270_config,"TopWindow",WindowState[f].name, CurrentState & WindowState[f].flag);

	SaveConfigFile();
 }

 gchar * GetString(const gchar *group, const gchar *key, const gchar *def)
 {
 	gchar *ret = NULL;

 	if(g3270_config)
		ret = g_key_file_get_string(g3270_config,group,key,NULL);

	if(!ret)
		return g_strdup(def);

	return ret;
 }

 void SetString(const gchar *group, const gchar *key, const gchar *val)
 {
 	if(g3270_config)
 	{
 		if(val)
			g_key_file_set_string(g3270_config,group,key,val);
		else
			g_key_file_remove_key(g3270_config,group,key,NULL);
 	}
 }

 gint GetInt(const gchar *group, const gchar *key, gint def)
 {
 	if(!(g3270_config && g_key_file_has_key(g3270_config,group,key,NULL)))
		return def;
	return g_key_file_get_integer(g3270_config,group,key,NULL);
 }

 void SetInt(const gchar *group, const gchar *key, gint val)
 {
 	if(g3270_config)
		g_key_file_set_integer(g3270_config,group,key,val);
 }

 void RestoreWindowSize(const gchar *group, GtkWidget *widget)
 {
	if(g_key_file_has_key(g3270_config,group,"size",NULL))
	{
		gsize 	sz		= 2;
		gint	*vlr	=  g_key_file_get_integer_list(g3270_config,group,"size",&sz,NULL);
		if(vlr)
		{
			gtk_window_resize(GTK_WINDOW(widget),vlr[0],vlr[1]);
			g_free(vlr);
		}
	}
 }

static int filetest(const gchar *filename)
{
	Trace("Searching for %s (%d)",filename,(int) g_file_test(filename,G_FILE_TEST_IS_REGULAR));
	return g_file_test(filename,G_FILE_TEST_IS_REGULAR);
}

gchar * FindSystemConfigFile(const gchar *name)
{
	const gchar * const	*list =  g_get_system_config_dirs();
 	gchar					*filename;
 	int						f;

	if(program_data)
	{
		filename = g_build_filename(program_data,name,NULL);
		if(filetest(filename))
			return filename;
		g_free(filename);
	}

#ifdef DATAROOTDIR
	filename = g_build_filename(DATAROOTDIR,PACKAGE_NAME,name,NULL);
	if(filetest(filename))
		return filename;
	g_free(filename);
#endif

	// Search for the file in gtk's system config path
 	for(f=0;list[f];f++)
 	{
		filename = g_build_filename(list[f],PACKAGE_NAME,name,NULL);
		if(filetest(filename))
			return filename;
		g_free(filename);
 	}

	// Check if the file is available in current directory
	if(filetest(name))
		return g_strdup(name);

#ifdef DEBUG
	filename = g_build_filename(G_DIR_SEPARATOR_S,"usr","share",PACKAGE_NAME,name,NULL);
	if(filetest(filename))
		return filename;
	g_free(filename);
#endif

	return 0;
}

GKeyFile *GetConf(void)
{
	if(!g3270_config)
		OpenConfigFile();
	return g3270_config;
}
