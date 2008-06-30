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
#include <malloc.h>

/*---[ Structures ]----------------------------------------------------------------------------------------*/

 typedef struct _element
 {
 	int ch;
 	int attr;
 } ELEMENT;

/*---[ Prototipes ]----------------------------------------------------------------------------------------*/

 static void title(char *text);
 static void setsize(int rows, int cols);
 static void addch(int row, int col, int c, int attr);
 static void set_charset(char *dcs);
 static void redraw(void);

/*---[ Globals ]-------------------------------------------------------------------------------------------*/

 const struct lib3270_screen_callbacks g3270_screen_callbacks =
 {
	sizeof(struct lib3270_screen_callbacks),

	NULL,			// void (*init)(void);
	setsize,		// void (*setsize)(int rows, int cols);
	addch,			// void (*addch)(int row, int col, int c, int attr);
	set_charset,	// void (*charset)(char *dcs);
	title,			// void (*title)(char *text);
	NULL,			// void (*changed)(int bstart, int bend);
	NULL,			// void (*ring_bell)(void);
	redraw,			// void (*redraw)(void);
	NULL,			// void (*refresh)(void);
	NULL,			// void (*suspend)(void);
	NULL,			// void (*resume)(void);

 };

 static int 		terminal_rows	= 0;
 static int 		terminal_cols	= 0;
 static ELEMENT	*screen			= NULL;

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 static void title(char *text)
 {
 	Trace("Window %p title: \"%s\"",topwindow,text);
 	if(topwindow)
		gtk_window_set_title(GTK_WINDOW(topwindow),text);

 }

 static void setsize(int rows, int cols)
 {
 	if(screen)
 	{
		g_free(screen);
		screen = NULL;
 	}

	if(rows && cols)
	{
		screen = g_malloc(sizeof(ELEMENT) *(rows*cols));
		terminal_rows = rows;
		terminal_cols = cols;
	}

 	Trace("Terminal set to %d rows with %d cols, screen set to %p",rows,cols,screen);

 }

 static void addch(int row, int col, int c, int attr)
 {
 	ELEMENT *el;

 	if(!screen)
		return;

 	el = screen + (row*terminal_cols)+col;

	el->ch = c;
	el->attr = attr;

	// TODO (perry#1#): Update pixmap, queue screen redraw.

 }

 static void redraw(void)
 {
 	DrawScreen(terminal,color,pixmap);
 }

 /**
  * Draw entire buffer.
  *
  * @param	widget	Widget to be used as reference.
  * @param	clr		List of colors to be used when drawing.
  * @param	draw	The image destination.
  *
  */
 int DrawScreen(GtkWidget *widget, GdkColor *clr, GdkDrawable *draw)
 {
	GdkGC		*gc;
	PangoLayout *layout;
	ELEMENT		*el			= screen;
	int 		width;
	int 		height;
	int			x;
	int			y;
	int			left_margin = 0;
	int			top_margin = 0;
	int			row;
	int			col;

	if(!el)
		return -1;

	gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];

	/* Get width/height (assuming it's a fixed size font) */
	layout = gtk_widget_create_pango_layout(widget," ");
	pango_layout_get_pixel_size(layout,&width,&height);

	/* Fill pixmap with background color */
	gdk_drawable_get_size(draw,&width,&height);
	gdk_gc_set_foreground(gc,clr);
	gdk_draw_rectangle(draw,gc,1,0,0,width,height);

	/* Draw screen contens */
	y = top_margin;
	for(row = 0; row < terminal_rows;row++)
	{
		x = left_margin;
		for(col = 0; col < terminal_cols;col++)
		{
			/* Set character attributes in the layout */
			if(el->ch)
			{
				char ch[2];
				ch[0] =  el->ch;
				ch[1] = 0;
				gdk_gc_set_foreground(gc,clr+3);
				pango_layout_set_text(layout,ch,1);
				pango_layout_get_pixel_size(layout,&width,&height);
				gdk_draw_layout(draw,gc,x,y,layout);
			}

			el++;
			x += width;
		}
		y += height;
	}

	g_object_unref(layout);

	return 0;
 }

 static void set_charset(char *dcs)
 {
 	Trace("Screen charset: %s",dcs);
 }

