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

 GtkWidget			*terminal	= NULL;
 GdkPixmap			*pixmap		= NULL;

 static GdkColor	color[QTD_COLORS];


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static gboolean expose(GtkWidget *widget, GdkEventExpose *event, void *t)
 {
    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Event-Structures.html#GdkEventExpose
	GdkGC *gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];

    if(pixmap)
    {
    	// Draw pixmap
		gdk_draw_drawable(widget->window,gc,GDK_DRAWABLE(pixmap),
										event->area.x,event->area.y,
										event->area.x,event->area.y,
										event->area.width,event->area.height);
    }
    else
    {
    	// No pixmap, draw background in event->area
		gdk_gc_set_foreground(gc,color);
		gdk_draw_rectangle(widget->window,gc,1,event->area.x,event->area.y,event->area.width,event->area.height);
    }


	return 0;
 }

 static void configure(GtkWidget *widget, GdkEventConfigure *event, void *t)
 {
	GdkGC *gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];

    Trace("Configuring %p with %dx%d (Window: %p)",widget,event->width,event->height,widget->window);

    if(!widget->window)
		return;

	if(pixmap)
	{
		gdk_pixmap_unref(pixmap);
		pixmap = NULL;
	}

	pixmap = gdk_pixmap_new(widget->window,event->width,event->height,-1);

	gdk_gc_set_foreground(gc,color);
	gdk_draw_rectangle(pixmap,gc,1,0,0,event->width,event->height);

	// TODO (perry#1#): Redraw screen contents in the newly created pixmap.


 }

 static void realize(GtkWidget *widget, void *t)
 {
 	Trace("%p realized (Window: %p)",widget,widget->window);

 }

 static void destroy( GtkWidget *widget, gpointer data)
 {
	if(pixmap)
	{
		gdk_pixmap_unref(pixmap);
		pixmap = NULL;
	}
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

 int CreateTerminalWindow(GtkWidget *box)
 {
 	LoadColors();

	terminal = gtk_drawing_area_new();
	g_signal_connect(G_OBJECT(terminal), "destroy", G_CALLBACK(destroy), NULL);

    GTK_WIDGET_SET_FLAGS(terminal, GTK_CAN_DEFAULT);
    GTK_WIDGET_SET_FLAGS(terminal, GTK_CAN_FOCUS);

    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Events.html#GdkEventMask
    gtk_widget_add_events(terminal,GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_MOTION_MASK|GDK_BUTTON_RELEASE_MASK);

    g_signal_connect(G_OBJECT(terminal), "expose_event",  		G_CALLBACK(expose),		0);
    g_signal_connect(G_OBJECT(terminal), "configure-event",		G_CALLBACK(configure), 	0);
    g_signal_connect(G_OBJECT(terminal), "realize",				G_CALLBACK(realize),	0);

	gtk_box_pack_start(GTK_BOX(box), terminal, TRUE, TRUE, 0);
	gtk_widget_show(terminal);
	return 0;
 }

