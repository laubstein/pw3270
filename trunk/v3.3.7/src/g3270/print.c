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
 #include <errno.h>

 #include "g3270.h"

/*---[ Statics ]------------------------------------------------------------------------------------------------*/


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static int doPrint(GtkPrintOperation *prt, GtkPrintContext *context,cairo_t *cr,gint page)
 {
 	gchar					**text;
	PangoFontDescription	*FontDescr		= (PangoFontDescription *) g_object_get_data(G_OBJECT(prt),"g3270_FontDescr");
	PangoLayout				*FontLayout		= (PangoLayout *) g_object_get_data(G_OBJECT(prt),"g3270_FontLayout");
	int						pg			= 0;
	gdouble					maxHeight 	= gtk_print_context_get_height(context);
	gdouble					current		= 0;
	gdouble 				pos;

	if(!FontDescr)
	{
		FontDescr = pango_font_description_from_string(g_object_get_data(G_OBJECT(prt),"g3270_FontName"));
		g_object_set_data_full(G_OBJECT(prt),"g3270_FontDescr",FontDescr,(void (*)(gpointer)) pango_font_description_free);
	}

	if(!FontLayout)
	{
		Trace("Creating FontLayout to context %p",context);
		FontLayout = gtk_print_context_create_pango_layout(context);
		g_object_set_data_full(G_OBJECT(prt),"g3270_FontLayout",FontLayout,(void (*)(gpointer)) g_object_unref);
		pango_layout_set_font_description(FontLayout,FontDescr);
	}

	for(text = g_object_get_data(G_OBJECT(prt),"g3270_text");*text;text++)
	{
		gint width, height;

		pango_layout_set_text(FontLayout,*text,-1);
		pango_layout_get_pixel_size(FontLayout,&width,&height);

		pos = current;

		if( (current+ ((gdouble) height)) > maxHeight)
		{
			pg++;
			current = 0;
		}
		else
		{
			current += ((gdouble) height);
		}

		if(cr && page == pg)
		{
			cairo_move_to(cr,0,pos);
			pango_cairo_show_layout(cr,FontLayout);
		}
	}

	return pg+1;
 }

 static void draw_page(GtkPrintOperation *prt, GtkPrintContext *context, gint pg, gpointer user_data)
 {
	cairo_t *cr = gtk_print_context_get_cairo_context(context);
	doPrint(prt,context,cr,pg);
 }

 static void begin_print(GtkPrintOperation *prt, GtkPrintContext *context, gpointer user_data)
 {
		int pages = doPrint(prt,context,NULL,-1);
		if(pages <= 0)
		{
			gtk_print_operation_cancel(prt);
			return;
		}
		gtk_print_operation_set_n_pages(prt,pages);
 }

 static void custom_widget_apply(GtkPrintOperation *prt, GtkWidget *font_dialog, gpointer user_data)
 {
 	gchar *font = gtk_font_selection_get_font_name(GTK_FONT_SELECTION(font_dialog));
	if(font)
		g_object_set_data_full(G_OBJECT(prt),"g3270_FontName",font,g_free);
 }

 static void load_font(GtkWidget *widget, GtkPrintOperation *prt)
 {
	gtk_font_selection_set_font_name(GTK_FONT_SELECTION(widget),g_object_get_data(G_OBJECT(prt),"g3270_FontName"));
 }

 static GObject * create_custom_widget(GtkPrintOperation *prt, gpointer user_data)
 {
 	GtkWidget *font_dialog =  gtk_font_selection_new();
    g_signal_connect(font_dialog, "realize", G_CALLBACK(load_font), prt);
 	return G_OBJECT(font_dialog);
 }

 static void print_done(GtkPrintOperation *prt, GtkPrintOperationResult result, gpointer user_data)
 {
	GKeyFile 				*conf		= GetConf();
	gchar					*ptr;
	GtkPrintSettings		*settings	= gtk_print_operation_get_print_settings(prt);
	PangoFontDescription	*FontDescr	= (PangoFontDescription *) g_object_get_data(G_OBJECT(prt),"g3270_FontDescr");
	GtkPageSetup			*setup		= gtk_print_operation_get_default_page_setup(prt);

	if(!conf)
		return;

	if(settings)
		gtk_print_settings_to_key_file(settings,conf,"PrintSettings");

	if(setup)
		gtk_page_setup_to_key_file(setup,NULL,NULL);

	if(FontDescr)
	{
		ptr = pango_font_description_to_string(FontDescr);
		if(ptr)
		{
			g_key_file_set_string(conf,"Print","Font",ptr);
			g_free(ptr);
		}
	}
 }

 int PrintText(const char *name, gchar *text)
 {
	GKeyFile			*conf	= GetConf();
 	GtkPrintOperation	*prt;

 	if(!text)
		return -EINVAL;

 	prt = gtk_print_operation_new();

 	if(!prt)
 		return -1;

	// Set job parameters
	g_object_set_data_full(G_OBJECT(prt),"g3270_text",g_strsplit(g_strchomp(text),"\n",-1),(void (*)(gpointer)) g_strfreev);
	g_object_set_data_full(G_OBJECT(prt),"g3270_FontName",g_strdup(GetString("Print","Font","Courier 10")),g_free);

	// Configure print operation
	gtk_print_operation_set_job_name(prt,name);
	gtk_print_operation_set_allow_async(prt,0);
	gtk_print_operation_set_show_progress(prt,1);

	gtk_print_operation_set_custom_tab_label(prt,_( "Font" ));
	g_signal_connect(prt, "begin-print",    		G_CALLBACK(begin_print), 			0);
    g_signal_connect(prt, "draw-page",      		G_CALLBACK(draw_page),   			0);
	g_signal_connect(prt, "create-custom-widget",   G_CALLBACK(create_custom_widget),	0);
	g_signal_connect(prt, "custom-widget-apply",   	G_CALLBACK(custom_widget_apply),	0);
    g_signal_connect(prt, "done",      				G_CALLBACK(print_done),	 			0);

	if(conf)
	{
		gtk_print_operation_set_print_settings(prt,gtk_print_settings_new_from_key_file(conf,"PrintSettings",NULL));
		gtk_print_operation_set_default_page_setup(prt,gtk_page_setup_new_from_key_file(conf,NULL,NULL));
	}
	else
	{
		gtk_print_operation_set_default_page_setup(prt,gtk_page_setup_new());
		gtk_print_operation_set_print_settings(prt,gtk_print_settings_new());
	}

	// Run Print dialog
	gtk_print_operation_run(prt,GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,GTK_WINDOW(topwindow),NULL);

    g_object_unref(prt);

    return 0;
 }
