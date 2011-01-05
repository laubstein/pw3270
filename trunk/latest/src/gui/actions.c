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
 * Este programa está nomeado como actions.c e possui - linhas de código.
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

 #include "gui.h"
 #include "actions.h"
 #include "ui_parse.h"
 #include <lib3270/actions.h>
 #include <glib.h>
 #include <stdlib.h>
 #include <gdk/gdkkeysyms.h>

 #ifndef GDK_NUMLOCK_MASK
	#define GDK_NUMLOCK_MASK GDK_MOD2_MASK
 #endif

 #define ERROR_DOMAIN g_quark_from_static_string("uiparser")

/*---[ Action callback table ]----------------------------------------------------------------------------------*/

 enum action_type
 {
 	ACTION_TYPE_GUI,
 	ACTION_TYPE_CALL,
 	ACTION_TYPE_CLEAR_SELECTION,
 	ACTION_TYPE_FKEY,

 	ACTION_TYPE_INVALID
 };

 // Redefine action-table macros
 #undef DECLARE_PW3270_ACTION
 #undef DECLARE_LIB3270_ACTION
 #undef DECLARE_LIB3270_CLEAR_SELECTION_ACTION
 #undef DECLARE_LIB3270_KEY_ACTION
 #undef DECLARE_LIB3270_CURSOR_ACTION
 #undef DECLARE_LIB3270_FKEY_ACTION

 #define DECLARE_PW3270_ACTION( name, attr )			{ ACTION_TYPE_GUI, #name , (GCallback) action_ ## name, attr },
 #define DECLARE_LIB3270_ACTION( name )  				{ ACTION_TYPE_CALL, #name, (GCallback) lib3270_ ## name, NULL },
 #define DECLARE_LIB3270_CLEAR_SELECTION_ACTION( name ) { ACTION_TYPE_CLEAR_SELECTION, #name, (GCallback) lib3270_ ## name, NULL },
 #define DECLARE_LIB3270_KEY_ACTION( name )				{ ACTION_TYPE_CALL, #name, (GCallback) lib3270_ ## name, NULL },
 #define DECLARE_LIB3270_CURSOR_ACTION( name )			{ ACTION_TYPE_CALL, "cursor" #name, (GCallback) lib3270_cursor_ ## name, NULL },
 #define DECLARE_LIB3270_FKEY_ACTION( name )			{ ACTION_TYPE_FKEY, #name, (GCallback) lib3270_ ## name, NULL },

 static const struct action_data
 {
 	enum action_type	type;			/**< Action type (used to define the callback method) */
 	const 				gchar *name;	/**< Action name */
 	GCallback			callback;		/**< Action method */
 	const				gchar *attr;	/**< Action attributes */
 } action_table[] =
 {
	#include "action_table.h"
	#include "lib3270/action_table.h"

	// Compatibility
	{ ACTION_TYPE_GUI, 				"return", 			(GCallback) action_enter,		"" },
	{ ACTION_TYPE_CLEAR_SELECTION,	"escape", 			(GCallback) lib3270_reset,		"" },
	{ ACTION_TYPE_CALL, 			"previousfield",	(GCallback) lib3270_backtab,	"" },
	{ ACTION_TYPE_CALL, 			"nextfield",		(GCallback) lib3270_tab,		"" },
	{ ACTION_TYPE_CALL, 			"home",				(GCallback) lib3270_firstfield,	"" },

 };


/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 GtkActionGroup		* action_group[ACTION_GROUP_MAX+1]	= { 0 };
 GtkAction			* action_by_id[ACTION_ID_MAX]		= { NULL };
 GtkAction 			* action_scroll[ACTION_SCROLL_MAX]	= { NULL };

 const gchar *action_group_name[ACTION_GROUP_MAX] =
 {
		"default",
		"online",
		"offline",
		"selection",
		"clipboard",
		"paste",
		"filetransfer"
 };

 const gchar *action_id_name[ACTION_ID_MAX] =
 {
		"copyastable",
		"copyasimage",
		"pastenext",
		"unselect",
		"reselect"
 };

 static struct _keyboard_action
 {
	guint			keyval;
	GdkModifierType	state;
	GtkAction		*action;
	GCallback		def;
 } keyboard_action[] =
 {
	{ GDK_Left,				0,					NULL,	G_CALLBACK(lib3270_cursor_left)		},
	{ GDK_Up,				0,					NULL,	G_CALLBACK(lib3270_cursor_up)		},
	{ GDK_Right,			0,					NULL,	G_CALLBACK(lib3270_cursor_right)	},
	{ GDK_Down,				0,					NULL,	G_CALLBACK(lib3270_cursor_down)		},
	{ GDK_Tab,				0,					NULL,	G_CALLBACK(lib3270_tab)				},
	{ GDK_ISO_Left_Tab,		GDK_SHIFT_MASK,		NULL,	G_CALLBACK(lib3270_backtab)			},
	{ GDK_KP_Left,			0,					NULL,	G_CALLBACK(lib3270_cursor_left)		},
	{ GDK_KP_Up,			0,					NULL,	G_CALLBACK(lib3270_cursor_up)		},
	{ GDK_KP_Right,			0,					NULL,	G_CALLBACK(lib3270_cursor_right)	},
	{ GDK_KP_Down,			0,					NULL,	G_CALLBACK(lib3270_cursor_down)		},
	{ GDK_KP_Add,			GDK_NUMLOCK_MASK,	NULL,	G_CALLBACK(lib3270_tab)				},

	{ GDK_3270_PrintScreen,	0,					NULL,	G_CALLBACK(action_printscreen)		},
	{ GDK_Sys_Req,			0,					NULL,	G_CALLBACK(lib3270_sysreq)			},

	{ GDK_Print,			GDK_CONTROL_MASK,	NULL,	G_CALLBACK(action_printscreen)		},
	{ GDK_Print,			GDK_SHIFT_MASK,		NULL,	G_CALLBACK(lib3270_sysreq)			},

#ifdef WIN32
	{ GDK_Pause,			0,					NULL,	0									},
#endif

 };

 static struct _toggle_action
 {
 	GtkAction *set;
 	GtkAction *reset;
 	GtkAction *toggle;
 } toggle_action[N_TOGGLES+GUI_TOGGLE_COUNT] = { { 0 } };

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

 static	GtkAction	* action_nop = NULL;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 void init_actions(void)
 {
	int f;

	if(!action_nop)
		action_nop = gtk_action_new("NOP",NULL,NULL,NULL);

	memset(action_group,0,sizeof(action_group));

	for(f=0;f<G_N_ELEMENTS(action_group_name);f++)
		action_group[f] = gtk_action_group_new(action_group_name[f]);

	for(f=0;f<G_N_ELEMENTS(action_by_id);f++)
		action_by_id[f] = action_nop;

	for(f=0;f<G_N_ELEMENTS(action_scroll);f++)
		action_scroll[f] = NULL;

	for(f=0;f<G_N_ELEMENTS(keyboard_action);f++)
		keyboard_action[f].action = 0;

	for(f=0;f<G_N_ELEMENTS(toggle_action);f++)
	{
		toggle_action[f].set	= action_nop;
		toggle_action[f].reset	= action_nop;
		toggle_action[f].toggle	= action_nop;
	}
 }

 void update_3270_toggle_action(int toggle, int value)
 {
	if(toggle_action[toggle].toggle && GTK_IS_TOGGLE_ACTION(toggle_action[toggle].toggle))
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(toggle_action[toggle].toggle), value == 0 ? FALSE : TRUE);

	if(toggle_action[toggle].reset)
		gtk_action_set_visible(toggle_action[toggle].reset,value);

	if(toggle_action[toggle].set)
		gtk_action_set_visible(toggle_action[toggle].set,!value);
 }

 void action_group_set_sensitive(ACTION_GROUP_ID id, gboolean status)
 {
	// TODO (perry#1#): Replace action_group_set_sensitive() for a macro call to gtk_action_group_set_sensitive
	gtk_action_group_set_sensitive(action_group[id],status);
 }

/*
 void set_action_sensitive_by_name(const gchar *name,gboolean sensitive)
 {
	int p;

	for(p = 0; action_group[p]; p++)
	{
		GtkAction *act = gtk_action_group_get_action(action_group[p],name);
		if(act)
		{
			Trace("%s: %s(%s)",__FUNCTION__,name,sensitive ? "sensitive" : "insensitive");
			gtk_action_set_sensitive(act,sensitive);
			return;
		}
	}

	Trace("%s: %s isn't available",__FUNCTION__,name);

 }
 */

 static const struct action_data * get_action_data(const gchar *name, GError **error)
 {
	int f;

	for(f=0;f<G_N_ELEMENTS(action_table);f++)
	{
		if(!g_ascii_strcasecmp(action_table[f].name,name))
			return action_table+f;
	}

	return NULL;
 }

 static void clear_and_call(GtkAction *action, void (*call)(void))
 {
 	unselect();
 	call();
 }

 static void pfkey(GtkAction *action, gpointer key)
 {
 	if(!TOGGLED_KEEP_SELECTED)
		unselect();

	Trace("%s: action=%p key=%d",__FUNCTION__,action,(int) key);

 	lib3270_pfkey((int) key);
 }

 static void pakey(GtkAction *action, gpointer key)
 {
	if(!TOGGLED_KEEP_SELECTED)
		unselect();

 	lib3270_pakey((int) key);
 }

 int action_setup_default(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
	const struct action_data *data = get_action_data(name, error);

//	Trace("data=%p strchr=%p",data,strchr(name,'.'));

	if(data)
	{
		// Internal action, setup correctly
		if(connect)
		{
			switch(data->type)
			{
			case ACTION_TYPE_GUI:
				g_signal_connect(G_OBJECT(action),"activate",data->callback,0);
				break;

			case ACTION_TYPE_CALL:
				g_signal_connect(G_OBJECT(action),"activate",data->callback,0);
				break;

			case ACTION_TYPE_CLEAR_SELECTION:
				g_signal_connect(G_OBJECT(action),"activate",G_CALLBACK(clear_and_call),data->callback);
				break;

			default:
				Trace("%s - unexpected action %s",__FUNCTION__,name);
				*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Invalid or unexpected action name: %s" ), name);
				return -1;
			}
		}
		return 0;
	}
#ifdef HAVE_PLUGINS
	else if(strchr(name,'.'))
	{
		// Plugin action
		gchar				* plugin_name	= g_strdup(name);
		gchar				* entry_name	= strchr(plugin_name,'.');
		GModule			 	* plugin;

		*(entry_name++) = 0;

		plugin = get_plugin_by_name(plugin_name);
		if(plugin)
		{
			GCallback callback;

			if(!get_symbol_by_name(plugin,(gpointer) &callback,"action_%s_%s",entry_name,GTK_IS_TOGGLE_ACTION(action) ? "toggled" : "activated"))
			{
#ifdef DEBUG
				gtk_action_set_sensitive(GTK_ACTION(action),FALSE);
#else
				gtk_action_set_visible(GTK_ACTION(action),FALSE);
#endif
			}
			else if(connect)
			{
				g_signal_connect(G_OBJECT(action),GTK_IS_TOGGLE_ACTION(action) ? "toggled" : "activate",callback,topwindow);
			}
		}
		else
		{
#ifdef DEBUG
			gtk_action_set_sensitive(GTK_ACTION(action),FALSE);
#else
			gtk_action_set_visible(GTK_ACTION(action),FALSE);
#endif
		}

		g_free(plugin_name);
		return 0;

	}
#endif

	return -1;
 }

 static int get_toggle_id(const gchar **names, const gchar **values, GError **error)
 {
 	const gchar	* toggle_name = get_xml_attribute(names, values, "id");
 	int 			  f;

	if(!toggle_name)
	{
		*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Invalid toggle definition" ));
		return -1;
	}

 	// Check for lib3270 toggles
 	for(f=0;f<N_TOGGLES;f++)
 	{
 		if(!g_ascii_strcasecmp(toggle_name,get_3270_toggle_name(f)))
			return f;
 	}

 	// Check for GUI toggles
 	for(f=0;f<GUI_TOGGLE_COUNT;f++)
 	{
 		if(!g_ascii_strcasecmp(toggle_name,gui_toggle_name[f]))
			return f+N_TOGGLES;
 	}

	*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Unexpected or invalid toggle \"%s\"" ), toggle_name);

 	return -1;
 }

 static void lib3270_toggle_action(GtkToggleAction *action, gpointer id)
 {
	gboolean active = gtk_toggle_action_get_active(action);
	set_toggle((int) id,active);
 }

 LOCAL_EXTERN void gui_toogle_set_visible(enum GUI_TOGGLE id, gboolean visible)
 {
 	int idx = ((int) id) + N_TOGGLES;

	Trace("*************** %p %p %p ",toggle_action[idx].reset, toggle_action[idx].set, toggle_action[idx].toggle);

	if(toggle_action[idx].reset)
		gtk_action_set_visible(toggle_action[idx].reset,visible && gui_toggle_state[id]);

	if(toggle_action[idx].set)
		gtk_action_set_visible(toggle_action[idx].set,visible && !gui_toggle_state[id]);

	if(toggle_action[idx].toggle)
		gtk_action_set_visible(toggle_action[idx].toggle,visible);

 }

 void gui_toogle_set_active(enum GUI_TOGGLE id, gboolean active)
 {
 	int idx = ((int) id) + N_TOGGLES;

 	if(gui_toggle_state[(int) id] == active)
		return;

	Trace("*************** %p %p %p ",toggle_action[idx].reset, toggle_action[idx].set, toggle_action[idx].toggle);

	gui_toggle_state[(int) id] = active;

	SetBoolean("Toggles",gui_toggle_name[id],gui_toggle_state[id]);

	if(toggle_action[idx].reset)
		gtk_action_set_visible(toggle_action[idx].reset,gui_toggle_state[id]);

	if(toggle_action[idx].set)
		gtk_action_set_visible(toggle_action[idx].set,!gui_toggle_state[id]);

	if(toggle_action[idx].toggle)
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(toggle_action[idx].toggle),active);

	if(id == GUI_TOGGLE_BOLD)
		action_redraw(0);
 }

 static void gui_toggle_action(GtkToggleAction *action, gpointer id)
 {
	gui_toogle_set_active((enum GUI_TOGGLE) id, gtk_toggle_action_get_active(action));
 }

 static void action_toggle_gdk_debug(GtkToggleAction *action, gpointer user_data)
 {
	gdk_window_set_debug_updates(gtk_toggle_action_get_active(action));
 }

 int action_setup_toggle(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
 	int id = get_toggle_id(names,values,error);

 	if(id < 0)
	{
		name = get_xml_attribute(names, values, "id");

		if(!name)
			return -1;

		Trace("[%s] %d",name,g_strcasecmp(name,"gdkdebug"));

		if(!g_strcasecmp(name,"GDKDebug"))
		{
			g_error_free(*error);
			*error = NULL;

			if(connect)
				g_signal_connect(G_OBJECT(action),"toggled",G_CALLBACK(action_toggle_gdk_debug),(gpointer) id);
			return 0;
		}
		return -1;
	}

	if(!connect)
		return 0;

	toggle_action[id].toggle = action;

	if(id >= N_TOGGLES)
	{
		id -= N_TOGGLES;
		Trace("Connect toggle \"%s\" to gui (id=%d)",get_xml_attribute(names, values, "id"),id);
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),gui_toggle_state[id]);
		g_signal_connect(G_OBJECT(action),"toggled",G_CALLBACK(gui_toggle_action),(gpointer) id);
	}
	else
	{
		Trace("Connect toggle \"%s\" to lib3270",get_xml_attribute(names, values, "id"));
		gtk_toggle_action_set_active(GTK_TOGGLE_ACTION(action),Toggled(id) ? TRUE : FALSE);
		g_signal_connect(G_OBJECT(action),"toggled",G_CALLBACK(lib3270_toggle_action),(gpointer) id);
	}


	return 0;
 }

 static void lib3270_toggle_set(GtkAction *action, gpointer id)
 {
	set_toggle((int) id,TRUE);
 }

 static void gui_toggle_set(GtkAction *action, gpointer id)
 {
	gui_toogle_set_active((enum GUI_TOGGLE) id,TRUE);
 }

 int action_setup_toggleset(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
 	gboolean state = FALSE;
 	int id = get_toggle_id(names,values,error);

 	if(id < 0)
		return -1;

	if(!connect)
		return 0;

	toggle_action[id].set = action;

	if(id >= N_TOGGLES)
	{
		g_signal_connect(G_OBJECT(action),"activate",G_CALLBACK(gui_toggle_set),(gpointer) (id - N_TOGGLES));
		state = gui_toggle_state[ ((int) id) - N_TOGGLES];
	}
	else
	{
		g_signal_connect(G_OBJECT(action),"activate",G_CALLBACK(lib3270_toggle_set),(gpointer) id);
		state = Toggled(id) ? TRUE : FALSE;
	}

	gtk_action_set_visible(action,!state);

	if(toggle_action[(int) id].reset)
		gtk_action_set_visible(toggle_action[(int) id].reset,state);

	return 0;
 }

 static void lib3270_toggle_reset(GtkAction *action, gpointer id)
 {
	set_toggle((int) id,FALSE);
 }

 static void gui_toggle_reset(GtkAction *action, gpointer id)
 {
	gui_toogle_set_active((enum GUI_TOGGLE) id,FALSE);
 }

 int action_setup_togglereset(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
 	gboolean state = FALSE;
 	int id = get_toggle_id(names,values,error);

 	if(id < 0)
		return -1;

	if(!connect)
		return 0;

	toggle_action[id].reset = action;

	if(id >= N_TOGGLES)
	{
		g_signal_connect(G_OBJECT(action),"activate",G_CALLBACK(gui_toggle_reset),(gpointer) (id - N_TOGGLES));
		state = gui_toggle_state[ ((int) id) - N_TOGGLES];
	}
	else
	{
		g_signal_connect(G_OBJECT(action),"activate",G_CALLBACK(lib3270_toggle_reset),(gpointer) id);
		state = Toggled(id) ? TRUE : FALSE;
	}

	gtk_action_set_visible(action,state);

	if(toggle_action[(int) id].set)
		gtk_action_set_visible(toggle_action[(int) id].set,!state);

	return 0;
 }

 int action_setup_pfkey(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
 	static const gchar *attr[] = { "id", "value" };
 	int f;
	const struct action_data *data = get_action_data(name, error);

	if(!data)
		return -1;

	for(f=0;f < G_N_ELEMENTS(attr);f++)
	{
		const gchar *id = get_xml_attribute(names, values, attr[f]);
		if(id)
		{
			int vl = atoi(id);
			int pflen = G_N_ELEMENTS(pf_action);

			if(vl < 1 || vl > (pflen * 2))
			{
				*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Attribute %s in %s is unexpected of invalid" ), attr[f], name);
				return -1;
			}

			if(vl <= pflen)
				pf_action[vl-1].normal = action;
			else
				pf_action[vl-(pflen+1)].shift = action;

			if(connect)
				g_signal_connect(G_OBJECT(action),"activate",G_CALLBACK(pfkey),(gpointer) vl);

			return 0;
		}


	}

	// No key attribute, error
	*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Incomplete PFkey definition (no id): %s" ), name);
	return -1;
 }

 int action_setup_pakey(GtkAction *action, const gchar *name, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
 	static const gchar *attr[] = { "id", "value" };
 	int f;
	const struct action_data *data = get_action_data(name, error);

	if(!data)
		return -1;

	for(f=0;f < G_N_ELEMENTS(attr);f++)
	{
		const gchar *id = get_xml_attribute(names, values, attr[f]);
		if(id)
		{
			int vl = atoi(id);
			if(vl < 1)
			{
				*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Attribute %s in %s is unexpected of invalid" ), attr[f], name);
				return -1;
			}

			if(connect)
				g_signal_connect(G_OBJECT(action),"activate",G_CALLBACK(pakey),(gpointer) vl);

			return 0;
		}


	}

	// No key attribute, error
	*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Incomplete PFkey definition (no id): %s" ), name);
	return -1;
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

#ifdef DEBUG
		Trace("PF%02d: %s %s action: %p",f+1,gdk_keyval_name(event->keyval),state & GDK_SHIFT_MASK ? "Shift " : "",action);

		if(action)
		{
			Trace("Action_name: \"%s\"", gtk_action_get_name(action));
		}

#endif

		if(action)
			gtk_action_activate(action);
		else
			pfkey(NULL, (gpointer) (f + ((state & GDK_SHIFT_MASK) ? 13 : 1)));

		return TRUE;
	}

#ifdef WIN32
	// FIXME (perry#1#): Find a better way!
	if( event->keyval == 0xffffff && event->hardware_keycode == 0x0013)
		event->keyval = GDK_Pause;
#endif

	Trace("Key action 0x%04x: %s %s keycode: 0x%04x",event->keyval,gdk_keyval_name(event->keyval),state & GDK_SHIFT_MASK ? "Shift " : "",event->hardware_keycode);

    // Check for special keyproc actions
	for(f=0; f < G_N_ELEMENTS(keyboard_action);f++)
	{
		if(keyboard_action[f].keyval == event->keyval && state == keyboard_action[f].state)
		{
			if(keyboard_action[f].action)
			{
				gtk_action_activate(keyboard_action[f].action);
				return TRUE;
			}
			else if(keyboard_action[f].def)
			{
				keyboard_action[f].def();
				return TRUE;
			}
			return FALSE;
		}
	}

 	return FALSE;
 }
