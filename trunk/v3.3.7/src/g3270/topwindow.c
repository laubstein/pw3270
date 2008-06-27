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

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 GtkWidget *topwindow = NULL;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 void action_exit(GtkWidget *w, gpointer data)
 {
 	gtk_main_quit();
 }

 static gboolean delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
 {
 	action_exit(widget,0);
    return FALSE;
 }

 gboolean map_event(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
 {
    return 0;
 }

 static void destroy( GtkWidget *widget, gpointer   data )
 {
    gtk_main_quit();
 }

 int CreateTopWindow(void)
 {
 	GtkWidget	*vbox;

	topwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	g_signal_connect(G_OBJECT(topwindow),	"delete_event", 		G_CALLBACK(delete_event),			NULL);
	g_signal_connect(G_OBJECT(topwindow),	"destroy", 				G_CALLBACK(destroy),				NULL);
	g_signal_connect(G_OBJECT(topwindow),	"map-event",			G_CALLBACK(map_event),				NULL);

    vbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(topwindow),vbox);

	if(CreateTerminalWindow(vbox))
		return -1;

#ifdef PACKAGE_NAME
	gtk_window_set_role(GTK_WINDOW(topwindow), PACKAGE_NAME "_TOP" );
#else
	gtk_window_set_role(GTK_WINDOW(topwindow), "G3270_TOP" );
#endif

	return 0;
 }

