/*
* Copyright 2008, Banco do Brasil S.A.
*
* This file is part of g3270
*
* This program file is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; version 3 of the License.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program in a file named COPYING; if not, write to the
* Free Software Foundation, Inc.,
* 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301 USA
*
* Authors:
*
* Perry Werneck<perry.werneck@gmail.com>
*
*/

 #include "config.h"
 #include "g3270.h"
 #include <ctype.h>

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

 static GKeyFile	*conf	= NULL;
 static gboolean	changed	= FALSE;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static gchar * FindConfigFile(void)
 {
 	static const gchar 	*name = PACKAGE_NAME ".conf";
	const gchar * const	*list;
 	gchar					*filename;
 	const gchar			*fixed[] = { g_get_user_config_dir(), g_get_home_dir() };
 	int						f;

	// Search the fixed paths
	for(f=0; f < (sizeof(fixed)/sizeof(const gchar *)); f++)
	{
		CHECK_FILENAME(fixed[f],name);
		CHECK_FILENAME(fixed[f],PACKAGE_NAME,name);
	}

	// Search system config path
	list =  g_get_system_config_dirs();
 	for(f=0;list[f];f++)
 	{
		CHECK_FILENAME(list[f],PACKAGE_NAME,name);
 	}

	// Check if the file is available in current directory
	if(g_file_test(name,G_FILE_TEST_IS_REGULAR))
		return g_strdup(name);

	return 0;
 }


 int OpenConfigFile(void)
 {
	gchar *filename = FindConfigFile();

	if(conf)
	{
		g_key_file_free(conf);
		conf = NULL;
	}

	conf = g_key_file_new();

	if(filename)
	{
		Log("Loading %s",filename);
		g_key_file_load_from_file(conf,filename,G_KEY_FILE_NONE,NULL);
	}


 	return 0;
 }

 int SaveConfigFile(void)
 {
 	gchar *ptr;
 	gchar *filename;

 	if(!changed)
		return 0;

	changed = FALSE;

 	ptr = g_key_file_to_data(conf,NULL,NULL);

	if(ptr)
	{
		gchar *buffer;

		for(buffer = ptr;*ptr && isspace(*ptr);ptr++);

		// Save the buffer contents
		filename = g_build_filename( g_get_user_config_dir(), PACKAGE_NAME ".conf", NULL);
		Trace("Saving %s...",filename);
		g_file_set_contents(filename,ptr,-1,NULL);
		g_free(filename);

		// Release buffer
		g_free(buffer);
	}
	return 0;
 }

 int CloseConfigFile(void)
 {
 	SaveConfigFile();
	g_key_file_free(conf);
	conf = NULL;
	return 0;
 }

 void action_Save(void)
 {
 	int 			pos[2];
	GdkWindowState	CurrentState;
	int				f;

	Trace("%s conf: %p",__FUNCTION__,conf);

	if(!conf)
		return;

	CurrentState = gdk_window_get_state(topwindow->window);

	if( !(CurrentState & (GDK_WINDOW_STATE_FULLSCREEN|GDK_WINDOW_STATE_MAXIMIZED|GDK_WINDOW_STATE_ICONIFIED)) )
	{
		// Save top window's size
		gtk_window_get_size(GTK_WINDOW(topwindow),&pos[0],&pos[1]);
		Trace("Topwindow size: %dx%d",pos[0],pos[1]);
		g_key_file_set_integer_list(conf,"TopWindow","size",pos,2);
	}

	for(f=0;f<(sizeof(WindowState)/sizeof(struct _WindowState));f++)
		g_key_file_set_boolean(conf,"TopWindow",WindowState[f].name, CurrentState & WindowState[f].flag);

	changed = TRUE;
	SaveConfigFile();
 }

 gchar * GetString(const gchar *group, const gchar *key, const gchar *def)
 {
 	gchar *ret = NULL;

 	if(conf)
		ret = g_key_file_get_string(conf,group,key,NULL);

	if(!ret)
		return g_strdup(def);

	return ret;
 }

 void action_Restore(void)
 {
	Trace("%s conf: %p",__FUNCTION__,conf);

	if(!conf)
		return;

 	/* Set window size */
	if(g_key_file_has_key(conf,"TopWindow","size",NULL))
	{
		gsize 	sz		= 2;
		gint	*vlr	=  g_key_file_get_integer_list(conf,"TopWindow","size",&sz,NULL);
		if(vlr)
		{
			Trace("Window size: %dx%d (sz=%d)",vlr[0],vlr[1],sz);
			gtk_window_resize(GTK_WINDOW(topwindow),vlr[0],vlr[1]);
			g_free(vlr);
		}
	}



 }

