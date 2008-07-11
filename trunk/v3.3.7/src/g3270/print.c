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

 #include "config.h"
 #include <globals.h>

 #include "g3270.h"

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *font_dialog, gpointer user_data)
 {
 	gchar *font = gtk_font_selection_get_font_name(GTK_FONT_SELECTION(font_dialog));
	if(font)
		g_object_set_data_full(G_OBJECT(prt),"g3270_font",font,g_free);
 }

 static void load_font(GtkWidget *widget, GtkPrintOperation *prt)
 {
	gtk_font_selection_set_font_name(GTK_FONT_SELECTION(widget),g_object_get_data(G_OBJECT(prt),"g3270_font"));
 }

 static GObject * create_custom_widget(GtkPrintOperation *prt, gpointer user_data)
 {
 	GtkWidget *font_dialog =  gtk_font_selection_new();
    g_signal_connect(font_dialog, "realize", G_CALLBACK(load_font), prt);
 	return G_OBJECT(font_dialog);
 }

 int PrintText(const char *name, gchar *text)
 {
 	GtkPrintOperation *prt = gtk_print_operation_new();

 	if(!prt)
 		return -1;

	// Set job parameters
	g_object_set_data_full(G_OBJECT(prt),"g3270_text",g_strchomp(text),g_free);
	g_object_set_data_full(G_OBJECT(prt),"g3270_font",g_strdup("Courier 12"),g_free);

	// Configure print operation
	gtk_print_operation_set_job_name(prt,name);
	gtk_print_operation_set_allow_async(prt,0);
	gtk_print_operation_set_show_progress(prt,1);

	gtk_print_operation_set_custom_tab_label(prt,_( "Font" ));
//	g_signal_connect(prt, "begin-print",    		G_CALLBACK(begin_print), 			0);
//    g_signal_connect(prt, "draw-page",      		G_CALLBACK(draw_page),   			0);
//    g_signal_connect(prt, "done",      				G_CALLBACK(print_done),	 			0);
	g_signal_connect(prt, "create-custom-widget",   G_CALLBACK(create_custom_widget),	0);
	g_signal_connect(prt, "custom-widget-apply",   	G_CALLBACK(custom_widget_apply),	0);


	gtk_print_operation_set_default_page_setup(prt,gtk_page_setup_new());
	gtk_print_operation_set_print_settings(prt,gtk_print_settings_new());


	// Run Print dialog
	gtk_print_operation_run(prt,GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,GTK_WINDOW(topwindow),NULL);

    g_object_unref(prt);

    return 0;
 }
