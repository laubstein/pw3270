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
#include <gdk/gdk.h>
#include <string.h>
#include <malloc.h>

// TODO (perry#7#): Find a better way to get font sizes!!!
#define MAX_FONT_SIZES	54

/*---[ Structs ]------------------------------------------------------------------------------------------------*/

 typedef struct _fontsize
 {
 	int 	size;
 	int 	width;
	int 	height;
 } FONTSIZE;

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 GtkWidget				*terminal			= NULL;
 GdkPixmap				*pixmap				= NULL;
 GdkColor				color[TERMINAL_COLOR_COUNT+1];
 gint					cMode				= CURSOR_MODE_ENABLED|CURSOR_MODE_BASE|CURSOR_MODE_SHOW;
 gint					cCol				= 0;
 gint					cRow				= 0;

 static GdkRectangle	cursor;
 static gint			sWidth				= 0;
 static gint			sHeight				= 0;
 static int			blink_enabled		= 0;

 static guint 			last_keyval = 0;
 static GtkIMContext	*im;

 static PangoFontDescription	*font					= NULL;
 static FONTSIZE				fsize[MAX_FONT_SIZES];


/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

 static void RedrawCursor(void);
 static void set_showcursor(int value, int reason);

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static GdkPixmap * GetPixmap(GtkWidget *widget)
 {
	GdkPixmap	*pix;
	gdk_drawable_get_size(widget->window,&sWidth,&sHeight);
	pix = gdk_pixmap_new(widget->window,sWidth,sHeight,-1);
	DrawScreen(widget, color, pix);
	DrawOIA(widget,color,pix);
	RedrawCursor();
	return pix;
 }

 static gboolean expose(GtkWidget *widget, GdkEventExpose *event, void *t)
 {
    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Event-Structures.html#GdkEventExpose
	GdkGC *gc = gdk_gc_new(widget->window);

    if(!pixmap)
		pixmap = GetPixmap(widget); // No pixmap, get a new one

	gdk_draw_drawable(widget->window,	gc,
										GDK_DRAWABLE(pixmap),
										event->area.x,event->area.y,
										event->area.x,event->area.y,
										event->area.width,event->area.height);

	if((cMode & CURSOR_MODE_ENABLED) && GTK_WIDGET_HAS_FOCUS(widget))
	{
		// Draw cursor

		if(cMode & CURSOR_MODE_CROSS)
		{
			// Draw cross-hair cursor
			gdk_gc_set_foreground(gc,color+TERMINAL_COLOR_CROSS_HAIR);
			gdk_draw_line(widget->window,gc,cursor.x,0,cursor.x,OIAROW);
			gdk_draw_line(widget->window,gc,0,cursor.y,sWidth,cursor.y);
		}

		if( (cMode & (CURSOR_MODE_BASE|CURSOR_MODE_SHOW)) == (CURSOR_MODE_BASE|CURSOR_MODE_SHOW) )
		{
			// Draw standard cursor
			gdk_gc_set_foreground(gc,color+TERMINAL_COLOR_CURSOR);
			gdk_draw_rectangle(widget->window,gc,TRUE,cursor.x,cursor.y-cursor.height,cursor.width,cursor.height);
		}
	}

	gdk_gc_destroy(gc);
	return 0;
 }

 /**
  * Cache font sizes.
  *
  * Get the font metrics for the selected size, cache it for speed up terminal resizing.
  *
  * @param sel		The font size.
  *
  */
 static void SetFontSize(int sel)
 {
	PangoLayout *layout;

 	if(fsize[sel].size)
 	{
		pango_font_description_set_size(font,fsize[sel].size);
		gtk_widget_modify_font(terminal,font);
		return;
 	}

	// TODO (perry#9#): Fint a better and faster way to get the character size.
	fsize[sel].size = (sel+1) * PANGO_SCALE;
	pango_font_description_set_size(font,fsize[sel].size);

	gtk_widget_modify_font(terminal,font);
	layout = gtk_widget_create_pango_layout(terminal,"A");

	pango_layout_get_pixel_size(layout,&fsize[sel].width,&fsize[sel].height);

	g_object_unref(layout);

 }

 static int lWidth = -1;
 static int lHeight = -1;
 static int lFont = -1;

 static void configure(GtkWidget *widget, GdkEventConfigure *event, void *t)
 {
 	int 		f;
	int 		left	= left_margin;
	int 		top		= top_margin;;
	GdkPixmap	*pix;
	GdkGC		*gc;

    if(!widget->window)
		return;

	if(lWidth == event->width && lHeight == event->height && lFont > 0)
		return;

	lWidth = event->width;
	lHeight = event->height;

    Trace("Configuring %p with %dx%d (Window: %p Realized: %d)",widget,event->width,event->height,widget->window,(int) GTK_WIDGET_REALIZED(widget));

	if(lFont == -1)
	{
		/* Font all font sizes */
		lFont = -2;
		Trace("Loading font sizes from 0 to %d",MAX_FONT_SIZES);
		for(f=0;f<MAX_FONT_SIZES;f++)
		{
			SetFontSize(f);
			Trace("Font %d fits on %dx%d",f,terminal_rows*fsize[f].height,terminal_cols*fsize[f].width);
		}
//		gtk_widget_set_size_request(widget,fsize->width,fsize->height);
	}

	/* Get the best font for the current window size */
	for(f=0;f<MAX_FONT_SIZES && (fsize[f].height*(terminal_rows+1)) < event->height && (fsize[f].width*terminal_cols) < event->width;f++);

	if(f >= MAX_FONT_SIZES)
		f = (MAX_FONT_SIZES-1);
	else if(f > 0)
		f--;

	if(f != lFont)
	{
		Trace("Font size changes from %d to %d (%dx%d)",lFont,f,terminal_cols*fsize[f].width,terminal_rows*fsize[f].height);
		lFont = f;
		pango_font_description_set_size(font,fsize[f].size);
		gtk_widget_modify_font(terminal,font);

		if(pixmap)
		{
			/* Font size, invalidate entire image */
			gdk_pixmap_unref(pixmap);
			pixmap = NULL;
		}

	}

	/* Center image */
	left_margin = (event->width >> 1) - ((terminal_cols * fsize[f].width) >> 1);
	if(left_margin < 0)
		left_margin = 0;

	top_margin = (event->height >> 1) - (((terminal_rows+1) * fsize[f].height) >> 1);
	if(top_margin < 0)
		top_margin = 0;

	/* No pixmap, will create a new one in the next expose */
	if(!pixmap)
		return;

	/* Font size hasn't changed, rebuild pixmap using the saved image */
	pix = gdk_pixmap_new(widget->window,event->width,event->height,-1);
	gc = gdk_gc_new(pix);

	gdk_gc_set_foreground(gc,color);
	gdk_draw_rectangle(pix,gc,1,0,0,event->width,event->height);

	gdk_draw_drawable(pix,gc,pixmap,
								left,top,
								left_margin,top_margin,
								(terminal_cols * fsize[lFont].width),OIAROW+1+fsize[lFont].height);

	gdk_gc_destroy(gc);

	/* Set the new pixmap */
	gdk_pixmap_unref(pixmap);
	pixmap = pix;
	RedrawCursor();
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
    Trace("Realizing %p",widget);

	// Configure im context
    gtk_im_context_set_client_window(im,widget->window);
    gdk_window_set_cursor(widget->window,wCursor[0]);
    LoadImages(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE(widget)]);
 }

 static gboolean focus_in(GtkWidget *widget, GdkEventFocus *event, gpointer x)
 {
	gtk_im_context_focus_in(im);
	InvalidateCursor();
	return 0;
 }

 static gboolean focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer x)
 {
	gtk_im_context_focus_out(im);
	InvalidateCursor();
	return 0;
 }

 static void im_commit(GtkIMContext *imcontext, gchar *arg1, gpointer user_data)
 {
	ParseInput(arg1);
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
 	static const char *DefaultColors =	"black,"
											"#00FFFF,"
											"red,"
											"pink,"
											"green1,"
											"turquoise,"
											"yellow,"
											"white,"
											"black,"
											"DeepSkyBlue,"
											"orange,"
											"DeepSkyBlue,"
											"PaleGreen,"
											"PaleTurquoise,"
											"grey,"
											"white,"
											"green," 		// TERMINAL_COLOR_CURSOR
											"green," 		// TERMINAL_COLOR_CROSS_HAIR
											"green," 		// TERMINAL_COLOR_OIA
											"black,"	 	// TERMINAL_COLOR_OIA_BACKGROUND
											"white,"		// TERMINAL_COLOR_OIA_STATUS_OK
											"red";			// TERMINAL_COLOR_OIA_STATUS_INVALID

 	int 	f;

	// FIXME (perry#9#): Load colors from configuration file.
 	char	*buffer = strdup(DefaultColors);

 	char	*ptr = strtok(buffer,",");

 	for(f=0;ptr && f < TERMINAL_COLOR_COUNT;f++)
 	{
 		if(ptr)
 		{
			gdk_color_parse(ptr,color+f);
			ptr = strtok(NULL,",");
 		}
		else
		{
			gdk_color_parse("green",color+f);
		}
		gdk_colormap_alloc_color(gtk_widget_get_default_colormap(),color+f,TRUE,TRUE);
 	}

 	free(buffer);

 	return 0;
 }

 static void set_crosshair(int value, int reason)
 {
 	if(value)
		cMode |= CURSOR_MODE_CROSS;
	else
		cMode &= ~CURSOR_MODE_CROSS;
	gtk_widget_queue_draw(terminal);
 }

 static gboolean blink_cursor(gpointer ptr)
 {
 	if(!blink_enabled)
		return FALSE;

 	if(cMode & CURSOR_MODE_ENABLED)
 	{
		cMode ^= CURSOR_MODE_SHOW;
		InvalidateCursor();
 	}
	return TRUE;
 }


 static void set_blink(int value, int reason)
 {
 	if(value == blink_enabled)
		return;

	blink_enabled = value;

	if(value)
		g_timeout_add((guint) 750, (GSourceFunc) blink_cursor, 0);
	else
		cMode |= CURSOR_MODE_SHOW;


	gtk_widget_queue_draw(terminal);
 }

 GtkWidget *CreateTerminalWindow(void)
 {
	memset(fsize,0,MAX_FONT_SIZES * sizeof(FONTSIZE));

 	LoadColors();

	im = gtk_im_context_simple_new();

	terminal = gtk_drawing_area_new();
	g_signal_connect(G_OBJECT(terminal), "destroy", G_CALLBACK(destroy), NULL);


	// Configure terminal widget
    GTK_WIDGET_SET_FLAGS(terminal, GTK_CAN_DEFAULT|GTK_CAN_FOCUS);

    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Events.html#GdkEventMask
    gtk_widget_add_events(terminal,GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_MOTION_MASK|GDK_BUTTON_RELEASE_MASK);

    g_signal_connect(G_OBJECT(terminal),	"expose_event",  		G_CALLBACK(expose),					0);
    g_signal_connect(G_OBJECT(terminal),	"configure-event",		G_CALLBACK(configure),				0);
    g_signal_connect(G_OBJECT(terminal),	"key-press-event",		G_CALLBACK(key_press),				0);
    g_signal_connect(G_OBJECT(terminal),	"key-release-event",	G_CALLBACK(key_release),			0);
    g_signal_connect(G_OBJECT(terminal),	"realize",				G_CALLBACK(realize),				0);
    g_signal_connect(G_OBJECT(terminal),	"focus-in-event",		G_CALLBACK(focus_in),				0);
    g_signal_connect(G_OBJECT(terminal),	"focus-out-event",		G_CALLBACK(focus_out),				0);
    g_signal_connect(G_OBJECT(im),			"commit",				G_CALLBACK(im_commit),				0);

    // Connect mouse events
    g_signal_connect(G_OBJECT(terminal), "button-press-event",		G_CALLBACK(mouse_button_press),		0);
    g_signal_connect(G_OBJECT(terminal), "button-release-event",	G_CALLBACK(mouse_button_release),	0);
    g_signal_connect(G_OBJECT(terminal), "motion-notify-event",		G_CALLBACK(mouse_motion),			0);
    g_signal_connect(G_OBJECT(terminal), "scroll-event",			G_CALLBACK(mouse_scroll),			0);

	font = pango_font_description_from_string("Courier");

	register_tchange(CROSSHAIR,set_crosshair);
	register_tchange(CURSOR_POS,set_showcursor);
	register_tchange(CURSOR_BLINK,set_blink);

	return terminal;
 }

 void InvalidateCursor(void)
 {
	gtk_widget_queue_draw_area(terminal,cursor.x,cursor.y-cursor.height,cursor.width,cursor.height);
	if(cMode & CURSOR_MODE_CROSS)
	{
		gtk_widget_queue_draw_area(terminal,0,cursor.y,sWidth,cursor.height);
		gtk_widget_queue_draw_area(terminal,cursor.x,0,cursor.width,sHeight);
	}
 }

 static void DrawCursorPosition(void)
 {
	GdkGC 		*gc		= gdk_gc_new(terminal->window);
	PangoLayout *layout;
	int			x		= left_margin+(fWidth*(terminal_cols-7));
	char		buffer[10];

	gdk_gc_set_foreground(gc,color+TERMINAL_COLOR_OIA_BACKGROUND);
	gdk_draw_rectangle(pixmap,gc,1,x,OIAROW+1,fWidth*7,fHeight);

	layout = gtk_widget_create_pango_layout(terminal,"4");

	sprintf(buffer,"%03d/%03d",cRow,cCol);
	pango_layout_set_text(layout,buffer,-1);

	gdk_draw_layout_with_colors(pixmap,gc,
						x,OIAROW+1,
						layout,
						color+TERMINAL_COLOR_OIA_CURSOR,color+TERMINAL_COLOR_OIA_BACKGROUND);

	g_object_unref(layout);
	gdk_gc_destroy(gc);
	gtk_widget_queue_draw_area(terminal,x,OIAROW+1,fWidth*7,fHeight);

 }

 static void set_showcursor(int value, int reason)
 {
 	if(!(terminal && pixmap))
		return;

	if(value)
	{
		DrawCursorPosition();
	}
	else
	{
		GdkGC *gc	= gdk_gc_new(terminal->window);
		int   x		= left_margin+(fWidth*(terminal_cols-7));

		gdk_gc_set_foreground(gc,color+TERMINAL_COLOR_OIA_BACKGROUND);
		gdk_draw_rectangle(pixmap,gc,1,x,OIAROW+1,fWidth*7,fHeight);

		gdk_gc_destroy(gc);
		gtk_widget_queue_draw_area(terminal,x,OIAROW+1,fWidth*7,fHeight);

	}

	gtk_widget_queue_draw(terminal);
 }

 void MoveCursor(int row, int col)
 {
	if(row == cRow && col == cCol)
		return;

	Trace("Moving cursor to %d,%d",row,col);

	InvalidateCursor();

	cCol			= col;
	cRow			= row;

	RedrawCursor();

	if(Toggled(CURSOR_POS) && terminal && pixmap)
		DrawCursorPosition();

 }

 static void RedrawCursor(void)
 {
	memset(&cursor,0,sizeof(cursor));

	// Set cursor position
	cursor.x 		= left_margin + (cCol * fWidth);
	cursor.y 		= top_margin + ((cRow+1) * fHeight);
	cursor.width 	= fWidth;
	cursor.height 	= (fHeight >> 2)+1;

	// Mark to redraw
	InvalidateCursor();
 }

