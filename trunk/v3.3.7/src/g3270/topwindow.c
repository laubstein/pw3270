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

 #include "g3270.h"
 #include <lib3270/toggle.h>

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 GtkWidget *topwindow = NULL;
 GdkCursor *wCursor[CURSOR_MODE_USER];

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static gboolean delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
 {
 	gtk_main_quit();
    return FALSE;
 }

 static void destroy( GtkWidget *widget, gpointer   data )
 {
    gtk_main_quit();
 }

 static void set_fullscreen(int value, int reason)
 {
 	Trace("Fullscren mode toggled (value: %d",value);
 	if(value)
		gtk_window_fullscreen(GTK_WINDOW(topwindow));
	else
		gtk_window_unfullscreen(GTK_WINDOW(topwindow));

 }

 int CreateTopWindow(void)
 {
 	static int cr[CURSOR_MODE_USER] = { GDK_ARROW, GDK_WATCH, GDK_X_CURSOR };
 	GtkWidget	*vbox;
 	int			f;

	topwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	for(f=0;f<CURSOR_MODE_USER;f++)
		wCursor[f] = gdk_cursor_new(cr[f]);

	g_signal_connect(G_OBJECT(topwindow),	"delete_event", 		G_CALLBACK(delete_event),			NULL);
	g_signal_connect(G_OBJECT(topwindow),	"destroy", 				G_CALLBACK(destroy),				NULL);

    vbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(topwindow),vbox);

	if(!CreateTerminalWindow())
		return -1;

	gtk_box_pack_start(GTK_BOX(vbox), terminal, TRUE, TRUE, 0);
	gtk_widget_show(terminal);

#ifdef PACKAGE_NAME
	gtk_window_set_role(GTK_WINDOW(topwindow), PACKAGE_NAME "_TOP" );
#else
	gtk_window_set_role(GTK_WINDOW(topwindow), "G3270_TOP" );
#endif

	register_tchange(FULL_SCREEN,set_fullscreen);
	return 0;
 }

