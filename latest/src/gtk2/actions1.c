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
 * Este programa está nomeado como actions.c e possui 1088 linhas de código.
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
 #include <gdk/gdk.h>

 #include "gui.h"
 #include "fonts.h"
 #include "actions.h"
 #include "uiparser1.h"
 #include <gdk/gdkkeysyms.h>
 #include <errno.h>

// #include <globals.h>

// #include <lib3270/kybdc.h>
 #include <lib3270/toggle.h>
 #include <lib3270/plugins.h>

 #ifndef GDK_NUMLOCK_MASK
	#define GDK_NUMLOCK_MASK GDK_MOD2_MASK
 #endif

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

 static void action_Up(GtkWidget *w, gpointer user_data);
 static void action_Down(GtkWidget *w, gpointer user_data);
 static void action_Left(GtkWidget *w, gpointer user_data);
 static void action_Right(GtkWidget *w, gpointer user_data);
 static void action_Newline(GtkWidget *w, gpointer user_data);

 static void action_pfkey(GtkAction *action,gpointer id);
 static void action_pakey(GtkAction *action,gpointer id);


/*---[ Gui toggles ]--------------------------------------------------------------------------------------------*/

 const struct _gui_toggle_info gui_toggle_info[GUI_TOGGLE_COUNT] =
 {
    { "Bold", 			    FALSE	},
    { "KeepSelected", 	    FALSE	},
    { "Underline",		    TRUE	},
    { "AutoConnect",    	TRUE	},
    { "KPAlternative",		FALSE	}
 };

 gboolean gui_toggle_state[GUI_TOGGLE_COUNT] = { 0 };


/*---[ Action tables ]------------------------------------------------------------------------------------------*/

 GtkActionGroup			* action_group[ACTION_GROUP_MAX+1];
 GtkAction				* action_by_id[ACTION_ID_MAX] 		= { NULL };
 GtkAction 				* action_scroll[ACTION_SCROLL_MAX]	= { NULL, NULL, NULL, NULL };

 static struct _keyboard_action
 {
	guint			keyval;
	GdkModifierType	state;
	GtkAction		*action;
	GCallback		def;
 } keyboard_action[] =
 {
	{ GDK_Left,				0,					NULL,	G_CALLBACK(action_Left)				},
	{ GDK_Up,				0,					NULL,	G_CALLBACK(action_Up)				},
	{ GDK_Right,			0,					NULL,	G_CALLBACK(action_Right)			},
	{ GDK_Down,				0,					NULL,	G_CALLBACK(action_Down)				},
	{ GDK_Tab,				0,					NULL,	G_CALLBACK(lib3270_tab)				},
	{ GDK_ISO_Left_Tab,		GDK_SHIFT_MASK,		NULL,	G_CALLBACK(lib3270_backtab)			},
	{ GDK_KP_Left,			0,					NULL,	G_CALLBACK(action_Left)				},
	{ GDK_KP_Up,			0,					NULL,	G_CALLBACK(action_Up)				},
	{ GDK_KP_Right,			0,					NULL,	G_CALLBACK(action_Right)			},
	{ GDK_KP_Down,			0,					NULL,	G_CALLBACK(action_Down)				},

	{ GDK_KP_Add,			GDK_NUMLOCK_MASK,	NULL,	G_CALLBACK(action_kpadd)			},
	{ GDK_KP_Subtract,		GDK_NUMLOCK_MASK,	NULL,	G_CALLBACK(action_kpsubtract)		},

	{ GDK_3270_PrintScreen,	0,					NULL,	G_CALLBACK(action_printscreen)		},
	{ GDK_Sys_Req,			0,					NULL,	G_CALLBACK(lib3270_sysreq)			},

	{ GDK_Print,			GDK_CONTROL_MASK,	NULL,	G_CALLBACK(action_printscreen)		},
	{ GDK_Print,			GDK_SHIFT_MASK,		NULL,	G_CALLBACK(lib3270_sysreq)			},
	{ GDK_Control_R,		0,					NULL,	NULL								},
	{ GDK_Control_L,		0,					NULL,	NULL								},

#ifdef WIN32
	{ GDK_Pause,			0,					NULL,	NULL								},
#endif
 };

 static struct _pf_action
 {
	GtkAction 	*normal;
	GtkAction	*shift;
 } pf_action[12] =
 {
 	{ NULL,	NULL }, 	// PF01
 	{ NULL,	NULL }, 	// PF02
 	{ NULL,	NULL }, 	// PF03
 	{ NULL,	NULL }, 	// PF04
 	{ NULL,	NULL }, 	// PF05
 	{ NULL,	NULL }, 	// PF06
 	{ NULL,	NULL }, 	// PF07
 	{ NULL,	NULL }, 	// PF08
 	{ NULL,	NULL }, 	// PF09
 	{ NULL,	NULL }, 	// PF10
 	{ NULL,	NULL }, 	// PF11
 	{ NULL,	NULL }, 	// PF12
 };

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 GtkAction * get_action_by_name(const gchar *name)
 {
	int			p;
	GtkAction	*rc = NULL;

	for(p = 0; action_group[p] && !rc; p++)
		rc = gtk_action_group_get_action(action_group[p],name);

	return rc;
 }

 static void clear_and_call(GtkAction *action, int (*call)(void))
 {
 	unselect();
 	call();
 }

 LOCAL_EXTERN void gui_toogle_set_visible(enum GUI_TOGGLE id, gboolean visible)
 {
 	gchar		*name	= g_strconcat("Toggle",gui_toggle_info[id].name,NULL);
	GtkAction	*action = get_action_by_name(name);

	if(action)
		gtk_action_set_visible(action,visible);

	g_free(name);
 }


 static void action_Reset(void)
 {
 	clear_and_call(0,lib3270_reset);
 }

 static void clear_action(void)
 {
 	clear_and_call(0,lib3270_clear);
 }

 static void erase_input_action(void)
 {
 	clear_and_call(0,lib3270_eraseinput);
 }

 void action_Up(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(0,lib3270_cursor_up);
 }

 void action_Down(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(0,lib3270_cursor_down);
 }

 void action_Left(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(0,lib3270_cursor_left);
 }

 void action_Right(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(0,lib3270_cursor_right);
 }

 void action_Newline(GtkWidget *w, gpointer user_data)
 {
 	clear_and_call(0,lib3270_cursor_newline);
 }

 static void toggle_action(GtkToggleAction *action, gpointer id)
 {
 	Trace("%s(%d)",__FUNCTION__,(int) id);
 	set_toggle((int) id,gtk_toggle_action_get_active(action));
 }

 static void toggle_set_action(GtkAction *action,int id)
 {
 	set_toggle(id,TRUE);
 }

 static void toggle_reset_action(GtkAction *action,int id)
 {
 	set_toggle(id,FALSE);
 }

 void gui_toogle_set_active(enum GUI_TOGGLE id, gboolean active)
 {
	if(gui_toggle_state[id] == active)
		return;

    gui_toggle_state[id] = active;

	SetBoolean("Toggles",gui_toggle_info[id].name,gui_toggle_state[id]);

    if(id == GUI_TOGGLE_BOLD)
		action_redraw(0);
 }

 static void toggle_gui(GtkToggleAction *action, int id)
 {
	gui_toogle_set_active((enum GUI_TOGGLE) id, gtk_toggle_action_get_active(action));
 }

/**
 * Default script action - Call defined commands 1 by 1
 *
 */
 static void action_script_activated(GtkWidget *widget, GtkWidget *topwindow)
 {
 	const gchar	*filename	= g_object_get_data(G_OBJECT(widget),"script_filename");
 	const gchar	*text	= g_object_get_data(G_OBJECT(widget),"script_text");
	gchar			**line;
	int				ln;

	Trace("Internal script_text: %p",text);

	if(text)
	{
		line = g_strsplit(text,"\n",-1);
	}
	else if(filename)
	{
		// TODO (perry#1#): Read and split filename contents
		return;
	}
	else
	{
		return;
	}

	for(ln = 0; line[ln]; ln++)
	{
		line[ln] = g_strstrip(line[ln]);
		if(*line[ln] && *line[ln] != '#')
			run_script_command_line(line[ln],NULL);
	}

	g_strfreev(line);

 }

 void init_gui_toggles(void)
 {
 	int f;

 	for(f=0;f<GUI_TOGGLE_COUNT;f++)
 		gui_toggle_state[f] = GetBoolean("Toggles", gui_toggle_info[f].name, gui_toggle_info[f].def);
 }

 static void action_ToggleGDKDebug(GtkToggleAction *action, gpointer user_data)
 {
	gdk_window_set_debug_updates(gtk_toggle_action_get_active(action));
 }

 static void action_pfkey(GtkAction *action, gpointer id)
 {
	Trace("Running PF %d",(int) id);

	if(!TOGGLED_KEEP_SELECTED)
		unselect();
 	lib3270_pfkey((int) id);
 }

 static void action_pakey(GtkAction *action, gpointer id)
 {
	Trace("Running PA %d",(int) id);

	if(!TOGGLED_KEEP_SELECTED)
		unselect();
 	lib3270_pakey((int) id);
 }

 gboolean check_key_action(GtkWidget *widget, GdkEventKey *event)
 {
 	int f;
 	int	state = event->state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_ALT_MASK);

	// Check for PF keys
	if(event->keyval >= GDK_F1 && event->keyval <= GDK_F12 && !(state & (GDK_CONTROL_MASK|GDK_ALT_MASK)))
	{
		GtkAction *action;

		f = (event->keyval - GDK_F1);

		action = (state & GDK_SHIFT_MASK) ? pf_action[f].shift : pf_action[f].normal;

		Trace("PF%02d: %s %s action: %p",f+1,gdk_keyval_name(event->keyval),state & GDK_SHIFT_MASK ? "Shift " : "",action);

		if(action)
			gtk_action_activate(action);
		else
			action_pfkey(NULL, (gpointer) (f + ((state & GDK_SHIFT_MASK) ? 13 : 1)));

		return TRUE;
	}

#ifdef WIN32
	// FIXME (perry#1#): Find a better way!
	if( event->keyval == 0xffffff && event->hardware_keycode == 0x0013)
		event->keyval = GDK_Pause;

	// Windows sets <ctrl> in left/right control
	else if(state & GDK_CONTROL_MASK && (event->keyval == GDK_Control_R || event->keyval == GDK_Control_L))
		state &= ~GDK_CONTROL_MASK;

#endif

	Trace("Key action 0x%04x: Name:%s %s%s%s keycode: 0x%04x state: 0x%04x",
			event->keyval,
			gdk_keyval_name(event->keyval),
			state & GDK_SHIFT_MASK ? "Shift " : "",
			state & GDK_CONTROL_MASK ? "Control " : "",
			state & GDK_ALT_MASK ? "Shift " : "",
			event->hardware_keycode,
			state);

    // Check for special keyproc actions
	for(f=0; f < G_N_ELEMENTS(keyboard_action);f++)
	{
//		Trace("%d keyval=%04x state=%04x",f,keyboard_action[f].keyval,keyboard_action[f].state);

		if(keyboard_action[f].keyval == event->keyval && state == keyboard_action[f].state)
		{
#ifdef DEBUG
			Trace("Action: %p  def: %p",keyboard_action[f].action,keyboard_action[f].def);
			if(keyboard_action[f].action)
			{
				Trace("Action_name: %s",gtk_action_get_name(keyboard_action[f].action));
			}
#endif
			if(keyboard_action[f].action)
				gtk_action_activate(keyboard_action[f].action);
			else if(!keyboard_action[f].def)
				return FALSE;
			else
				keyboard_action[f].def();
			return TRUE;
		}
	}

	return FALSE;
 }

 GtkAction * create_action_by_descriptor(const gchar *name, struct action_descriptor *data)
 {
	int			state		= data->attr.key_state & (GDK_SHIFT_MASK|GDK_CONTROL_MASK|GDK_ALT_MASK);
	int			f;

//	Trace("%s: %d",name,data->ui_type);

	switch(data->ui_type)
	{
	case UI_CALLBACK_TYPE_TOGGLE:
		data->action = GTK_ACTION(gtk_toggle_action_new(name, gettext(data->attr.label ? data->attr.label : data->name), gettext(data->attr.tooltip), data->attr.stock_id));
		break;

	case UI_CALLBACK_TYPE_DEFAULT:
		data->action = gtk_action_new(name, gettext(data->attr.label ? data->attr.label : data->name), gettext(data->attr.tooltip),data->attr.stock_id);
		break;

	case UI_CALLBACK_TYPE_SCROLL:
		if(data->sub >= 0 && data->sub <= G_N_ELEMENTS(action_scroll))
			action_scroll[data->sub] = data->action = gtk_action_new(name, gettext(data->attr.label ? data->attr.label : data->name), gettext(data->attr.tooltip),data->attr.stock_id);
		break;

	case UI_CALLBACK_TYPE_SCRIPT:

		if(data->script.text)
			g_object_set_data_full(G_OBJECT(data->action),"script_text",g_strdup(data->script.text),g_free);

		if(data->callback)
			g_signal_connect(G_OBJECT(data->action),"activate",G_CALLBACK(data->callback),topwindow);
		else
			g_signal_connect(G_OBJECT(data->action),"activate",G_CALLBACK(action_script_activated),topwindow);

		data->callback = 0;
		break;

	default:
		Trace("Invalid action type %d in %s",data->ui_type,data->name);
		return NULL;
	}

//	Trace("%s(%s), callback: %p action: %p",__FUNCTION__,name,data->callback,data->action);

	if(data->callback)
		g_signal_connect(G_OBJECT(data->action),data->ui_type == UI_CALLBACK_TYPE_TOGGLE ? "toggled" : "activate",G_CALLBACK(data->callback),data->user_data);

	// Check for special keyboard action
	for(f=0;f<G_N_ELEMENTS(keyboard_action);f++)
	{
		if(data->attr.key_value == keyboard_action[f].keyval && (state == keyboard_action[f].state))
		{
			keyboard_action[f].action = data->action;
			gtk_action_group_add_action(action_group[data->group],data->action);
			return data->action;
		}
	}

	// Check for PF actions
	if(data->attr.key_value >= GDK_F1 && data->attr.key_value <= GDK_F12 && !(state & (GDK_CONTROL_MASK|GDK_ALT_MASK)))
	{
		f = data->attr.key_value - GDK_F1;

		if(state&GDK_SHIFT_MASK)
			pf_action[f].shift = data->action;
		else
			pf_action[f].normal = data->action;
		gtk_action_group_add_action(action_group[data->group],data->action);
		return data->action;
	}

	// Register action
	if(data->attr.accel)
		gtk_action_group_add_action_with_accel(action_group[data->group],data->action,data->attr.accel);
	else
		gtk_action_group_add_action(action_group[data->group],data->action);

	return data->action;
 }

 LOCAL_EXTERN int get_action_info_by_name(const gchar *key, const gchar **names, const gchar **values, gchar **name, UI_CALLBACK *info)
 {
	static const struct _action
	{
		const gchar	* name;				/**< Name of the action */
		GCallback	  callback;			/**< Callback for "activated/toggled" signal */
	}
	action[] =
	{
		// Offline actions
		{	"Connect",			G_CALLBACK(action_connect)			},
		{	"LoadScreenDump",	G_CALLBACK(action_loadscreendump)	},
		{ 	"SetHostname",		G_CALLBACK(action_sethostname)		},
		{	"TestPattern",		G_CALLBACK(lib3270_testpattern)		},

		// Online actions
		{	"Redraw",			G_CALLBACK(action_redraw)			},
		{	"SaveScreen",		G_CALLBACK(action_savescreen)		},
		{	"PrintScreen",		G_CALLBACK(action_printscreen)		},
		{	"DumpScreen",		G_CALLBACK(action_dumpscreen)		},
		{	"Disconnect",		G_CALLBACK(action_disconnect)		},

		// Select actions
		{	"SelectField",		G_CALLBACK(action_selectfield)		},

		{ 	"SelectRight",		G_CALLBACK(action_selectright)		},
		{ 	"SelectLeft",		G_CALLBACK(action_selectleft)		},
		{ 	"SelectUp",			G_CALLBACK(action_selectup)			},
		{ 	"SelectDown",		G_CALLBACK(action_selectdown)		},

		{	"SelectionRight",	G_CALLBACK(action_selectionright)	},
		{	"SelectionLeft",	G_CALLBACK(action_selectionleft)	},
		{	"SelectionUp",		G_CALLBACK(action_selectionup)		},
		{	"SelectionDown",	G_CALLBACK(action_selectiondown)	},

		// Cursor Movement
		{ 	"CursorRight",		G_CALLBACK(action_Right)			},
		{ 	"CursorLeft",		G_CALLBACK(action_Left)				},
		{ 	"CursorUp",			G_CALLBACK(action_Up)				},
		{ 	"CursorDown",		G_CALLBACK(action_Down)				},
		{ 	"Newline",			G_CALLBACK(action_Newline)			},

		{	"NextField",		G_CALLBACK(lib3270_tab)				},
		{	"PreviousField",	G_CALLBACK(lib3270_backtab)			},

		{	"kpadd",			G_CALLBACK(action_kpadd)			},
		{	"kpsubtract",		G_CALLBACK(action_kpsubtract)		},

		// Edit actions
		{	"PasteNext",		G_CALLBACK(action_pastenext)		},
		{	"PasteTextFile",	G_CALLBACK(action_pastetextfile)	},
		{	"Reselect",			G_CALLBACK(action_reselect)				},
		{	"SelectAll",		G_CALLBACK(action_selectall)		},
		{	"EraseInput",		G_CALLBACK(erase_input_action)		},
		{	"Clear",			G_CALLBACK(clear_action)			},

		{	"Reset",			G_CALLBACK(action_Reset)			},
		{	"Escape",			G_CALLBACK(action_Reset)			},

		// File-transfer actions
		{	"Download",			G_CALLBACK(action_download)			},
		{	"Upload",   		G_CALLBACK(action_upload)			},

		// Selection actions
		{	"Append",			G_CALLBACK(action_append)			},
		{	"Unselect",			G_CALLBACK(action_unselect)			},
		{	"Copy",				G_CALLBACK(action_copy)				},
		{	"CopyAsTable",		G_CALLBACK(action_copyastable)		},

		{	"CopyAsImage",		G_CALLBACK(action_copyasimage)		},

		{	"PrintSelected",	G_CALLBACK(action_printselected)	},
		{	"SaveSelected",		G_CALLBACK(action_saveselected)		},

		{	"PrintClipboard",	G_CALLBACK(action_printclipboard)	},
		{	"SaveClipboard",	G_CALLBACK(action_saveclipboard)	},
		{	"Paste",			G_CALLBACK(action_paste)			},

		{	"FileMenu",			NULL								},
		{	"FTMenu",			NULL								},
		{	"NetworkMenu",		NULL								},
		{	"HelpMenu",			NULL								},
		{	"EditMenu",			NULL								},
		{	"OptionsMenu",		NULL								},
		{	"SettingsMenu",		NULL								},
		{	"ScriptsMenu",		NULL								},
		{	"ViewMenu",			NULL								},
		{	"InputMethod",		NULL								},
		{	"ToolbarMenu",		NULL								},
		{	"DebugMenu",		NULL								},
		{	"FontSettings",		NULL								},
		{	"Preferences",		NULL								},
		{	"Network",			NULL								},
		{	"Properties",		NULL								},
		{	"ScreenSizes",		NULL								},
		{	"About",			G_CALLBACK(action_about)			},
		{	"Quit",				G_CALLBACK(action_quit)				},
		{	"SelectColors",		G_CALLBACK(action_selectcolors)		},

		{	"Save",				NULL								},

		{ 	"Return",			G_CALLBACK(action_enter)			},
		{ 	"Enter",			G_CALLBACK(action_enter)			},

		// Lib3270 calls
		{ "EraseEOF",			G_CALLBACK(lib3270_eraseeof)		},
		{ "EraseEOL",			G_CALLBACK(lib3270_eraseeol)		},
		{ "Home",				G_CALLBACK(lib3270_firstfield)		},
		{ "DeleteWord",			G_CALLBACK(lib3270_deleteword)		},
		{ "EraseField",			G_CALLBACK(lib3270_deletefield)		},
		{ "Delete",				G_CALLBACK(lib3270_delete)			},
		{ "Erase",				G_CALLBACK(lib3270_erase)			},
		{ "SysREQ",				G_CALLBACK(lib3270_sysreq)			},
		{ "Attn",				G_CALLBACK(lib3270_attn)			},
		{ "Break",				G_CALLBACK(lib3270_break)			},

		{ "FirstField",			G_CALLBACK(lib3270_firstfield)		},

	};

	int			f;

	if(name)
		*name = NULL;

	memset(info,0,sizeof(UI_CALLBACK));

	if(!g_ascii_strcasecmp(key,"pfkey"))
	{
		info->type 		= UI_CALLBACK_TYPE_DEFAULT;
		info->label		= "pfkey";
		info->callback	= G_CALLBACK(action_pfkey);
		info->user_data = (gpointer) atoi(get_xml_attribute(names, values, "id"));
		if(name)
			*name = g_strdup_printf("pf%02d",(int) info->user_data);
		return 0;
	}

	if(!g_ascii_strcasecmp(key,"pakey"))
	{
		info->type 		= UI_CALLBACK_TYPE_DEFAULT;
		info->label		= "pakey";
		info->callback	= G_CALLBACK(action_pakey);
		info->user_data = (gpointer) atoi(get_xml_attribute(names, values, "id"));
		if(name)
			*name = g_strdup_printf("pa%02d",(int) info->user_data);
		return 0;
	}

	if(!g_ascii_strcasecmp(key,"toggle"))
	{
		// Check for lib3270 toggles
		const gchar *id = get_xml_attribute(names, values, "id");

		for(f=0;f<N_TOGGLES;f++)
		{
			if(!g_ascii_strcasecmp(id,get_toggle_name(f)))
			{
				info->type 		= UI_CALLBACK_TYPE_TOGGLE;
				info->callback	= G_CALLBACK(toggle_action);
				info->user_data = (gpointer) f;
				if(name)
					*name = g_strconcat("Toggle",get_toggle_name(f),NULL);
				return 0;
			}
		}

		// Check for GUI toggles
		for(f=0;f<G_N_ELEMENTS(gui_toggle_info);f++)
		{
			if(!g_ascii_strcasecmp(id,gui_toggle_info[f].name))
			{
				info->type 		= UI_CALLBACK_TYPE_TOGGLE;
				info->callback	= G_CALLBACK(toggle_gui);
				info->user_data = (gpointer) f;
				if(name)
					*name = g_strconcat("Toggle",gui_toggle_info[f].name,NULL);
				return 0;
			}
		}

		if(!g_ascii_strcasecmp(id,"gdkdebug"))
		{
			info->type 		= UI_CALLBACK_TYPE_TOGGLE;
			info->callback	= G_CALLBACK(action_ToggleGDKDebug);
			info->user_data = (gpointer) f;
			if(name)
				*name = g_strdup("ToggleGDKDebug");
			return 0;
		}

		return EINVAL;
	}

	if(!g_ascii_strcasecmp(key,"toggleset"))
	{
		const gchar *id = get_xml_attribute(names, values, "id");

		for(f=0;f<N_TOGGLES;f++)
		{
			if(!g_ascii_strcasecmp(id,get_toggle_name(f)))
			{
				info->type 		= UI_CALLBACK_TYPE_DEFAULT;
//				info->label		= toggle_info[f].set_label ? toggle_info[f].set_label : toggle_info[f].do_label;
				info->callback	= G_CALLBACK(toggle_set_action);
				info->user_data = (gpointer) f;
				if(name)
					*name = g_strconcat("ToggleSet",get_toggle_name(f),NULL);
				return 0;
			}
		}
		return EINVAL;
	}

	if(!g_ascii_strcasecmp(key,"togglereset"))
	{
		const gchar *id = get_xml_attribute(names, values, "id");

		for(f=0;f<N_TOGGLES;f++)
		{
			if(!g_ascii_strcasecmp(id,get_toggle_name(f)))
			{
				info->type 		= UI_CALLBACK_TYPE_DEFAULT;
//				info->label		= toggle_info[f].reset_label ? toggle_info[f].reset_label : toggle_info[f].do_label;
				info->callback	= G_CALLBACK(toggle_reset_action);
				info->user_data = (gpointer) f;
				if(name)
					*name = g_strconcat("ToggleReset",get_toggle_name(f),NULL);
				return 0;
			}
		}
		return EINVAL;
	}

	// Search internal actions
	for(f=0;f<G_N_ELEMENTS(action);f++)
	{
		if(!g_ascii_strcasecmp(key,action[f].name))
		{
			info->type = UI_CALLBACK_TYPE_DEFAULT;
//			info->label = action[f].label;
			info->callback = action[f].callback;
			return 0;
		}
	}

	// Search for plugin actions
	if(strchr(key,'.'))
	{
		gchar	*plugin_name = g_strdup(key);
		gchar	*entry_name	 = strchr(plugin_name,'.');
		GModule *plugin;

		*(entry_name++) = 0;

		plugin = get_plugin_by_name(plugin_name);
		if(plugin)
		{
			gpointer cbk;

			if(get_symbol_by_name(plugin,&cbk,"action_%s_activated",entry_name))
			{
				info->type 		= UI_CALLBACK_TYPE_DEFAULT;
				info->callback	= (GCallback) cbk;
				info->user_data	= (gpointer) topwindow;

			}
			else if(get_symbol_by_name(plugin,&cbk,"action_%s_toggled",entry_name))
			{
				info->type		= UI_CALLBACK_TYPE_TOGGLE;
				info->callback	= (GCallback) cbk;
				info->user_data	= (gpointer) topwindow;

			}
		}

		g_free(plugin_name);
		if(info->callback)
			return 0;
	}

	// Check if it's an external program action
	if(!g_ascii_strncasecmp(key,"call",4) && strlen(key) > 5 && g_ascii_isspace(*(key+4)))
	{

	}

	return ENOENT;
 }

 void action_group_set_sensitive(ACTION_GROUP_ID id, gboolean status)
 {
	gtk_action_group_set_sensitive(action_group[id],status);
 }

 static void set_ft_action_state(int state)
 {
	gtk_action_group_set_sensitive(action_group[ACTION_GROUP_FT],state);
 }

 void init_actions(void)
 {
	static const gchar	*group_name[]	= { "default", "online", "offline", "selection", "clipboard", "paste", "filetransfer" };
 	GtkAction				*dunno			= gtk_action_new("Dunno",NULL,NULL,NULL);
	int						f;

	#ifdef DEBUG
		if(ACTION_GROUP_MAX != G_N_ELEMENTS(group_name))
		{
			Trace("Unexpected action_group size, found %d, expecting %d",(int) G_N_ELEMENTS(group_name),(int) ACTION_GROUP_MAX);
			exit(-1);
		}
	#endif

	for(f=0;f<ACTION_GROUP_MAX;f++)
	{
		action_group[f] = gtk_action_group_new(group_name[f]);

		Trace("action(%d): %p %s",f,action_group[f],group_name[f]);
		gtk_action_group_set_translation_domain(action_group[f], PACKAGE_NAME);
	}

	action_group[f] = 0;
	g_object_set_data(G_OBJECT(topwindow),"ActionGroups",action_group);

	for(f=0;f<G_N_ELEMENTS(action_by_id);f++)
		action_by_id[f] = dunno;

#ifdef 	X3270_FT
	set_ft_action_state(0);
	register_schange(ST_3270_MODE, set_ft_action_state);
#else
	gtk_action_group_set_sensitive(ft_actions,FALSE);
#endif

 }

 void update_3270_toggle_action(int toggle, int value)
 {
	const gchar	*name = get_toggle_name(toggle);

 	GtkAction	*action;
 	gchar		*ptr;

	ptr = g_strconcat("Toggle",name,NULL);
	action = get_action_by_name(ptr);
	g_free(ptr);

	// Update toggle buttons
	if(action)
		gtk_toggle_action_set_active((GtkToggleAction *) action,value);

	// Update toolbar items
	ptr = g_strconcat("ToggleReset",name,NULL);
	action = get_action_by_name(ptr);
	g_free(ptr);

	if(action)
		gtk_action_set_visible(action,value ? TRUE : FALSE);

	ptr = g_strconcat("ToggleSet",name,NULL);
	action = get_action_by_name(ptr);
	g_free(ptr);
	if(action)
		gtk_action_set_visible(action,value ? FALSE : TRUE);

 }



