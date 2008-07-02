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
#include <string.h>
#include <lib3270/localdefs.h>

/*---[ Structures ]----------------------------------------------------------------------------------------*/

 #define MAX_CHR_LENGTH 3

 typedef struct _element
 {
 	gchar	ch[MAX_CHR_LENGTH];
 	short	fg;
 	short	bg;
 } ELEMENT;

/*---[ Prototipes ]----------------------------------------------------------------------------------------*/

 static void title(char *text);
 static void setsize(int rows, int cols);
 static void addch(int row, int col, int c, unsigned short attr);
 static void set_charset(char *dcs);
 static void redraw(void);
 static void reset(int lock, const char *msg);
 static void status(STATUS_CODE id);

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
	reset,			// void (*reset)(int lock, const char *msg);
	status,			// void (*status)(STATUS_CODE id);
	NULL,			// void (*typeahead)(int on);
	NULL,			// void (*printer)(int on);
	NULL,			// void (*compose)(int on, unsigned char c, int keytype);
	NULL,			// void (*oia_flag)(OIA_FLAG id, int on);


 };

 int 				terminal_rows	= 0;
 int 				terminal_cols	= 0;
 int				left_margin		= 0;
 int				top_margin		= 0;

 static ELEMENT	*screen			= NULL;
 static char		*charset		= NULL;

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 static void title(char *text)
 {
 	Trace("Window %p title: \"%s\"",topwindow,text);
 	if(topwindow)
		gtk_window_set_title(GTK_WINDOW(topwindow),text);

 }

 static void addch(int row, int col, int c, unsigned short attr)
 {
	gchar	in[2] = { (char) c, 0 };
	gsize	sz;
	gchar	*ch;
	ELEMENT temp;
 	ELEMENT *el;

 	if(!screen)
		return;

	memset(&temp,0,sizeof(temp));

	if(c)
	{
		if(charset)
		{
			ch = g_convert(in, -1, "UTF-8", charset, NULL, &sz, NULL);

			if(sz < MAX_CHR_LENGTH)
			{
				memcpy(temp.ch,ch,sz);
			}
			else
			{
				Log("Invalid size when converting \"%s\" to \"%s\"",in,ch);
				memset(temp.ch,0,MAX_CHR_LENGTH);
			}
			g_free(ch);

		}
		else
		{
			memcpy(temp.ch,in,2);
		}

	}
	else
	{
		memset(temp.ch,0,MAX_CHR_LENGTH);
	}

	// TODO (perry#1#): Get the correct colors
	temp.bg = (attr & 0xF0) >> 4;
	temp.fg = (attr & 0x0F);

	// Get element entry in the buffer, update ONLY if changed
 	el = screen + (row*terminal_cols)+col;

	if(!memcmp(el,&temp,sizeof(ELEMENT)))
		return;

	memcpy(el,&temp,sizeof(ELEMENT));

	// Update pixmap, queue screen redraw.
	if(terminal && pixmap)
	{
		gint x, y, width,height;
		PangoLayout *layout = gtk_widget_create_pango_layout(terminal," ");

		pango_layout_set_text(layout,el->ch,-1);
		pango_layout_get_pixel_size(layout,&width,&height);

		x = left_margin + (col * width);
		y = top_margin + (row * height);

		gdk_draw_layout_with_colors(	pixmap,
										terminal->style->fg_gc[GTK_WIDGET_STATE(terminal)],
										x,y,
										layout,
										color+el->fg,
										color+el->bg );

		g_object_unref(layout);

		gtk_widget_queue_draw_area(terminal,x,y,width,height);
	}

 }

 static void setsize(int rows, int cols)
 {
	g_free(screen);
	screen = NULL;

	if(rows && cols)
	{
		screen = g_new0(ELEMENT,(rows*cols));
		terminal_rows = rows;
		terminal_cols = cols;
	}

 	Trace("Terminal set to %d rows with %d cols, screen set to %p",rows,cols,screen);

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
	int			row;
	int			col;

	if(!el)
		return -1;


	gc = widget->style->fg_gc[GTK_WIDGET_STATE(widget)];

	// Fill pixmap with background color
	gdk_drawable_get_size(draw,&width,&height);
	gdk_gc_set_foreground(gc,clr);
	gdk_draw_rectangle(draw,gc,1,0,0,width,height);

	// Draw screen contens
	layout = gtk_widget_create_pango_layout(widget," ");
	pango_layout_get_pixel_size(layout,&width,&height);

	y = top_margin;
	for(row = 0; row < terminal_rows;row++)
	{
		x = left_margin;
		for(col = 0; col < terminal_cols;col++)
		{
			// Set character attributes in the layout
			if(el->ch)
			{
				pango_layout_set_text(layout,el->ch,-1);
				pango_layout_get_pixel_size(layout,&width,&height);
				gdk_draw_layout_with_colors(draw,gc,x,y,layout,clr+el->fg,clr+el->bg);
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
	g_free(charset);
	charset = g_strdup(dcs);
 }

 static void reset(int lock, const char *msg)
 {
 	Trace("Reset Lock: %d Msg: \"%s\"",lock,msg);
 }

 static void status(STATUS_CODE id)
 {
#ifdef DEBUG
	static const char *status_code[STATUS_CODE_USER] =
		{
			"STATUS_CODE_BLANK",
			"STATUS_CODE_SYSWAIT",
			"STATUS_CODE_TWAIT",
			"STATUS_CODE_CONNECTED",
			"STATUS_CODE_DISCONNECTED",
			"STATUS_CODE_AWAITING_FIRST",
			"STATUS_CODE_MINUS",
			"STATUS_CODE_PROTECTED",
			"STATUS_CODE_NUMERIC",
			"STATUS_CODE_OVERFLOW",
			"STATUS_CODE_INHIBIT",
			"STATUS_CODE_X",
		};

#endif

	if(id >= STATUS_CODE_USER)
	{
		Log("Unexpected status code %d",(int) id);
		return;
	}

	Trace("Status changed to \"%s\"",status_code[id]);

 }
