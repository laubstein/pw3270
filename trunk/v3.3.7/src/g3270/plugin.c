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

 #include <errno.h>
 #include <glib.h>

 #include <lib3270/config.h>
 #include <globals.h>

 #include "g3270.h"
 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>

 #include <lib3270/plugins.h>

/*---[ Structs ]------------------------------------------------------------------------------------------------*/

 struct call_parameter
 {
 	const gchar *name;
 	const gchar *arg;
 };

 struct custom_action_call
 {
	GtkUIManager	*ui;
	GtkActionGroup	**groups;
	guint			n_groups;
	GKeyFile 		*conf;
 } custom_action_call;

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

#ifdef HAVE_PLUGINS
 static GSList *plugins = NULL;
#endif

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

#ifdef HAVE_PLUGINS

 static int scan_for_plugins(const gchar *path)
 {
 	GDir			*dir;
 	const gchar	*name;
 	GModule			*handle;
 	gchar			*filename;

	Trace("Loading plugins in \"%s\"",path);

    dir = g_dir_open(path,0,NULL);
    if(!dir)
		return ENOENT;

	name = g_dir_read_name(dir);
	while(name)
	{
//		G_MODULE_SUFFIX
		filename = g_build_filename(path,name,NULL);

		Trace("Loading plugin in %s",filename);

		handle = g_module_open(filename,G_MODULE_BIND_LOCAL);

		Trace("Handle for %s: %p",filename,handle);

		if(handle)
		{
			plugins = g_slist_append(plugins,handle);
		}
		else
		{
			Log("Can't load %s",filename);
		}
		g_free(filename);
		name = g_dir_read_name(dir);
	}

	g_dir_close(dir);

    return 0;
 }

#endif

 int LoadPlugins(void)
 {
#ifdef HAVE_PLUGINS
 	gchar *path;

	if(!g_module_supported())
		return EINVAL;

#if defined( DEBUG )
	path = g_build_filename(".","plugins",NULL);
	scan_for_plugins(path);
	g_free(path);
#elif defined(_WIN32)
	path = g_build_filename(".","plugins",NULL);
	scan_for_plugins(path);
	g_free(path);
#elif defined( LIBDIR )
	path = g_build_filename(LIBDIR,PACKAGE_NAME,"plugins",NULL);
	scan_for_plugins(path);
	g_free(path);
#else
	path = g_build_filename(".","plugins",NULL);
	scan_for_plugins(path);
	g_free(path);
#endif


#endif

	return 0;
 }

 void unload(GModule *handle,gpointer arg)
 {
#ifdef HAVE_PLUGINS
 	g_module_close(handle);
#endif
 }

 int UnloadPlugins(void)
 {
#ifdef HAVE_PLUGINS
 	g_slist_foreach(plugins,(GFunc) unload,NULL);
 	g_slist_free(plugins);
 	plugins = NULL;
#endif
	return 0;
 }

#ifdef HAVE_PLUGINS
 static void call(GModule *handle, struct call_parameter *arg)
 {
	void (*ptr)(const gchar *arg) = NULL;
	if(g_module_symbol(handle, arg->name, (gpointer) &ptr))
		ptr(arg->arg);
 }

 static void addui(GModule *handle, GtkUIManager *ui)
 {
	void (*ptr)(GtkUIManager *ui) = NULL;
	if(g_module_symbol(handle, "AddPluginUI", (gpointer) &ptr))
		ptr(ui);
 }

#endif

 void CallPlugins(const gchar *name, const gchar *arg)
 {
#ifdef HAVE_PLUGINS
 	struct call_parameter p = { name, arg };

 	if(plugins)
 	 	g_slist_foreach(plugins,(GFunc) call,&p);
#endif
 }

 void AddPluginUI(GtkUIManager *ui)
 {
#ifdef HAVE_PLUGINS
 	if(plugins)
 	 	g_slist_foreach(plugins,(GFunc) addui,ui);
#endif
 }

 static void process_ended(GPid pid,gint status,gchar *tempfile)
 {
 	Trace("Process %d ended with status %d",(int) pid, status);
 	remove(tempfile);
 	g_free(tempfile);
 }

 static void RunCommand(const gchar *cmd, const gchar *str)
 {
	GError	*error		= NULL;
	gchar	*filename	= NULL;
	GPid 	pid			= 0;
	gchar	*argv[3];
	gchar	tmpname[20];

	Trace("Running comand %s\n%s",cmd,str);

	do
	{
		g_free(filename);
		g_snprintf(tmpname,19,"%08lx.tmp",rand() ^ ((unsigned long) time(0)));
		filename = g_build_filename(g_get_tmp_dir(),tmpname,NULL);
	} while(g_file_test(filename,G_FILE_TEST_EXISTS));

	Trace("Temporary file: %s",filename);

	if(!g_file_set_contents(filename,str,-1,&error))
	{
		if(error)
		{
			PopupAnError( N_( "Error creating temporary file:\n%s" ), error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
		remove(filename);
		g_free(filename);
		return;
	}

	argv[0] = (gchar *) cmd;
	argv[1] = filename;
	argv[2] = NULL;

	Trace("Spawning %s %s",cmd,filename);

	error = NULL;

	if(!g_spawn_async(	NULL,											// const gchar *working_directory,
						argv,											// gchar **argv,
						NULL,											// gchar **envp,
						G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD,	// GSpawnFlags flags,
						NULL,											// GSpawnChildSetupFunc child_setup,
						NULL,											// gpointer user_data,
						&pid,											// GPid *child_pid,
						&error ))										// GError **error);
	{
		if(error)
		{
			PopupAnError( N_( "Error spawning %s\n%s" ), argv[0], error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
		remove(filename);
		g_free(filename);
		return;
	}

	Trace("pid %d",(int) pid);

	g_child_watch_add(pid,(GChildWatchFunc) process_ended,filename);
 }

 static void ExecWithScreen(GtkAction *action, gpointer cmd)
 {
 	gchar *screen = GetScreenContents(TRUE);
 	RunCommand(cmd,screen);
 	g_free(screen);

 }

 static void ExecWithCopy(GtkAction *action, gpointer cmd)
 {
 	Trace("%s Command to execute: %s",__FUNCTION__,(gchar *) cmd);
 }

 static void ExecWithSelection(GtkAction *action, gpointer cmd)
 {
 	gchar *screen = GetScreenContents(FALSE);
 	RunCommand(cmd,screen);
 	g_free(screen);
 }

 static void PFKey(GtkAction *action, gpointer cmd)
 {
	action_internal(PF_action, IA_DEFAULT, cmd, CN);
 }

#ifdef HAVE_PLUGINS
 static void loadaction(GModule *handle, struct custom_action_call *arg)
 {
	void (*ptr)(GtkUIManager *ui, GtkActionGroup **groups, guint n_actions, GKeyFile *conf) = NULL;

	Trace("Searching for custom actions in %p",handle);

	if(g_module_symbol(handle, "LoadCustomActions", (gpointer) &ptr))
		ptr(arg->ui,arg->groups,arg->n_groups,arg->conf);
 }
#endif

 void LoadCustomActions(GtkUIManager *ui, GtkActionGroup **groups, guint n_actions, GKeyFile *conf)
 {
 	static const struct _call
 	{
 		const gchar *name;
		void 	(*run)(GtkAction *action, gpointer cmd);
 	} call[] =
 	{
 		{ "ExecWithScreen", 	ExecWithScreen		},
 		{ "ExecWithCopy",		ExecWithCopy		},
 		{ "ExecWithSelection",	ExecWithSelection	},
 		{ "PFKey",				PFKey				}
 	};

	gchar 		*filename;

#ifdef HAVE_PLUGINS
	struct custom_action_call arg = { ui, groups, n_actions, conf };

 	if(plugins)
 	 	g_slist_foreach(plugins,(GFunc) loadaction,&arg);

#endif

	filename = FindSystemConfigFile("actions.conf");

	Trace("Actions.conf: %p",filename);

	if(!filename)
		return;

	Trace("Loading %s",filename);

	// Load custom actions
	conf = g_key_file_new();

	if(g_key_file_load_from_file(conf,filename,G_KEY_FILE_NONE,NULL))
	{
		int f;
		gchar **group = g_key_file_get_groups(conf,NULL);

		for(f=0;group[f];f++)
		{
			static const gchar *name[] = {	"label", 		// 0
												"tooltip",		// 1
												"stock_id",		// 2
												"action",		// 3
												"value",		// 4
												"accelerator" 	// 5
											};

			int			p;
			GtkAction 	*action;
			gchar		*parm[G_N_ELEMENTS(name)];
			void 		(*run)(GtkAction *action, gpointer cmd)	= NULL;

			Trace("Custom action(%d): %s",f,group[f]);

			for(p=0;p<G_N_ELEMENTS(name);p++)
			{
				parm[p] = g_key_file_get_locale_string(conf,group[f],name[p],NULL,NULL);
				if(!parm[p])
					parm[p] = g_key_file_get_string(conf,group[f],name[p],NULL);
			}

			if(!parm[0])
				parm[0] = group[f];

			for(p=0;p<G_N_ELEMENTS(call) && !run;p++)
			{
				if(!strcmp(parm[3],call[p].name))
					run = call[p].run;
			}

			if(!run)
			{
				WarningPopup( N_( "Invalid action \"%s\" when loading %s" ),parm[3],filename);
				Log("Invalid action %s in %s",parm[3],filename);
			}
			else
			{
				action = gtk_action_new(group[f],parm[0],parm[1],parm[2]);

				Trace("gtk_action_new(%s,%s,%s,%s): %p",group[f],parm[0],parm[1],parm[2],action);

				if(action)
				{
					// FIXME (perry#1#): Add a closure function to g_free the allocated string.
					g_signal_connect(G_OBJECT(action),"activate", G_CALLBACK(run),g_strdup(parm[4]));

					if(parm[5])
						gtk_action_group_add_action_with_accel(groups[0],action,parm[5]);
					else
						gtk_action_group_add_action(groups[0],action);
				}
			}
		}
		g_strfreev(group);
	}

	g_key_file_free(conf);
	g_free(filename);

 }
