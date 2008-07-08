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

 #include <globals.h>

 #include "g3270.h"
 #include <lib3270/kybdc.h>
 #include <lib3270/actionsc.h>
 #include <lib3270/toggle.h>

 #ifndef GDK_NUMLOCK_MASK
	#define GDK_NUMLOCK_MASK GDK_MOD2_MASK
 #endif

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

/*---[ Callback tables ]----------------------------------------------------------------------------------------*/
/*
 #ifdef DEBUG
     #define DECLARE_KEYPROC(key, state, action) { key, state, #key " (" #state ")", action, #action }
 #else
     #define DECLARE_KEYPROC(key, state, action) { key, state, action }
 #endif

 #ifdef DEBUG
     #define DECLARE_ACTION(key, state, action, cause, parm1, parm2) { key, state, #key " (" #state ")", action, #action, cause, parm1, parm2 }
 #else
     #define DECLARE_ACTION(key, state, action, cause, parm1, parm2) { key, state, action, cause, parm1, parm2 }
 #endif
*/

 #ifdef DEBUG
	#define LIB3270_ACTION(key,state,action) { key, state, #key " (" #state ")", (void (*)(GtkWidget *, gpointer)) action_lib3270, (gpointer) action }
	#define PF_ACTION(key,state,action) { key, state, #key " (" #state ")", (void (*)(GtkWidget *, gpointer)) action_pf, (gpointer) action }
	#define G3270_ACTION(key,state,action) { key, state, #key " (" #state ")", (void (*)(GtkWidget *, gpointer)) action, 0 }
	#define TOGGLE_ACTION(key,state,action) { key, state, #key " (" #state ")", (void (*)(GtkWidget *, gpointer)) action_Toggle, (gpointer) action }
 #else
	#define LIB3270_ACTION(key,state,action) { key, state, (void (*)(GtkWidget *, gpointer)) action_lib3270, (gpointer) action }
	#define PF_ACTION(key,state,action) { key, state, (void (*)(GtkWidget *, gpointer)) action_pf, (gpointer) action }
	#define G3270_ACTION(key,state,action) { key, state, (void (*)(GtkWidget *, gpointer)) action, 0 }
	#define TOGGLE_ACTION(key,state,action) { key, state, (void (*)(GtkWidget *, gpointer)) action_Toggle, (gpointer) action }
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

#define IS_FUNCTION_KEY(event)   (event->keyval >= GDK_F1 && event->keyval <= GDK_F12 && !(event->state & (GDK_MOD1_MASK|GDK_CONTROL_MASK)))

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static void action_lib3270(GtkWidget *w, XtActionProc proc)
 {
 	action_internal(proc,IA_DEFAULT,CN,CN);
 }

 static void action_pf(GtkWidget *w, gpointer id)
 {
	action_internal(PF_action, IA_DEFAULT, id, CN);
 }

 void action_PageUP(GtkWidget *w, gpointer user_data)
 {
	WaitingForChanges = TRUE;

	// TODO (perry#1#): Read FN association from configuration file.
	action_internal(PF_action, IA_DEFAULT, "7", CN);
 }

 void action_PageDown(GtkWidget *w, gpointer user_data)
 {
	WaitingForChanges = TRUE;

	// TODO (perry#1#): Read FN association from configuration file.
	action_internal(PF_action, IA_DEFAULT, "8", CN);
 }

 gboolean KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
 	static const struct WindowActions keyproc[] =
 	{
		LIB3270_ACTION( GDK_Return,			0,					Enter_action),
		LIB3270_ACTION( GDK_KP_Enter,		0,					Enter_action),
		LIB3270_ACTION( GDK_Insert,			0,					ToggleInsert_action),

		LIB3270_ACTION( GDK_Left,			0,					Left_action),
		LIB3270_ACTION( GDK_Up,				0,					Up_action),
		LIB3270_ACTION( GDK_Right,			0,					Right_action),
		LIB3270_ACTION( GDK_Down,			0,					Down_action),

		LIB3270_ACTION( GDK_ISO_Left_Tab,	0,					BackTab_action),
		LIB3270_ACTION( GDK_Tab,			0,					Tab_action),
		LIB3270_ACTION( GDK_KP_Add,			GDK_NUMLOCK_MASK,	Tab_action),

		LIB3270_ACTION( GDK_r,				GDK_CONTROL_MASK,	Reset_action),
		LIB3270_ACTION( GDK_Escape,			0,					Reset_action),

		LIB3270_ACTION( GDK_Delete,			0,					Delete_action),
		LIB3270_ACTION( GDK_BackSpace,		0,					Erase_action),

		G3270_ACTION( 	GDK_Page_Up,		0,					action_PageUP),
		G3270_ACTION( 	GDK_Page_Down,		0,					action_PageDown),

		PF_ACTION(		GDK_Page_Up,		GDK_SHIFT_MASK,		"23"),
		PF_ACTION(		GDK_Page_Down,		GDK_SHIFT_MASK,		"24"),

 	};

 	int		f;
 	char	buffer[10];

	/* Is function key? */
	if(IS_FUNCTION_KEY(event))
    {
        sprintf(buffer,"%d",(event->keyval - GDK_F1)+1);
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

static const char *ui_mainwindow_ui_desc =
"<ui>"
"	<menubar name='MainwindowMenubar'>"
"		<menu action='FileMenu'>"
"			<menuitem action='Quit'/>"
"		</menu>"
"		<menu action='EditMenu'>"
"			<menuitem action='Copy'/>"
"			<menuitem action='Paste'/>"
"		</menu>"
"		<menu action='NetworkMenu'>"
"			<menuitem action='Connect' />"
"			<menuitem action='Disconnect' />"
"		</menu>"
"		<menu action='HelpMenu'>"
"			<menuitem action='About' />"
"		</menu>"
"	</menubar>"
"</ui>";

/*
	The name of the action.
	The stock id for the action, or the name of an icon from the icon theme.
	The label for the action. This field should typically be marked for translation, see gtk_action_group_set_translation_domain(). If label is NULL, the label of the stock item with id stock_id is used.
	The accelerator for the action, in the format understood by gtk_accelerator_parse().
	The tooltip for the action. This field should typically be marked for translation, see gtk_action_group_set_translation_domain().
	The function to call when the action is activated.

	http://library.gnome.org/devel/gtk/stable/gtk-Stock-Items.html

 */
 static GtkActionEntry internal_action_entries[] =
 {
 	{	"FileMenu",			NULL,					"_File",		NULL,			NULL,	NULL			},
 	{	"NetworkMenu",		NULL,					"_Network",		NULL,			NULL,	NULL			},
 	{	"HelpMenu",			NULL,					"Help",			NULL,			NULL,	NULL			},
 	{	"EditMenu",			NULL,					"_Edit",		NULL,			NULL,	NULL			},

 	{	"About",			GTK_STOCK_ABOUT,		"About",		NULL,			NULL,	NULL			},
 	{	"Connect",			GTK_STOCK_CONNECT,		"_Connect",		NULL,			NULL,	NULL			},
 	{	"Disconnect",		GTK_STOCK_DISCONNECT,	"_Disconnect",	NULL,			NULL,	NULL			},
 	{	"Quit",				GTK_STOCK_QUIT,			"Quit",			"<control>X",	NULL,	gtk_main_quit	},
 	{	"Copy",				GTK_STOCK_COPY,			"Copy",			"<control>C",	NULL,	NULL			},
 	{	"Paste",			GTK_STOCK_PASTE,		"Paste",		"<control>V",	NULL,	NULL			},
 };

 GtkUIManager * LoadApplicationUI(GtkWidget *widget)
 {
	GtkUIManager 	*ui_manager = gtk_ui_manager_new();
	GtkActionGroup	*actions;
	GError			*error = NULL;

	actions = gtk_action_group_new("InternalActions");
	gtk_action_group_add_actions(actions, internal_action_entries, G_N_ELEMENTS (internal_action_entries), topwindow);
	gtk_ui_manager_insert_action_group(ui_manager,actions, 0);

	if(!gtk_ui_manager_add_ui_from_string(ui_manager, ui_mainwindow_ui_desc, -1, &error))
		g_error("building menus failed: %s", error->message);

	return ui_manager;
 }

