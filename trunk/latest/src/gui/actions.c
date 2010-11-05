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
 #include <lib3270/actions.h>
 #include <gdk/gdkkeysyms.h>

 #ifndef GDK_NUMLOCK_MASK
	#define GDK_NUMLOCK_MASK GDK_MOD2_MASK
 #endif

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
 #define DECLARE_LIB3270_CURSOR_ACTION( name )			{ ACTION_TYPE_CALL, #name, (GCallback) lib3270_cursor_ ## name, NULL },
 #define DECLARE_LIB3270_FKEY_ACTION( name )			{ ACTION_TYPE_FKEY, #name, (GCallback) lib3270_ ## name, NULL },

 static const struct action_data
 {
 	unsigned short 	type;				/**< Action type (used to define the callback method) */
 	const 				gchar *name;	/**< Action name */
 	GCallback			callback;		/**< Action method */
 	const				gchar *attr;	/**< Action attributes */
 } action_table[] =
 {
	#include "action_table.h"
	#include "lib3270/action_table.h"
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

 static struct _keyboard_action
 {
	guint			keyval;
	GdkModifierType	state;
	GtkAction		*action;
 } keyboard_action[] =
 {
	{ GDK_Left,				0,					NULL	},
	{ GDK_Up,				0,					NULL	},
	{ GDK_Right,			0,					NULL	},
	{ GDK_Down,				0,					NULL	},
	{ GDK_Tab,				0,					NULL	},
	{ GDK_ISO_Left_Tab,		GDK_SHIFT_MASK,		NULL	},
	{ GDK_KP_Left,			0,					NULL	},
	{ GDK_KP_Up,			0,					NULL	},
	{ GDK_KP_Right,			0,					NULL	},
	{ GDK_KP_Down,			0,					NULL	},
	{ GDK_KP_Add,			GDK_NUMLOCK_MASK,	NULL	},

	{ GDK_3270_PrintScreen,	0,					NULL	},
	{ GDK_Sys_Req,			0,					NULL	},

	{ GDK_Print,			GDK_CONTROL_MASK,	NULL	},
	{ GDK_Print,			GDK_SHIFT_MASK,		NULL	},

#ifdef WIN32
	{ GDK_Pause,			0,					NULL	},
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
		action_scroll[f] = action_nop;

	for(f=0;f<G_N_ELEMENTS(keyboard_action);f++)
		keyboard_action[f].action = action_nop;

	for(f=0;f<G_N_ELEMENTS(toggle_action);f++)
	{
		toggle_action[f].set	= action_nop;
		toggle_action[f].reset	= action_nop;
		toggle_action[f].toggle	= action_nop;
	}

	for(f=0;f<G_N_ELEMENTS(pf_action);f++)
	{
		pf_action[f].normal = action_nop;
		pf_action[f].shift  = action_nop;
	}

 }

 void update_3270_toggle_action(int toggle, int value)
 {
	#warning Implementar
 }

 void action_group_set_sensitive(ACTION_GROUP_ID id, gboolean status)
 {
	// TODO (perry#1#): Replace action_group_set_sensitive() for a macro call to gtk_action_group_set_sensitive
	gtk_action_group_set_sensitive(action_group[id],status);
 }

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
 
 static const struct action_data * get_action_data(const gchar *name, GError **error)
 {
	int f;
	
	for(f=0;f<G_N_ELEMENTS(action_table);f++)
	{
		if(!g_ascii_strcasecmp(action_table[f].name,name))
			return action_table+f;
	}
	
	*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Invalid or unexpected action name: %s" ), name);

	return NULL;
 }


 int action_setup_default(GtkAction *action, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
	const struct action_data *data = get_action_data(action, error);
	
	if(!data)
		return -1;
	
	if(!connect)
		return 0;

	switch ((data->type)
	{
	case ACTION_TYPE_GUI:
		g_signal_connect(G_OBJECT(action),"activate",data->callback,0);
		break;
		
 	case ACTION_TYPE_CALL:
		break;
		
 	case ACTION_TYPE_CLEAR_SELECTION:
		break;
		
	default:
		*error = g_error_new(ERROR_DOMAIN,EINVAL, _( "Invalid or unexpected action name: %s" ), ???);
		return -1;
	}
		
//		g_signal_connect(G_OBJECT(action),"activate",data->callback,0);
		
	return 0;
 }

 int action_setup_toggle(GtkAction *action, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
 }
 
 int action_setup_toggleset(GtkAction *action, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
 }
 
 int action_setup_togglereset(GtkAction *action, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
 }
 
 int action_setup_pfkey(GtkAction *action, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
 }

 int action_setup_pakey(GtkAction *action, gboolean connect, const gchar **names, const gchar **values, GError **error)
 {
 }

