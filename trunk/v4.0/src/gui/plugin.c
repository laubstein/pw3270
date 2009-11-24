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
 * Este programa está nomeado como plugin.c e possui 538 linhas de código.
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
 #include <globals.h>

 #include "gui.h"
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
 static GSList *plugins		= NULL;
 gchar			*plugin_list	= NULL;
#endif

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

#ifdef HAVE_PLUGINS

 static void load_plugin(const gchar *filename)
 {
 	GModule *handle;

	Trace("Loading plugin in %s",filename);

	handle = g_module_open(filename,G_MODULE_BIND_LOCAL);

	Trace("Handle for %s: %p",filename,handle);

	if(handle)
	{
		plugins = g_slist_append(plugins,handle);
	}
	else
	{
		Trace("Can't load %s: %s",filename,g_module_error());
		Log("Can't load %s: %s",filename,g_module_error());
	}
 }

 static int scan_for_plugins(const gchar *path)
 {
 	GDir			*dir;
 	const gchar	*name;
 	gchar			*filename;

	Trace("Loading plugins in \"%s\"",path);

    dir = g_dir_open(path,0,NULL);
    if(!dir)
		return ENOENT;

	name = g_dir_read_name(dir);
	while(name)
	{
		filename = g_build_filename(path,name,NULL);
		if(g_str_has_suffix(filename,G_MODULE_SUFFIX))
			load_plugin(filename);
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

 	gchar	*path;
 	gchar	**list;
 	int		f;

	if(!g_module_supported())
		return EINVAL;

	if(plugin_list)
	{
		// Load only defined plugins
		list = g_strsplit(plugin_list,",",0);
		if(list)
		{
			for(f=0;list[f];f++)
				load_plugin(list[f]);
			g_strfreev(list);
		}
	}
	else
	{
		// Scan for all available plugins
#if defined( DEBUG )
		path = g_build_filename(".","plugins",NULL);
		scan_for_plugins(path);
		g_free(path);
#elif defined(_WIN32)
		path = g_build_filename(program_data,"plugins",NULL);
		scan_for_plugins(path);
		g_free(path);
#elif defined( LIBDIR )
		path = g_build_filename(LIBDIR,PACKAGE_NAME,"plugins",NULL);
		scan_for_plugins(path);
		g_free(path);
#else
		path = g_build_filename(program_data,"plugins",NULL);
		scan_for_plugins(path);
		g_free(path);
#endif
	}

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
	void (*ptr)(GtkUIManager *ui, const gchar *data) = NULL;
	if(g_module_symbol(handle, "AddPluginUI", (gpointer) &ptr))
		ptr(ui,program_data);
 }

 static void start_plugin(GModule *handle, struct call_parameter *arg)
 {
	void (*ptr)(GtkWidget *widget, const gchar *arg) = NULL;
	if(g_module_symbol(handle, arg->name, (gpointer) &ptr))
		ptr(topwindow,arg->arg);
 }

#endif

gboolean StartPlugins(const gchar *startup_script)
{
#ifdef HAVE_PLUGINS
 	struct call_parameter p = { "pw3270_plugin_startup", startup_script };

	Trace("Starting plugins with \"%s\"...",startup_script);

 	if(plugins)
 	 	g_slist_foreach(plugins,(GFunc) start_plugin,&p);

#endif
	return FALSE;
}

 void CallPlugins(const gchar *name, const gchar *arg)
 {
#ifdef HAVE_PLUGINS
 	struct call_parameter p = { name, arg };

 	if(plugins)
 	 	g_slist_foreach(plugins,(GFunc) call,&p);
#endif
 }

  void AddPluginUI(GtkUIManager *ui, const gchar *dunno)
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

 void RunExternalProgramWithText(const gchar *cmd, const gchar *str)
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
			Warning( N_( "Can't create temporary file:\n%s" ), error->message ? error->message : N_( "Unexpected error" ));
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
			Warning( N_( "Error spawning %s\n%s" ), argv[0], error->message ? error->message : N_( "Unexpected error" ));
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
 	RunExternalProgramWithText(cmd,screen);
 	g_free(screen);

 }

 static void ExecWithCopy(GtkAction *action, gpointer cmd)
 {
 	gchar *text = GetClipboard();
 	RunExternalProgramWithText(cmd,text);
 	g_free(text);
 }

 static void ExecWithSelection(GtkAction *action, gpointer cmd)
 {
 	gchar *screen = GetScreenContents(FALSE);
 	RunExternalProgramWithText(cmd,screen);
 	g_free(screen);
 }

 static void ExecPFKey(GtkAction *action, gpointer cmd)
 {
	if(!TOGGLED_KEEP_SELECTED)
		action_ClearSelection();
	action_PFKey(atoi(cmd));
 }

 static void ExecPAKey(GtkAction *action, gpointer cmd)
 {
	if(!TOGGLED_KEEP_SELECTED)
		action_ClearSelection();
	action_PAKey(atoi(cmd));
 }

#ifdef HAVE_PLUGINS

 static void loadaction(GModule *handle, struct custom_action_call *arg)
 {
	void (*ptr)(GtkUIManager *ui, GtkActionGroup **groups, guint n_actions, GKeyFile *conf) = NULL;

	Trace("Searching for custom actions in %p",handle);

	if(g_module_symbol(handle, "LoadCustomActions", (gpointer) &ptr))
		ptr(arg->ui,arg->groups,arg->n_groups,arg->conf);

 }

 struct external_action
 {
 	gchar 	name[0x0100];
 	void 	(*exec)(GtkAction *action, gpointer cmd);
 };

 static void findaction(GModule *handle, struct external_action *arg)
 {
 	if(arg->exec)
		return;

	if(!g_module_symbol(handle,arg->name,(gpointer) &arg->exec))
		arg->exec = NULL;

 }

#endif

 static int scan_for_actions(const gchar *path, GtkActionGroup **groups)
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
 		{ "PFKey",				ExecPFKey			},
 		{ "PAKey",				ExecPAKey			}
 	};

	GDir			*dir;
	const gchar	*name;

	/* Load custom action files */

	Trace("Loading actions in \"%s\"",path);

	dir = g_dir_open(path,0,NULL);
	if(!dir)
		return ENOENT;

	name = g_dir_read_name(dir);
	while(name)
	{
		if(g_str_has_suffix(name,"act"))
		{
			GKeyFile 	*conf;
			gchar		*filename = g_build_filename(path,name,NULL);

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
						parm[0] = g_strdup(group[f]);

					for(p=0;p<G_N_ELEMENTS(call) && !run;p++)
					{
						if(!strcmp(parm[3],call[p].name))
							run = call[p].run;
					}

#ifdef HAVE_PLUGINS
					if(!run && plugins)
					{
						// Search for plugin exported actions
						struct external_action arg;

						memset(&arg,0,sizeof(arg));
						g_snprintf(arg.name,0xFF,"pw3270Action_%s",parm[3]);

						g_slist_foreach(plugins,(GFunc) findaction, &arg);

						Trace("%s: %p",arg.name,arg.exec);

						run = arg.exec;

					}
#endif

					if(!run)
					{
						// Search for internal action
						run = (void (*)(GtkAction *action, gpointer cmd)) get_action_callback_by_name(parm[3]);
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
							gchar *ptr = g_strdup(parm[4]);
							g_object_set_data_full(G_OBJECT(action),"Arg",ptr,g_free);
							g_signal_connect(G_OBJECT(action),"activate", G_CALLBACK(run),ptr);

							if(parm[5])
								gtk_action_group_add_action_with_accel(groups[0],action,parm[5]);
							else
								gtk_action_group_add_action(groups[0],action);
						}
						else
						{
							WarningPopup( N_( "Can't create action \"%s\"(\"%s\",\"%s\",\"%s\") when loading %s" ),parm[3],parm[0],parm[1],parm[2],filename);
						}
					}

					for(p=0;p<G_N_ELEMENTS(name);p++)
						g_free(parm[p]);

				}
				g_strfreev(group);
			}

			g_key_file_free(conf);
			g_free(filename);
		}
		name = g_dir_read_name(dir);
	}

	g_dir_close(dir);

	return 0;
 }

 void LoadCustomActions(GtkUIManager *ui, GtkActionGroup **groups, guint n_actions, GKeyFile *conf)
 {
	gchar *path;

#ifdef HAVE_PLUGINS
	struct custom_action_call arg = { ui, groups, n_actions, conf };

 	if(plugins)
 	 	g_slist_foreach(plugins,(GFunc) loadaction,&arg);

#endif

	path = g_build_filename(program_data,"ui",NULL);

	scan_for_actions(path,groups);
	g_free(path);

 }


