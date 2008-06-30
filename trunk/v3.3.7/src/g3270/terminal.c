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
#include <gdk/gdk.h>
#include <string.h>
#include <malloc.h>


/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 GtkWidget				*terminal			= NULL;
 GdkPixmap				*pixmap				= NULL;
 GdkColor				color[QTD_COLORS];
 PangoFontDescription	*font				= NULL;

 static guint 			last_keyval = 0;
 static GtkIMContext	*im;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static GdkPixmap * GetPixmap(GtkWidget *widget)
 {
	GdkPixmap	*pix;
	gint		width;
    gint		height;

	gdk_drawable_get_size(widget->window,&width,&height);
	pix = gdk_pixmap_new(widget->window,width,height,-1);

	// Get the best font size
//	Trace("x1 %p",widget);
//	gtk_widget_modify_font(widget,font);
//	Trace("x2 %p",widget);


	DrawScreen(widget, color, pix);

	return pix;
 }

 static gboolean expose(GtkWidget *widget, GdkEventExpose *event, void *t)
 {
    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Event-Structures.html#GdkEventExpose
    if(!pixmap)
		pixmap = GetPixmap(widget); // No pixmap, get a new one

	gdk_draw_drawable(widget->window,	widget->style->fg_gc[GTK_WIDGET_STATE(widget)],
										GDK_DRAWABLE(pixmap),
										event->area.x,event->area.y,
										event->area.x,event->area.y,
										event->area.width,event->area.height);

	return 0;
 }

 static void configure(GtkWidget *widget, GdkEventConfigure *event, void *t)
 {
//    Trace("Configuring %p with %dx%d (Window: %p)",widget,event->width,event->height,widget->window);

    if(!widget->window)
		return;

	if(pixmap)
	{
		gdk_pixmap_unref(pixmap);
		pixmap = NULL;
	}
 }

 static void destroy( GtkWidget *widget, gpointer data)
 {
	if(pixmap)
	{
		gdk_pixmap_unref(pixmap);
		pixmap = NULL;
	}
 }

 static void realize(GtkWidget *widget, void *t)
 {
    gtk_im_context_set_client_window(im,widget->window);
 }

 static gboolean focus_in(GtkWidget *widget, GdkEventFocus *event, gpointer x)
 {
	gtk_im_context_focus_in(im);
	return 0;
 }

 static gboolean focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer x)
 {
	gtk_im_context_focus_out(im);
	return 0;
 }

 static void im_commit(GtkIMContext *imcontext, gchar *arg1, gpointer user_data)
 {
	Trace("Commit: %s (%02x)", arg1,(unsigned int) *arg1);
 }

 static gboolean key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
	if(gtk_im_context_filter_keypress(im,event))
		return TRUE;

	// Guarda tecla pressionada
	last_keyval = event->keyval;

	return FALSE;
 }

 static gboolean key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {

	if(gtk_im_context_filter_keypress(im,event))
		return TRUE;

	if(KeyboardAction(widget,event,user_data))
	{
		gtk_im_context_reset(im);
		return TRUE;
	}

	return 0;
 }

 int LoadColors(void)
 {
 	static const char *DefaultColors = "black,#00FFFF,red,pink,green1,turquoise,yellow,white,black,DeepSkyBlue,orange,DeepSkyBlue,PaleGreen,PaleTurquoise,grey,white";

 	int 	f;
 	char	*buffer = strdup(DefaultColors);
 	char	*ptr = strtok(buffer,",");

	// FIXME (perry#5#): Load colors from configuration file.
 	for(f=0;ptr && f < QTD_COLORS;f++)
 	{
		gdk_color_parse(ptr,color+f);
		gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),color+f,TRUE,TRUE);
 		ptr = strtok(NULL,",");
 	}

 	free(buffer);

 	return 0;
 }

 GtkWidget *CreateTerminalWindow(void)
 {
 	LoadColors();

	im = gtk_im_context_simple_new();

	terminal = gtk_drawing_area_new();
	g_signal_connect(G_OBJECT(terminal), "destroy", G_CALLBACK(destroy), NULL);

	font = pango_font_description_from_string("Courier 10");
	gtk_widget_modify_font(terminal,font);

    GTK_WIDGET_SET_FLAGS(terminal, GTK_CAN_DEFAULT);
    GTK_WIDGET_SET_FLAGS(terminal, GTK_CAN_FOCUS);

    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Events.html#GdkEventMask
    gtk_widget_add_events(terminal,GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_MOTION_MASK|GDK_BUTTON_RELEASE_MASK);

    g_signal_connect(G_OBJECT(terminal),	"expose_event",  		G_CALLBACK(expose),			0);
    g_signal_connect(G_OBJECT(terminal),	"configure-event",		G_CALLBACK(configure),		0);
    g_signal_connect(G_OBJECT(terminal),	"key-press-event",		G_CALLBACK(key_press),		0);
    g_signal_connect(G_OBJECT(terminal),	"key-release-event",	G_CALLBACK(key_release),	0);
    g_signal_connect(G_OBJECT(terminal),	"realize",				G_CALLBACK(realize),		0);
    g_signal_connect(G_OBJECT(terminal),	"focus-in-event",		G_CALLBACK(focus_in),		0);
    g_signal_connect(G_OBJECT(terminal),	"focus-out-event",		G_CALLBACK(focus_out),		0);
    g_signal_connect(G_OBJECT(im),			"commit",				G_CALLBACK(im_commit),		0);

	return terminal;
 }

