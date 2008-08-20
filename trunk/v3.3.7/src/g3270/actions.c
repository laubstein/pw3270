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

 #include <gdk/gdk.h>
 #include <gdk/gdkkeysyms.h>

 #include "config.h"
 #include <globals.h>

 #include "g3270.h"
 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>
 #include <lib3270/toggle.h>
 #include <lib3270/hostc.h>

 #ifndef GDK_NUMLOCK_MASK
	#define GDK_NUMLOCK_MASK GDK_MOD2_MASK
 #endif

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

 static void action_Clear(GtkWidget *w, gpointer user_data);
 static void action_Up(GtkWidget *w, gpointer user_data);
 static void action_Down(GtkWidget *w, gpointer user_data);
 static void action_Left(GtkWidget *w, gpointer user_data);
 static void action_Right(GtkWidget *w, gpointer user_data);
 static void action_Tab(GtkWidget *w, gpointer user_data);
 static void action_BackTab(GtkWidget *w, gpointer user_data);
 static void action_Connect(GtkWidget *w, gpointer user_data);
 static void action_Enter(GtkWidget *w, gpointer user_data);
 static void action_Disconnect(GtkWidget *w, gpointer user_data);
 static void action_PrintScreen(GtkWidget *w, gpointer user_data);
 static void action_PrintSelected(GtkWidget *w, gpointer user_data);
 static void action_PrintClipboard(GtkWidget *w, gpointer user_data);
 static void action_Quit(void);
 static void action_About(GtkWidget *w, gpointer user_data);

 static void action_SaveScreen(void);
 static void action_SaveSelected(void);
 static void action_SaveClipboard(void);
 static void action_DumpScreen(void);
 static void action_LoadScreenDump(void);

/*---[ Callback tables ]----------------------------------------------------------------------------------------*/

 #ifdef DEBUG
	#define G3270_ACTION(key,state,action) { key, state, #key " (" #state ")", (void (*)(GtkWidget *, gpointer)) action, 0, #action }
 #else
	#define G3270_ACTION(key,state,action) { key, state, (void (*)(GtkWidget *, gpointer)) action, 0 }
 #endif

 struct WindowActions
 {
	guint	keyval;
	guint	state;

#ifdef DEBUG
	const char	*trace;
#endif


	void (*callback)(GtkWidget *w, gpointer data);
	const gpointer user_data;

#ifdef DEBUG
	const char	 *action_trace;
#endif
 };

/*---[ Action tables ]------------------------------------------------------------------------------------------*/

/*
	The name of the action.
	The stock id for the action, or the name of an icon from the icon theme.
	The label for the action. This field should typically be marked for translation, see gtk_action_group_set_translation_domain(). If label is NULL, the label of the stock item with id stock_id is used.
	The accelerator for the action, in the format understood by gtk_accelerator_parse().
	The tooltip for the action. This field should typically be marked for translation, see gtk_action_group_set_translation_domain().
	The function to call when the action is activated.

	http://library.gnome.org/devel/gtk/stable/gtk-Stock-Items.html

 */
 static const GtkActionEntry internal_action_entries[] =
 {
 	/* Top menus */
 	{	"FileMenu",			NULL,					N_( "_File" ),				NULL,				NULL,	NULL								},
 	{	"NetworkMenu",		NULL,					N_( "_Network" ),			NULL,				NULL,	NULL								},
 	{	"HelpMenu",			NULL,					N_( "Help" ),				NULL,				NULL,	NULL								},
 	{	"EditMenu",			NULL,					N_( "_Edit" ),				NULL,				NULL,	NULL								},
 	{	"OptionsMenu",		NULL,					N_( "_Options" ),			NULL,				NULL,	NULL								},
 	{	"SettingsMenu",		NULL,					N_( "Settings" ),			NULL,				NULL,	NULL								},
	{	"FontSettings",		GTK_STOCK_SELECT_FONT,	N_( "Select font" ),		NULL,				NULL,	NULL								},

 	/* Stock menus */
 	{	"Preferences",		GTK_STOCK_PREFERENCES,	N_( "Preferences" ),		NULL,				NULL,	NULL								},
 	{	"Network",			GTK_STOCK_NETWORK,		N_( "Network" ),			NULL,				NULL,	NULL								},
 	{	"Properties",		GTK_STOCK_PROPERTIES,	N_( "Properties" ),			NULL,				NULL,	NULL								},

	/* Misc actions */
 	{	"About",			GTK_STOCK_ABOUT,		N_( "About" ),				NULL,				NULL,	G_CALLBACK(action_About)			},
 	{	"Connect",			GTK_STOCK_CONNECT,		N_( "_Connect" ),			NULL,				NULL,	G_CALLBACK(action_Connect)			},
 	{	"Disconnect",		GTK_STOCK_DISCONNECT,	N_( "_Disconnect" ),		NULL,				NULL,	G_CALLBACK(action_Disconnect)		},
 	{	"Quit",				GTK_STOCK_QUIT,			N_( "_Quit" ),				NULL,				NULL,	G_CALLBACK(action_Quit)				},
 	{	"SelectColors",		GTK_STOCK_SELECT_COLOR,	N_( "Colors" ),				NULL,				NULL,	G_CALLBACK(action_SelectColors)		},
	{	"DumpScreen",		NULL,					N_( "Dump screen" ),		"<Alt>D",			NULL,	G_CALLBACK(action_DumpScreen)		},
	{	"LoadScreenDump",	NULL,					N_( "Load screen dump" ),	"<Alt>R",			NULL,	G_CALLBACK(action_LoadScreenDump)	},
	{ 	"SetHostname",		GTK_STOCK_HOME,			N_( "Set hostname" ),		NULL,				NULL,	G_CALLBACK(action_SetHostname)		},

 	/* Edit actions */
 	{	"Copy",				GTK_STOCK_COPY,			N_( "Copy" ),				NULL,				NULL,	G_CALLBACK(action_Copy)				},
 	{	"Append",			GTK_STOCK_ADD,			N_( "Add to copy" ),		"<Shift><Ctrl>c",	NULL,	G_CALLBACK(action_Append)			},
 	{	"Paste",			GTK_STOCK_PASTE,		N_( "Paste" ),				NULL,				NULL,	G_CALLBACK(action_Paste)			},
 	{	"PasteNext",		NULL,					N_( "Paste next" ),			"<Shift><Ctrl>v",	NULL,	G_CALLBACK(action_PasteNext)		},
 	{	"Unselect",			NULL,					N_( "Unselect" ),			"<Ctrl>u",			NULL,	G_CALLBACK(action_ClearSelection)	},
 	{	"Reselect",			NULL,					N_( "Reselect" ),			"<Ctrl>r",			NULL,	G_CALLBACK(Reselect)				},
 	{	"SelectAll",		GTK_STOCK_SELECT_ALL,	N_( "Select all" ),			"<Ctrl>a",			NULL,	G_CALLBACK(action_SelectAll)		},
 	{	"Clear",			GTK_STOCK_CLEAR,		N_( "Clear fields" ),		"Clear",			NULL,	G_CALLBACK(action_Clear)			},

 	/* Printer actions */
	{	"PrintScreen",		GTK_STOCK_PRINT,		N_( "Print" ),				"Print",			NULL,	G_CALLBACK(action_PrintScreen)		},
	{	"PrintSelected",	NULL,					N_( "Print selected" ),		NULL,				NULL,	G_CALLBACK(action_PrintSelected)	},
	{	"PrintClipboard",	NULL,					N_( "Print copy" ),			NULL,				NULL,	G_CALLBACK(action_PrintClipboard)	},

 	/* Save actions */
 	{	"Save",				GTK_STOCK_SAVE,			N_( "Save" ),				NULL,				NULL,	NULL								},
	{	"SaveScreen",		NULL,					N_( "Save screen" ),		NULL,				NULL,	G_CALLBACK(action_SaveScreen)		},
	{	"SaveSelected",		NULL,					N_( "Save selected" ),		NULL,				NULL,	G_CALLBACK(action_SaveSelected)		},
	{	"SaveClipboard",	NULL,					N_( "Save copy" ),			NULL,				NULL,	G_CALLBACK(action_SaveClipboard)	},

	/* Select actions */
	{	"SelectField",		NULL,					N_( "Select Field" ),		"<Ctrl>f",			NULL,	G_CALLBACK(action_SelectField)		},

	{ 	"SelectRight",		NULL,					N_( "Select Right" ),		"<Shift>Right",		NULL,	G_CALLBACK(action_SelectRight)		},
	{ 	"SelectLeft",		NULL,					N_( "Select Left" ),		"<Shift>Left",		NULL,	G_CALLBACK(action_SelectLeft)		},
	{ 	"SelectUp",			NULL,					N_( "Select Up" ),			"<Shift>Up",		NULL,	G_CALLBACK(action_SelectUp)			},
	{ 	"SelectDown",		NULL,					N_( "Select Down" ),		"<Shift>Down",		NULL,	G_CALLBACK(action_SelectDown)		},

	{	"SelectionRight",	NULL,					N_( "Selection Right" ),	"<Alt>Right",		NULL,	G_CALLBACK(action_SelectionRight)	},
	{	"SelectionLeft",	NULL,					N_( "Selection Left" ),		"<Alt>Left",		NULL,	G_CALLBACK(action_SelectionLeft)	},
	{	"SelectionUp",		NULL,					N_( "Selection Up" ),		"<Alt>Up",			NULL,	G_CALLBACK(action_SelectionUp)		},
	{	"SelectionDown",	NULL,					N_( "Selection Down" ),		"<Alt>Down",		NULL,	G_CALLBACK(action_SelectionDown)	},

	/* Cursor Movement */
	{ 	"CursorRight",		GTK_STOCK_GO_FORWARD,	N_( "Right" ),				"Right",			NULL,	G_CALLBACK(action_Right)			},
	{ 	"CursorLeft",		GTK_STOCK_GO_BACK,		N_( "Left" ),				"Left",				NULL,	G_CALLBACK(action_Left)				},
	{ 	"CursorUp",			GTK_STOCK_GO_UP,		N_( "Up" ),					"Up",				NULL,	G_CALLBACK(action_Up)				},
	{ 	"CursorDown",		GTK_STOCK_GO_DOWN,		N_( "Down" ),				"Down",				NULL,	G_CALLBACK(action_Down)				},
	{ 	"Enter",			NULL,					N_( "Enter" ),				"Return",			NULL,	G_CALLBACK(action_Enter)			},

	/* Terminal Actions */
	{ 	"Return",			GTK_STOCK_APPLY,		N_( "Return" ),				"Return",			NULL,	G_CALLBACK(action_Enter)			},
	{	"Redraw",			NULL,					N_( "Redraw screen" ),		NULL,				NULL,	G_CALLBACK(action_Redraw)			},
 };

 GtkActionGroup	*main_actions = NULL;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static void clear_and_call(GtkAction *action, XtActionProc call)
 {
 	action_ClearSelection();
 	action_internal(call, IA_DEFAULT, CN, CN);
 }

 void action_Clear(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(NULL,EraseInput_action);
 }

 void action_Up(GtkWidget *w, gpointer user_data)
 {
  	clear_and_call(NULL,Up_action);
 }

 void action_Down(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(NULL,Down_action);
 }

 void action_Left(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(NULL,Left_action);
 }

 void action_Right(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(NULL,Right_action);
 }

 void action_Tab(GtkWidget *w, gpointer user_data)
 {
 	action_internal(Tab_action, IA_DEFAULT, CN, CN);
 }

 void action_BackTab(GtkWidget *w, gpointer user_data)
 {
 	action_internal(BackTab_action, IA_DEFAULT, CN, CN);
 }

 void DisableNetworkActions(void)
 {
 	static const gchar	*name[] = { "Disconnect", "Connect" };
 	int						f;
	GtkAction 				*action;

 	for(f=0;f < G_N_ELEMENTS(actions);f++)
 	{
		action = gtk_action_group_get_action(main_actions,name[f]);
		if(action)
			gtk_action_set_sensitive(action,FALSE);
 	}
 }

 static void action_Disconnect(GtkWidget *w, gpointer user_data)
 {
 	Trace("%s Connected:%d Widget: %p",__FUNCTION__,PCONNECTED,w);

 	if(!PCONNECTED)
 		return;

	DisableNetworkActions();
 	action_ClearSelection();
 	action_internal(Disconnect_action, IA_DEFAULT, CN, CN);
 }


 static void action_Connect(GtkWidget *w, gpointer user_data)
 {
 	Trace("%s Connected:%d Widget: %p",__FUNCTION__,PCONNECTED,w);

 	if(PCONNECTED)
 		return;

	// TODO (perry#5#): If there's no previous server ask for it.

	DisableNetworkActions();
 	action_ClearSelection();
	action_internal(Reconnect_action, IA_DEFAULT, CN, CN);

 }

 void action_Enter(GtkWidget *w, gpointer user_data)
 {
 	action_ClearSelection();
 	if(PCONNECTED)
		action_internal(Enter_action, IA_DEFAULT, CN, CN);
	else
		action_Connect(w,user_data);
 }

 static void action_PrintScreen(GtkWidget *w, gpointer user_data)
 {
	PrintText("g3270", GetScreenContents(TRUE));
 }

 static void action_PrintSelected(GtkWidget *w, gpointer user_data)
 {
	PrintText("g3270", GetSelection());
 }

 static void action_PrintClipboard(GtkWidget *w, gpointer user_data)
 {
	PrintText("g3270", GetClipboard());
 }

 static void action_Quit(void)
 {
 	action_Save();
 	gtk_main_quit();
 }

 static void action_About(GtkWidget *w, gpointer user_data)
 {
 	static const char *authors[] = {	"Paul Mattes <Paul.Mattes@usa.net>",
										"GTRC",
										"Perry Werneck <perry.werneck@gmail.com>",
										"and others",
										NULL};

	static const char license[] =
	N_( "This program is free software; you can redistribute it and/or "
		"modify it under the terms of the GNU General Public License as "
 		"published by the Free Software Foundation; either version 2 of the "
		"License, or (at your option) any later version.\n\n"
		"This program is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the "
		"GNU General Public License for more details.\n\n"
		"You should have received a copy of the GNU General Public License "
		"along with this program; if not, write to the Free Software "
		"Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02111-1307 "
		"USA" );

	GdkPixbuf	*logo = NULL;
	gchar		*file = FindSystemConfigFile(PACKAGE_NAME ".jpg");

	if(file)
	{
		Log("Loading %s",file);
		logo = gdk_pixbuf_new_from_file(file, NULL);
		g_free(file);
	}

 	gtk_show_about_dialog(	GTK_WINDOW(topwindow),
							"program-name",    		PACKAGE_NAME,
							"authors", 				authors,
							"license", 				gettext( license ),
							"comments",				_( "3270 Terminal emulator for GTK."),
							"version", 				PACKAGE_VERSION,
							"wrap-license",			TRUE,
							"logo",					logo,
							NULL
						);

	if(logo)
		gdk_pixbuf_unref(logo);
 }

 void process_ended(GPid pid,gint status,gchar *tempfile)
 {
 	Trace("Process %d ended with status %d",(int) pid, status);
 	remove(tempfile);
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
			popup_an_error( N_( "Error creating temporary file:\n%s" ), error->message ? error->message : N_( "Unexpected error" ));
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

	if(!g_spawn_async(	NULL,							// const gchar *working_directory,
						argv,							// gchar **argv,
						NULL,							// gchar **envp,
						G_SPAWN_SEARCH_PATH,			// GSpawnFlags flags,
						NULL,							// GSpawnChildSetupFunc child_setup,
						NULL,							// gpointer user_data,
						&pid,							// GPid *child_pid,
						&error ))						// GError **error);
	{
		if(error)
		{
			popup_an_error( N_( "Error spawning %s\n%s" ), argv[0], error->message ? error->message : N_( "Unexpected error" ));
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
 	Trace("%s Command to execute: %s",__FUNCTION__,cmd);
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

 static void LoadCustomActions(GtkActionGroup *actions)
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

	gchar 		*filename = FindSystemConfigFile("actions.conf");
	GKeyFile	*conf;

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
				Log("Invalid action type %s in %s",parm[3],filename);
			}
			else
			{
				action = gtk_action_new(group[f],parm[0],parm[1],parm[2]);

				if(action)
				{
					// FIXME (perry#1#): Add a closure function to g_free the allocated string.
					g_signal_connect(G_OBJECT(action),"activate", G_CALLBACK(run),g_strdup(parm[4]));

					if(parm[5])
						gtk_action_group_add_action_with_accel(actions,action,parm[5]);
					else
						gtk_action_group_add_action(actions,action);
				}
			}
		}
		g_strfreev(group);
	}

	g_key_file_free(conf);
	g_free(filename);
 }

 static void toggle_action(GtkToggleAction *action, int id)
 {
 	set_toggle(id,gtk_toggle_action_get_active(action));
 }

 static void toggle_set(GtkAction *action,int id)
 {
 	set_toggle(id,TRUE);
 }

 static void toggle_reset(GtkAction *action,int id)
 {
 	set_toggle(id,FALSE);
 }

 static void LoadToggleActions(GtkActionGroup *actions)
 {
	// TODO (perry#9#): Add tooltips
 	static const struct _toggle_list
 	{
 		gboolean		set;
 		int				toggle;
		const gchar	*label;
		const gchar	*tooltip;
		const gchar	*stock_id;
	} toggle_list[] =
	{
		{ TRUE,		FULL_SCREEN,	NULL,	NULL,	GTK_STOCK_FULLSCREEN		},
		{ FALSE,	FULL_SCREEN,	NULL,	NULL,	GTK_STOCK_LEAVE_FULLSCREEN	},
	};

	// TODO (perry#9#): Add tooltips
 	static const struct _toggle_info
 	{
 		const gchar *label;
 		const gchar *tooltip;
 		const gchar *stock_id;
 		const gchar *accelerator;
	}
	toggle_info[N_TOGGLES] =
	{
		{ N_( "Monocase" ),						NULL,	NULL,	NULL				},
		{ N_( "Alt Cursor" ),					NULL,	NULL,	NULL				},
		{ N_( "Blinking Cursor" ),				NULL,	NULL,	NULL				},
		{ N_( "Show timing" ),					NULL,	NULL,	NULL				},
		{ N_( "Track Cursor" ),					NULL,	NULL,	NULL				},
		{ N_( "DS Trace" ),						NULL,	NULL,	NULL				},
		{ N_( "Scroll bar" ),					NULL,	NULL,	NULL				},
		{ N_( "Line Wrap" ),					NULL,	NULL,	NULL				},
		{ N_( "Blank Fill" ),					NULL,	NULL,	NULL				},
		{ N_( "Screen Trace" ),					NULL,	NULL,	NULL				},
		{ N_( "Event Trace" ),					NULL,	NULL,	NULL				},
		{ N_( "Paste with left margin" ),		NULL,	NULL,	NULL				},
		{ N_( "Select by rectangles" ),			NULL,	NULL,	N_( "<alt>S" )		},
		{ N_( "Cross Hair Cursor" ),			NULL,	NULL,	N_( "<alt>X" )		},
		{ N_( "Visible Control chars" ),		NULL,	NULL,	NULL				},
		{ N_( "Aid wait" ),						NULL,	NULL,	NULL				},
		{ N_( "Full Screen" ),					NULL,	NULL,	N_( "<alt>Home" )	},
		{ N_( "Auto-Reconnect" ),				NULL,	NULL,	NULL				},
		{ N_( "Insert" ),						NULL,	NULL,	"Insert"			},
		{ N_( "Keypad" ),						NULL,	NULL,	NULL				}
	};

 	int f;

	/* Toggle actions */
 	for(f=0;f<N_TOGGLES;f++)
 	{
 		char buffer[20] = "Toggle";

		if(toggle_info[f].label)
		{
			strncat(buffer,get_toggle_name(f),20);

			GtkToggleAction *action = gtk_toggle_action_new(	buffer,
																gettext(toggle_info[f].label),
																gettext(toggle_info[f].tooltip),
																toggle_info[f].stock_id );
			gtk_toggle_action_set_active(action,Toggled(f));
			g_signal_connect(G_OBJECT(action),"toggled", G_CALLBACK(toggle_action),(gpointer) f);

			if(toggle_info[f].accelerator)
				gtk_action_group_add_action_with_accel(actions,(GtkAction *) action, gettext(toggle_info[f].accelerator));
			else
				gtk_action_group_add_action(actions,(GtkAction *) action);

		}
 	}

	/* Set/Reset actions */
 	for(f=0;f< G_N_ELEMENTS(toggle_list);f++)
 	{
 		int		id = toggle_list[f].toggle;
 		char	buffer[20];

 		strcpy(buffer,toggle_list[f].set ? "Set" : "Reset" );

		strncat(buffer,get_toggle_name(id),20);

		Trace("Creating action \"%s\"",buffer);

		GtkAction *action = gtk_action_new(	buffer,
											gettext(toggle_list[f].label),
											gettext(toggle_list[f].tooltip),
											toggle_list[f].stock_id );

		if(toggle_list[f].set)
		{
			g_signal_connect(G_OBJECT(action),"activate", G_CALLBACK(toggle_set),(gpointer) id);
			if(Toggled(id))
				gtk_action_set_visible(action,FALSE);
		}
		else
		{
			g_signal_connect(G_OBJECT(action),"activate", G_CALLBACK(toggle_reset),(gpointer) id);
			if(!Toggled(id))
				gtk_action_set_visible(action,FALSE);
		}

		gtk_action_group_add_action(actions,action);
 	}

 }

 static void call3270(GtkAction *action, XtActionProc call)
 {
 	action_internal(call, IA_DEFAULT, CN, CN);
 }

 static void Load3270Actions(GtkActionGroup *actions)
 {
 	struct call_3270
 	{
 		const gchar *name;
 		const gchar *label;
 		const gchar *tooltip;
 		const gchar *stock_id;
 		XtActionProc call;
 		const gchar *accelerator;
	};

	// TODO (perry#9#): Add tooltips
 	static const struct call_3270 action_clear[] =
	{
		{ "Reset",			N_( "Reset" ),
							NULL,
							NULL,
							Reset_action,
							N_( "<Ctrl>r" ) },

		{ "Escape",			N_( "Escape" ),
							NULL,
							NULL,
							Reset_action,
							"Escape" }
	};

 	static const struct call_3270 action_info[] =
	{
		{ "EraseEOF",		N_( "Erase EOF" ),
							N_( "Erase to the end of the field" ),
							NULL,
							EraseEOF_action,
							"End" },

		{ "Home",			N_( "Field start" ),
							NULL,
							NULL,
							Home_action,
							"Home" },

		{ "DeleteWord",		N_( "Delete Word" ),
							NULL,
							NULL,
							DeleteWord_action,
							N_( "<Ctrl>w" )	},

		{ "EraseField",		N_( "Erase Field" ),
							NULL,
							NULL,
							DeleteField_action,
							N_( "<Ctrl>u" ) },

		{ "Delete",			N_( "Delete Char" ),
							NULL,
							NULL,
							Delete_action,
							"Delete" },

		{ "Erase",			N_( "Backspace" ),
							NULL,
							NULL,
							Erase_action,
							"BackSpace" },

		{ "NextField",		N_( "Next field" ),
							NULL,
							NULL,
							Tab_action,
							"Tab" },

		{ "SysREQ",			N_( "Sys Req" ),
							NULL,
							NULL,
							SysReq_action,
							"Sys_Req" },

	};

	int f;

 	for(f=0;f<G_N_ELEMENTS(action_info);f++)
 	{
		GtkAction *action = gtk_action_new(	action_info[f].name,
											gettext(action_info[f].label),
											gettext(action_info[f].tooltip),
											action_info[f].stock_id );

		g_signal_connect(G_OBJECT(action),"activate", G_CALLBACK(call3270),(gpointer) action_info[f].call);

		if(action_info[f].accelerator)
			gtk_action_group_add_action_with_accel(actions,(GtkAction *) action, gettext(action_info[f].accelerator));
		else
			gtk_action_group_add_action(actions,(GtkAction *) action);

 	}

 	for(f=0;f<G_N_ELEMENTS(action_clear);f++)
 	{
		GtkAction *action = gtk_action_new(	action_clear[f].name,
											gettext(action_clear[f].label),
											gettext(action_clear[f].tooltip),
											action_clear[f].stock_id );

		g_signal_connect(G_OBJECT(action),"activate", G_CALLBACK(clear_and_call),(gpointer) action_clear[f].call);

		if(action_info[f].accelerator)
			gtk_action_group_add_action_with_accel(actions,(GtkAction *) action, gettext(action_clear[f].accelerator));
		else
			gtk_action_group_add_action(actions,(GtkAction *) action);

 	}
 }


 GtkUIManager * LoadApplicationUI(GtkWidget *widget)
 {
	GtkUIManager 	*ui_manager = gtk_ui_manager_new(); // http://library.gnome.org/devel/gtk/stable/GtkUIManager.html
	GError			*error = NULL;
	gchar			*ui;

	// Load actions
	main_actions = gtk_action_group_new("Actions");
	gtk_action_group_set_translation_domain(main_actions, GETTEXT_PACKAGE);

	gtk_action_group_add_actions(main_actions, internal_action_entries, G_N_ELEMENTS (internal_action_entries), topwindow);

	Load3270Actions(main_actions);
	LoadCustomActions(main_actions);
	LoadToggleActions(main_actions);

	// Add actions and load UI
	gtk_ui_manager_insert_action_group(ui_manager,main_actions, 0);

	ui = FindSystemConfigFile("ui.xml");
	if(ui)
	{
		Log("Loading interface from %s",ui);
		if(!gtk_ui_manager_add_ui_from_file(ui_manager,ui,&error))
		{
			if(error && error->message)
				popup_an_error( _( "Can't build Application UI: %s" ),error->message);
			else
				popup_an_error( _( "Can't build Application UI!" ));
		}
		g_free(ui);
	}
	else
	{
		popup_an_error( _( "Can't find UI definition file" ) );
	}

	/* Update UI */
	gtk_ui_manager_ensure_update(ui_manager);
	return ui_manager;
 }

 int GetFunctionKey(GdkEventKey *event)
 {
 	int rc = (event->keyval - GDK_F1)+1;

 	if(event->state & GDK_SHIFT_MASK)
 	   rc += 12;

	return rc;
 }

 gboolean KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
 	// TODO (perry#2#): Put all keyboard actions as accelerators.
 	static const struct WindowActions keyproc[] =
 	{
		G3270_ACTION(	GDK_Left,			0,					action_Left),
		G3270_ACTION(	GDK_Up,				0,					action_Up),
		G3270_ACTION(	GDK_Right,			0,					action_Right),
		G3270_ACTION(	GDK_Down,			0,					action_Down),

		G3270_ACTION(	GDK_KP_Left,		0,					action_Left),
		G3270_ACTION(	GDK_KP_Up,			0,					action_Up),
		G3270_ACTION(	GDK_KP_Right,		0,					action_Right),
		G3270_ACTION(	GDK_KP_Down,		0,					action_Down),

		G3270_ACTION(	GDK_ISO_Left_Tab,	0,					action_BackTab),
		G3270_ACTION(	GDK_Tab,			0,					action_Tab),
		G3270_ACTION(	GDK_KP_Add,			GDK_NUMLOCK_MASK,	action_Tab),

 	};

 	int		f;
 	char	buffer[10];

	/* Is function key? */
	if(IS_FUNCTION_KEY(event))
    {
		action_ClearSelection();
    	sprintf(buffer,"%d",GetFunctionKey(event));
        action_internal(PF_action, IA_DEFAULT, buffer, CN);
        return TRUE;
    }

    /* Check for special keyproc actions */
	for(f=0; f < (sizeof(keyproc)/sizeof(struct WindowActions));f++)
	{
		if(keyproc[f].keyval == event->keyval && (event->state & keyproc[f].state) == keyproc[f].state)
		{
			Trace("Key: %s\tAction: %s",keyproc[f].trace,keyproc[f].action_trace);
			keyproc[f].callback(widget,keyproc[f].user_data);
			return TRUE;
		}
	}


	return FALSE;
 }

 static int SaveText(const char *title, gchar *text)
 {

	GtkWidget *dialog = gtk_file_chooser_dialog_new( gettext(title),
                                                     GTK_WINDOW(topwindow),
                                                     GTK_FILE_CHOOSER_ACTION_SAVE,
                                                     GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
                                                     GTK_STOCK_SAVE,	GTK_RESPONSE_ACCEPT,
                                                     NULL );


	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GError	*error = NULL;
		gchar	*filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));

		if(!g_file_set_contents(filename,text,-1,&error))
		{
			popup_an_error( N_( "Error saving %s\n%s" ), filename, error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
	}

	gtk_widget_destroy(dialog);
 	g_free(text);
 	return 0;
 }

 static void action_LoadScreenDump(void)
 {
	GtkWidget *dialog = gtk_file_chooser_dialog_new( _( "Load screen dump" ),
                                                     GTK_WINDOW(topwindow),
													 GTK_FILE_CHOOSER_ACTION_OPEN,
                                                     GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
                                                     GTK_STOCK_OPEN,	GTK_RESPONSE_ACCEPT,
                                                     NULL );


	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GError		*error = NULL;
		gchar		*filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gsize		sz;
		struct ea	*buffer	= NULL;

		if(!g_file_get_contents(filename, (gchar **) &buffer, &sz, &error))
		{
			popup_an_error( N_( "Error loading %s\n%s" ), filename, error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
		else
		{
			sz /= sizeof(struct ea);
			if(set_device_buffer(buffer,sz))
				popup_an_error( N_( "Can't set device buffer contents" ) );
		}

		g_free(buffer);
	}

	gtk_widget_destroy(dialog);
 }

 static void action_DumpScreen(void)
 {
	GtkWidget *dialog = gtk_file_chooser_dialog_new( _( "Dump screen contents" ),
                                                     GTK_WINDOW(topwindow),
                                                     GTK_FILE_CHOOSER_ACTION_SAVE,
                                                     GTK_STOCK_CANCEL,	GTK_RESPONSE_CANCEL,
                                                     GTK_STOCK_SAVE,	GTK_RESPONSE_ACCEPT,
                                                     NULL );


	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT)
	{
		GError		*error = NULL;
		gchar		*filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		int			sz;
		struct ea	*buffer = copy_device_buffer(&sz);

		if(!g_file_set_contents(filename,(gchar *) buffer,sz*sizeof(struct ea),&error))
		{
			popup_an_error( N_( "Error saving %s\n%s" ), filename, error->message ? error->message : N_( "Unexpected error" ));
			g_error_free(error);
		}
		free(buffer);
	}

	gtk_widget_destroy(dialog);
 }

 static void action_SaveScreen(void)
 {
	SaveText(N_( "Save screen contents" ), GetScreenContents(TRUE));
 }

 static void action_SaveSelected(void)
 {
	SaveText(N_( "Save selected text" ), GetSelection());
 }

 static void action_SaveClipboard(void)
 {
	SaveText(N_( "Save clipboard contents" ), GetClipboard());
 }

 void SetHostname(const gchar *hostname)
 {
	SetString("Network","Hostname",hostname);
	CallPlugins("SetHostname",hostname);
 }

 void action_SetHostname(void)
 {
 	const char		*host 		= GetString("Network","Hostname","");
 	gboolean		again		= TRUE;
	char 			buffer[1024];
 	GtkTable		*table		= GTK_TABLE(gtk_table_new(2,2,FALSE));
 	GtkEntry		*entry		= GTK_ENTRY(gtk_entry_new());
 	GtkToggleButton	*checkbox	= GTK_TOGGLE_BUTTON(gtk_check_button_new_with_label( _( "Secure connection" ) ));
 	GtkWidget 		*dialog 	= gtk_dialog_new_with_buttons(	_( "Select hostname" ),
																GTK_WINDOW(topwindow),
																GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT,
																GTK_STOCK_CONNECT,	GTK_RESPONSE_ACCEPT,
																GTK_STOCK_CANCEL,	GTK_RESPONSE_REJECT,
																NULL	);

	gtk_window_set_icon_name(GTK_WINDOW(dialog),GTK_STOCK_HOME);
	gtk_entry_set_max_length(entry,1000);
	gtk_entry_set_width_chars(entry,60);

	gtk_table_attach(table,gtk_label_new( _( "Hostname:" ) ), 0,1,0,1,0,0,5,0);
	gtk_table_attach(table,GTK_WIDGET(entry), 1,2,0,1,GTK_EXPAND|GTK_FILL,0,0,0);
	gtk_table_attach(table,GTK_WIDGET(checkbox), 1,2,1,2,GTK_EXPAND|GTK_FILL,0,0,0);


	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox),GTK_WIDGET(table),FALSE,FALSE,2);

	if(strncmp(host,"L:",2))
	{
		gtk_entry_set_text(entry,host);
	}
	else
	{
		gtk_toggle_button_set_active(checkbox,TRUE);
		gtk_entry_set_text(entry,host+2);
	}

	gtk_widget_show_all(GTK_WIDGET(table));

 	while(again)
 	{
 		switch(gtk_dialog_run(GTK_DIALOG(dialog)))
 		{
		case GTK_RESPONSE_ACCEPT:
			if(gtk_toggle_button_get_active(checkbox))
				strcpy(buffer,"L:");
			else
				*buffer = 0;

			strncat(buffer,gtk_entry_get_text(entry),1023);

			if(host_connect(buffer) >= 0)
			{
				if(!wait4negotiations(buffer))
				{
					again = FALSE;
					SetHostname(buffer);
				}
			}
			break;

		case GTK_RESPONSE_REJECT:
			again = FALSE;
			break;
 		}
 	}

	gtk_widget_destroy(dialog);

 }
