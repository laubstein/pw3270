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

/*
 struct custom_action_call
 {
	GtkUIManager	*ui;
	GtkActionGroup	**groups;
	guint			n_groups;
	GKeyFile 		*conf;
 } custom_action_call;
*/

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

#ifdef HAVE_PLUGINS
 static GSList *plugins		= NULL;
 gchar			*plugin_list	= NULL;
 gchar			*plugin_path	= NULL;
#endif

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

#ifdef HAVE_PLUGINS

 static void load_plugin(const gchar *filename)
 {
 	GModule 					*handle;

	Trace("Loading plugin in %s",filename);

	handle = g_module_open(filename,G_MODULE_BIND_LOCAL);

	Trace("Handle for %s: %p",filename,handle);

	if(handle)
	{
		PW3270_PLUGIN_VERSION_INFO	*info;

		if(g_module_symbol(handle, "pw3270_plugin_version_info", (gpointer) &info))
		{
			Trace("%s info: Revision: %d Version: %s Description: \"%s\"",filename,info->rev,info->vrs,info->descr);
			if(info->rev < PW3270_PLUGIN_REQUIRED_REVISION)
			{
				// Incompatible or outdated plugin
				GtkWidget *dialog;

				Log("Plugin \"%s\" can't be loaded: Revision: %d Version: %s Description: \"%s\"",filename,info->rev,info->vrs,info->descr);

				dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
													GTK_DIALOG_DESTROY_WITH_PARENT,
													GTK_MESSAGE_WARNING,
													GTK_BUTTONS_OK,
													_(  "The plugin \"%s\" is incompatible or outdated." ), info->descr);

				gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't load plugin" ) );
				gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), _( "It was designed for pw3270 version %s (Revision %d)\nand can't be loaded in current application level." ),info->vrs,info->rev);
				gtk_dialog_run(GTK_DIALOG (dialog));
				gtk_widget_destroy(dialog);

				g_module_close(handle);
				return;
			}
		}
		else
		{
			Log("Warning: No EXPORT_PW3270_PLUGIN_INFORMATION info on \"%s\"",filename);
		}

		plugins = g_slist_append(plugins,handle);
	}
	else
	{
		Trace("Can't load %s: %s",filename,g_module_error());
		Log("Can't load \"%s\": %s",filename,g_module_error());
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
			{
				if(g_file_test(list[f],G_FILE_TEST_EXISTS))
				{
					load_plugin(list[f]);
				}
				else
				{
					GtkWidget *dialog;

					dialog = gtk_message_dialog_new(	GTK_WINDOW(topwindow),
														GTK_DIALOG_DESTROY_WITH_PARENT,
														GTK_MESSAGE_WARNING,
														GTK_BUTTONS_OK,
														_(  "Can't load \"%s\"" ), list[f]);

					gtk_window_set_title(GTK_WINDOW(dialog), _( "Can't load plugin" ) );
					gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), _( "The required plugin file wasn't found" ));
					gtk_dialog_run(GTK_DIALOG (dialog));
					gtk_widget_destroy(dialog);

				}
			}
			g_strfreev(list);
		}
	}
	else
	{
		// Scan for all available plugins
		if(plugin_path)
		{
			scan_for_plugins(plugin_path);
		}
		else
		{
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
	void (*ptr)(GtkWidget *widget, const gchar *arg) = NULL;
	if(g_module_symbol(handle, arg->name, (gpointer) &ptr))
		ptr(topwindow,arg->arg);
 }

 static void start_plugin(GModule *handle, struct call_parameter *arg)
 {
	void (*ptr)(GtkWidget *widget, const gchar *arg) = NULL;

//	Trace("%s::%s: %d",g_module_name(handle),arg->name,g_module_symbol(handle, arg->name, (gpointer) &ptr));

	if(g_module_symbol(handle, arg->name, (gpointer) &ptr))
		ptr(topwindow,arg->arg);
 }

 static void stop_plugin(GModule *handle, gpointer dunno)
 {
	void (*ptr)(GtkWidget *widget) = NULL;
	if(g_module_symbol(handle, "pw3270_plugin_stop", (gpointer) &ptr))
		ptr(topwindow);
 }

#endif

gboolean StartPlugins(const gchar *startup_script)
{
#ifdef HAVE_PLUGINS
 	struct call_parameter p = { "pw3270_plugin_start", startup_script };

	Trace("Starting plugins with \"%s\"...",startup_script);

 	if(plugins)
 	 	g_slist_foreach(plugins,(GFunc) start_plugin,&p);

#endif
	return FALSE;
}

gboolean StopPlugins(void)
{
#ifdef HAVE_PLUGINS

 	if(plugins)
 	 	g_slist_foreach(plugins,(GFunc) stop_plugin,0);

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

/*
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
*/

/*
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
	Trace("%s(%d)",__FUNCTION__,atoi(cmd));
	action_PAKey(atoi(cmd));
 }
*/

 gboolean get_symbol_by_name(GModule *module, gpointer *pointer, const gchar *fmt, ...)
 {
 	gboolean ret = FALSE;
#ifdef HAVE_PLUGINS
	gchar 	*symbol_name;
	va_list	arg;

	va_start(arg, fmt);
	symbol_name = g_strdup_vprintf(fmt,arg);
	va_end(arg);

	Trace("Module: %p Symbol: %s",module,symbol_name);
	if(module)
	{
		ret = g_module_symbol(module,symbol_name,pointer);
	}
	else
	{
		GSList *el = plugins;
		while(el && !ret)
		{
			Trace("el->data: %p",el->data);
			if(el->data)
				ret = g_module_symbol((GModule *)el->data,symbol_name,pointer);
			el = el->next;
		}
	}

	Trace("Symbol(%s): %p Found: %s",symbol_name,*pointer,ret ? "yes" : "no");

	g_free(symbol_name);
#endif
	return ret;
 }

 GModule * get_plugin_by_name(const gchar *plugin_name)
 {
 	GModule *ret = NULL;
#ifdef HAVE_PLUGINS
	GSList *el = plugins;

	while(el && !ret)
	{
		GModule *module	= (GModule *) el->data;
		gchar	*name	= g_path_get_basename(g_module_name(module));
		gchar 	*ptr	= strchr(name,'.');
		if(ptr)
			*ptr = 0;

		if(!g_ascii_strcasecmp(plugin_name,name))
			ret = module;

		g_free(name);

		el = el->next;
	}
#endif
	return ret;
 }
