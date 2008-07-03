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

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

/*---[ Callback tables ]----------------------------------------------------------------------------------------*/

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

 struct WindowActions
 {
	guint	keyval;
	guint	state;

#ifdef DEBUG
	const char	*trace;
#endif

	void (*callback)(GtkWidget *w, gpointer data);

#ifdef DEBUG
	const char	 *action_trace;
#endif

 };


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 void action_Enter(GtkWidget *w, gpointer data)
 {
 	// TODO (perry#9#): Test if disconnected, if yes connect it to the last server
	action_internal(Enter_action, IA_DEFAULT, CN, CN);
 }

 void action_Left(GtkWidget *w, gpointer data)
 {
	action_internal(Left_action, IA_DEFAULT, CN, CN);
 }

 void action_Up(GtkWidget *w, gpointer data)
 {
	action_internal(Up_action, IA_DEFAULT, CN, CN);
 }

 void action_Right(GtkWidget *w, gpointer data)
 {
	action_internal(Right_action, IA_DEFAULT, CN, CN);
 }

 void action_Down(GtkWidget *w, gpointer data)
 {
	action_internal(Down_action, IA_DEFAULT, CN, CN);
 }

 gboolean KeyboardAction(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
 	static const struct WindowActions keyproc[] =
 	{
		DECLARE_KEYPROC( GDK_Return,		0,	action_Enter            ),
        DECLARE_KEYPROC( GDK_KP_Enter,		0,	action_Enter            ),
		DECLARE_KEYPROC( GDK_Left,			0,	action_Left				),
		DECLARE_KEYPROC( GDK_Up,			0,	action_Up				),
		DECLARE_KEYPROC( GDK_Right,			0,	action_Right			),
		DECLARE_KEYPROC( GDK_Down,			0,	action_Down				),

 	};

 	int f;

    /* Check for special keyproc actions */
	for(f=0; f < (sizeof(keyproc)/sizeof(struct WindowActions));f++)
	{
		if(keyproc[f].keyval == event->keyval && (event->state & keyproc[f].state) == keyproc[f].state)
		{
			Trace("Key: %s\tAction: %s",keyproc[f].trace,keyproc[f].action_trace);
			keyproc[f].callback(widget,user_data);
			return TRUE;
		}
	}



	return FALSE;
 }
