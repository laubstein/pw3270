/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como terminal.c e possui 636 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

#include "gui.h"
#include "fonts.h"

#include <lib3270/toggle.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define KEYBOARD_STATE_MASK GDK_SHIFT_MASK
// |GDK_ALT_MASK|GDK_LOCK_MASK)

/*---[ Structs ]------------------------------------------------------------------------------------------------*/


/*---[ Globals ]------------------------------------------------------------------------------------------------*/

// GdkColor						  color[TERMINAL_COLOR_COUNT+1];

 GtkWidget						* terminal				= NULL;
 GdkPixmap						* pixmap_terminal		= NULL;

 GtkIMContext					* input_method			= NULL;

 // Cursor info
 gint							cMode					= CURSOR_MODE_ENABLED|CURSOR_MODE_BASE|CURSOR_MODE_SHOW;
 gint							cCol					= 0;
 gint							cRow					= 0;
// static GdkPixmap				*pCursor				= NULL;
 static GdkRectangle			rCursor;
 static int					blink_enabled			= 0;

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

 static void set_showcursor(int value, enum toggle_type reason);
 static void set_blink(int value, enum toggle_type reason);

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static void release_pixmaps(void)
 {
	if(pixmap_terminal)
	{
		gdk_pixmap_unref(pixmap_terminal);
		pixmap_terminal = NULL;
	}

 }

 static void draw_cursor(void)
 {
 	if(!valid_terminal_window())
		return;

	rCursor.x 		= left_margin + (cCol * fontWidth);
	rCursor.y 		= top_margin + (cRow * fontHeight);
	rCursor.width 	= fontWidth;

	Trace("Cursor at %d,%d",rCursor.x,rCursor.y);

	gtk_im_context_set_cursor_location(input_method,&rCursor);

//	if(cMode & (CURSOR_MODE_ENABLED|CURSOR_MODE_CROSS) == (CURSOR_MODE_ENABLED|CURSOR_MODE_CROSS))
	{
		int	 width;
		int	 height;

		gdk_drawable_get_size(terminal->window,&width,&height);

		gtk_widget_queue_draw_area(terminal,0,rCursor.y+fontAscent,width,1);
		gtk_widget_queue_draw_area(terminal,rCursor.x,0,1,height);

	}

 }



 static gboolean expose(GtkWidget *widget, GdkEventExpose *event, void *t)
 {
	GdkWindow *window = gtk_widget_get_window(widget);
 	cairo_t *cr	= gdk_cairo_create(window);

 	if(!pixmap_terminal)
 	{
 		// Rebuild pixmap
 		gint width;
 		gint height;
		gdk_drawable_get_size(window,&width,&height);
		pixmap_terminal = gdk_pixmap_new(window,width,height,-1);
		update_terminal_contents();
 	}

    gdk_cairo_set_source_pixmap(cr, pixmap_terminal, 0, 0);
    gdk_cairo_rectangle(cr, &event->area);
    cairo_fill(cr);

    cairo_destroy(cr);

//	if(cMode & (CURSOR_MODE_ENABLED|CURSOR_MODE_CROSS) == (CURSOR_MODE_ENABLED|CURSOR_MODE_CROSS))
	{
		// Draw cross-hair cursor
		int			width;
		int			height;
		GtkStyle	*style	= gtk_widget_get_style(widget);
		GdkGC 		*gc		= style->fg_gc[GTK_STATE_NORMAL];

		gdk_gc_set_foreground(gc,color+TERMINAL_COLOR_CROSS_HAIR);
		gdk_drawable_get_size(widget->window,&width,&height);

		gdk_draw_line(widget->window,gc,rCursor.x,0,rCursor.x,OIAROW-1);
		gdk_draw_line(widget->window,gc,0,rCursor.y+fontAscent,width,rCursor.y+fontAscent);

	}

/*
    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Event-Structures.html#GdkEventExpose
	GdkGC *gc;

	if(!widget->window)
		return 0;

	gc = getCachedGC(widget->window);

    if(!pixmap)
		pixmap = GetPixmap(widget); // No pixmap, get a new one

	gdk_draw_drawable(widget->window,	gc,
										GDK_DRAWABLE(pixmap),
										event->area.x,event->area.y,
										event->area.x,event->area.y,
										event->area.width,event->area.height);

	if((cMode & CURSOR_MODE_ENABLED))
	{
		// Draw cursor

		if(cMode & CURSOR_MODE_CROSS)
		{
			// Draw cross-hair cursor
			gdk_gc_set_foreground(gc,color+TERMINAL_COLOR_CROSS_HAIR);
			gdk_draw_line(widget->window,gc,rCursor.x,0,rCursor.x,OIAROW-1);
			gdk_draw_line(widget->window,gc,0,rCursor.y+fontHeight,sWidth,rCursor.y+fontHeight);
		}

		if( (cMode & (CURSOR_MODE_BASE|CURSOR_MODE_SHOW)) == (CURSOR_MODE_BASE|CURSOR_MODE_SHOW) )
		{
			// Draw standard cursor
			gdk_draw_drawable(		widget->window,	gc,
									GDK_DRAWABLE(pCursor),
									0,rCursor.height,
									rCursor.x,rCursor.y+rCursor.height,
									rCursor.width,fontHeight-rCursor.height);

		}
	}
*/

	return 0;
 }

/**
 * Update font size & margins according to new terminal size.
 *
 * @param widget	Terminal widget.
 * @param width	New terminal width.
 * @param height	New terminal height.
 *
 */
 static void ResizeTerminal(GtkWidget *widget, gint width, gint height)
 {
 	static gint lastWidth  = -1;
 	static gint lastHeight = -1;

	if(lastWidth == width && lastHeight == height)
		return;

	lastWidth  = width;
	lastHeight = height;

 	update_screen_size(widget, width, height);

	// Invalidate pixmaps, will redraw on next expose
	release_pixmaps();

 }

 static void configure(GtkWidget *widget, GdkEventConfigure *event, void *t)
 {
    if(widget->window)
		ResizeTerminal(widget,event->width,event->height);
 }

 static void destroy( GtkWidget *widget, gpointer data)
 {
 	Trace("%s","Destroying terminal");
 	terminal = NULL;

	release_pixmaps();

	if(fontFace)
	{
		cairo_font_face_destroy(fontFace);
		fontFace = NULL;
	}

 	Trace("%s","Terminal destroyed");

 }

 static void realize(GtkWidget *widget, void *t)
 {
 	gint width, height;

    Trace("Realizing %p",widget);

	// Configure im context
    gtk_im_context_set_client_window(input_method,widget->window);

#ifdef MOUSE_POINTER_CHANGE
    gdk_window_set_cursor(widget->window,wCursor[0]);
#endif

	init_terminal_font(widget);

	#warning Work in progress
/*
    // Load images
    LoadImages(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE(widget)]);
*/

	// Set terminal size
	gdk_drawable_get_size(widget->window,&width,&height);

	ResizeTerminal(widget, width, height);


 }

 static gboolean focus_in(GtkWidget *widget, GdkEventFocus *event, gpointer x)
 {
	gtk_im_context_focus_in(input_method);
//	InvalidateCursor();
	return 0;
 }

 static gboolean focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer x)
 {
	gtk_im_context_focus_out(input_method);
//	InvalidateCursor();
	return 0;
 }

 static void im_commit(GtkIMContext *imcontext, gchar *arg1, gpointer user_data)
 {
	ParseInput(arg1);
 }

 static gboolean key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
	UpdateKeyboardState(event->state & KEYBOARD_STATE_MASK);

	if(gtk_im_context_filter_keypress(input_method,event))
		return TRUE;

	if(KeyboardAction(widget,event,user_data))
	{
		gtk_im_context_reset(input_method);
		return TRUE;
	}

	return FALSE;
 }

 static gboolean key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
	UpdateKeyboardState(event->state & KEYBOARD_STATE_MASK);

	if(gtk_im_context_filter_keypress(input_method,event))
		return TRUE;

	return FALSE;
 }

 static void set_crosshair(int value, enum toggle_type reason)
 {
 	if(value)
		cMode |= CURSOR_MODE_CROSS;
	else
		cMode &= ~CURSOR_MODE_CROSS;
 }

 static gboolean blink_cursor(gpointer ptr)
 {
 	if(!blink_enabled)
		return FALSE;

 	if(cMode & CURSOR_MODE_ENABLED)
 	{
 		if(GTK_WIDGET_HAS_FOCUS(terminal))
			cMode ^= CURSOR_MODE_SHOW;
		else
			cMode |= CURSOR_MODE_SHOW;
//		InvalidateCursor();
 	}
	return TRUE;
 }


 static void set_blink(int value, enum toggle_type reason)
 {
 	if(value == blink_enabled)
		return;

	blink_enabled = value;

	if(value)
		g_timeout_add((guint) 750, (GSourceFunc) blink_cursor, 0);
	else
		cMode |= CURSOR_MODE_SHOW;

//	InvalidateCursor();
 }

 static void set_insert(int value, enum toggle_type reason)
 {
 	#warning Work in progress

/*
	DrawOIA(pixmap,color);

	if(terminal)
		gtk_widget_queue_draw(terminal);

 	RedrawCursor();
*/
 }

 static void size_allocate(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
 {
 	Trace("Terminal changes to %dx%d pixels",allocation->width,allocation->height);
    if(widget->window)
		ResizeTerminal(widget,allocation->width,allocation->height);
 }

 GtkWidget *CreateTerminalWindow(void)
 {
	input_method = gtk_im_multicontext_new();

	terminal = gtk_event_box_new();
	gtk_widget_set_app_paintable(terminal,TRUE);
	gtk_widget_set_redraw_on_allocate(terminal,TRUE);

	g_signal_connect(G_OBJECT(terminal), "destroy", 			G_CALLBACK(destroy), NULL);

/*
	g_signal_connect(G_OBJECT(terminal), "direction-changed",	G_CALLBACK(direction_changed), NULL);
	g_signal_connect(G_OBJECT(terminal), "style-set",			G_CALLBACK(style_set), NULL);
*/

	// Configure terminal widget
    GTK_WIDGET_SET_FLAGS(terminal, GTK_CAN_DEFAULT|GTK_CAN_FOCUS);

    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Events.html#GdkEventMask
    gtk_widget_add_events(terminal,GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_MOTION_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK|GDK_ENTER_NOTIFY_MASK);

    g_signal_connect(G_OBJECT(terminal),		"expose_event",  		G_CALLBACK(expose),					0);
    g_signal_connect(G_OBJECT(terminal),		"configure-event",		G_CALLBACK(configure),				0);
    g_signal_connect(G_OBJECT(terminal),		"size-allocate",		G_CALLBACK(size_allocate),			0);
    g_signal_connect(G_OBJECT(terminal),		"key-press-event",		G_CALLBACK(key_press),				0);
    g_signal_connect(G_OBJECT(terminal),		"key-release-event",	G_CALLBACK(key_release),			0);
    g_signal_connect(G_OBJECT(terminal),		"realize",				G_CALLBACK(realize),				0);
    g_signal_connect(G_OBJECT(terminal),		"focus-in-event",		G_CALLBACK(focus_in),				0);
    g_signal_connect(G_OBJECT(terminal),		"focus-out-event",		G_CALLBACK(focus_out),				0);
    g_signal_connect(G_OBJECT(input_method),	"commit",				G_CALLBACK(im_commit),				0);

    // Connect mouse events
    g_signal_connect(G_OBJECT(terminal), 		"button-press-event",	G_CALLBACK(mouse_button_press),		0);
    g_signal_connect(G_OBJECT(terminal), 		"button-release-event",	G_CALLBACK(mouse_button_release),	0);
    g_signal_connect(G_OBJECT(terminal), 		"motion-notify-event",	G_CALLBACK(mouse_motion),    		0);
    g_signal_connect(G_OBJECT(terminal), 		"enter-notify-event",	G_CALLBACK(mouse_enter),    		0);
    g_signal_connect(G_OBJECT(terminal), 		"scroll-event",			G_CALLBACK(mouse_scroll),			0);

	register_tchange(CROSSHAIR,set_crosshair);
	register_tchange(CURSOR_POS,set_showcursor);
	register_tchange(CURSOR_BLINK,set_blink);
	register_tchange(RECTANGLE_SELECT,set_rectangle_select);
	register_tchange(INSERT,set_insert);

	return terminal;
 }

 void DrawCursorPosition(void)
 {
#ifdef ENABLE_PANGO
	GdkGC 		*gc;
	PangoLayout *layout;
	int			x		= left_margin+(fontWidth*(terminal_cols-7));
	char		buffer[10];

	if(!pixmap)
		return;

	gc = getCachedGC(pixmap);

	gdk_gc_set_foreground(gc,color+TERMINAL_COLOR_OIA_BACKGROUND);
	gdk_draw_rectangle(pixmap,gc,1,x,OIAROW+1,fontWidth*7,fontHeight);

	layout = getPangoLayout(TEXT_LAYOUT_OIA);

	sprintf(buffer,"%03d/%03d",cRow+1,cCol+1);
	pango_layout_set_text(layout,buffer,-1);

	gdk_draw_layout_with_colors(pixmap,gc,
						x,OIAROW+1,
						layout,
						color+TERMINAL_COLOR_OIA_CURSOR,color+TERMINAL_COLOR_OIA_BACKGROUND);

	gtk_widget_queue_draw_area(terminal,x,OIAROW+1,fontWidth*7,fontHeight);

#endif // ENABLE_PANGO

 }

 static void set_showcursor(int value, enum toggle_type reason)
 {
 	#warning work in progress
/*
 	if(!(terminal && pixmap))
		return;

	if(value)
	{
		DrawCursorPosition();
	}
	else
	{
		GdkGC *gc	= getCachedGC(terminal->window);
		int   x		= left_margin+(fontWidth*(terminal_cols-7));

		gdk_gc_set_foreground(gc,color+TERMINAL_COLOR_OIA_BACKGROUND);
		gdk_draw_rectangle(pixmap,gc,1,x,OIAROW+1,fontWidth*7,fontHeight);

		gtk_widget_queue_draw_area(terminal,x,OIAROW+1,fontWidth*7,fontHeight);

	}
*/
	gtk_widget_queue_draw(terminal);
 }

 void update_cursor_position(int row, int col)
 {
	if(row == cRow && col == cCol)
		return;

	gtk_im_context_reset(input_method);

 	if(!screen_suspended)
 	{
 		int width;
 		int height;

		cMode |= CURSOR_MODE_SHOW;

		gdk_drawable_get_size(terminal->window,&width,&height);
		gtk_widget_queue_draw_area(terminal,0,rCursor.y+fontAscent,width,1);
		gtk_widget_queue_draw_area(terminal,rCursor.x,0,1,height);

		cCol			= col;
		cRow			= row;

		draw_cursor();

		if(Toggled(CURSOR_POS) && valid_terminal_window())
			DrawCursorPosition();
 	}
 	else
 	{
		cCol			= col;
		cRow			= row;
 	}

 }

/*
 void RedrawCursor(void)
 {
 #ifdef ENABLE_PANGO

	ELEMENT 	el;
	GdkGC   	*gc;

	if(!terminal->window)
		return;

	memset(&rCursor,0,sizeof(rCursor));

	// Draw cursor pixmap
	if(!pCursor)
		pCursor = gdk_pixmap_new(terminal->window,fontWidth,fontHeight,-1);

	memcpy(&el,screen + (cRow*terminal_cols)+cCol,sizeof(ELEMENT));
	el.fg = 0;
	el.bg = TERMINAL_COLOR_CURSOR;

	gc = gdk_gc_new(pCursor);
	DrawElement(pCursor, color, gc, 0, 0, &el);
	gdk_gc_destroy(gc);

	// Set cursor position
	rCursor.x 		= left_margin + (cCol * fontWidth);
	rCursor.y 		= top_margin + (cRow * fontHeight);
	rCursor.width 	= fontWidth;

	if(Toggled(INSERT))
	{
		rCursor.height = 0;
	}
	else
	{
		rCursor.height =fontHeight - (fontHeight/4);

		if(rCursor.height < 1)
			rCursor.height = 1;
	}

	// Mark to redraw
	gtk_im_context_set_cursor_location(input_method,&rCursor);
	InvalidateCursor();
#else // ENABLE_PANGO

	#warning Need more work

#endif // ENABLE_PANGO
 }

*/

