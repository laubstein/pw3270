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
 * Este programa está nomeado como terminal.c e possui - linhas de código.
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
#include "oia.h"
#include "fonts.h"

#include <lib3270/toggle.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdlib.h>

#ifndef __APPLE__
	#include <malloc.h>
#endif

/*---[ Structs ]------------------------------------------------------------------------------------------------*/


/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 GtkWidget		* terminal				= NULL;
 GdkPixmap		* pixmap_terminal		= NULL;
 GtkIMContext	* input_method			= NULL;

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

 static void set_showcursor(int value, enum toggle_type reason);
 static void set_blink(int value, enum toggle_type reason);

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static void release_pixmaps(void)
 {
 	int f;

	if(pixmap_terminal)
	{
		gdk_pixmap_unref(pixmap_terminal);
		pixmap_terminal = NULL;
	}

	if(get_cursor_pixmap())
	{
		gdk_pixmap_unref(get_cursor_pixmap());
		pixmap_cursor = NULL;
	}

	for(f=0;f<OIA_PIXMAP_COUNT;f++)
	{
		if(pixmap_oia[f])
		{
			gdk_pixmap_unref(pixmap_oia[f]);
			pixmap_oia[f] = 0;
		}
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
		g_object_set_data_full(G_OBJECT(pixmap_terminal),"cached_gc",gdk_gc_new(GDK_DRAWABLE(pixmap_terminal)),g_object_unref);
		update_terminal_contents();
 	}

    gdk_cairo_set_source_pixmap(cr, pixmap_terminal, 0, 0);
    gdk_cairo_rectangle(cr, &event->area);
    cairo_fill(cr);

	if((cMode & CURSOR_MODE_ENABLED))
	{
		if(!get_cursor_pixmap())
		{
			pixmap_cursor = gdk_pixmap_new(window,rCursor.width,rCursor.height,-1);
			update_cursor_pixmap();
		}

		if( (cMode & (CURSOR_MODE_BASE|CURSOR_MODE_SHOW)) == (CURSOR_MODE_BASE|CURSOR_MODE_SHOW) )
		{
			gdk_cairo_set_source_pixmap(cr, get_cursor_pixmap(), rCursor.x, rCursor.y);

			if(Toggled(INSERT))
				cairo_rectangle(cr, rCursor.x, rCursor.y, rCursor.width, rCursor.height);
			else
				cairo_rectangle(cr, rCursor.x, rCursor.y+terminal_font_info.ascent, rCursor.width, terminal_font_info.descent);

			cairo_fill(cr);

/*
			GtkStyle	*style	= gtk_widget_get_style(widget);
			gdk_draw_drawable(		widget->window,	style->fg_gc[GTK_STATE_NORMAL],
									GDK_DRAWABLE(get_cursor_pixmap()),
									0,0,
									rCursor.x,rCursor.y,
									rCursor.width,rCursor.height);
*/
		}


		if(cMode & CURSOR_MODE_CROSS)
		{
			// Draw cross-hair cursor
			int			width;
			int			height;

			gdk_drawable_get_size(window,&width,&height);

			gdk_cairo_set_source_color(cr,color+TERMINAL_COLOR_CROSS_HAIR);
			cairo_rectangle(cr, rCursor.x, 0, 1, top_margin+2+(terminal_font_info.spacing*terminal_rows));
			cairo_rectangle(cr, 0, rCursor.y+fontAscent, width,1);
			cairo_fill(cr);
		}
	}

    cairo_destroy(cr);

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

 	update_terminal_font_size(width, height);

	// Invalidate pixmaps, will redraw on next expose
	release_pixmaps();
	update_cursor_info();

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
	release_font_info(&terminal_font_info);

 	Trace("%s","Terminal destroyed");

 }

 static void realize(GtkWidget *widget, void *t)
 {
 	gint width, height;
	cairo_t	*cr			= gdk_cairo_create(terminal->window);
	gchar	*fontname	= GetString("Terminal","Font","Courier");

    Trace("Realizing %p",widget);

	// Configure im context
    gtk_im_context_set_client_window(input_method,widget->window);

#ifdef MOUSE_POINTER_CHANGE
    gdk_window_set_cursor(widget->window,wCursor[0]);
#endif

	update_font_info(cr, fontname, &terminal_font_info);

	gtk_widget_set_size_request(terminal, terminal_cols*terminal_font_info.width, ((terminal_rows+2)*terminal_font_info.height));

	g_free(fontname);
	cairo_destroy(cr);

	// Set terminal size
	gdk_drawable_get_size(widget->window,&width,&height);

	ResizeTerminal(widget, width, height);

 }

 static gboolean focus_in(GtkWidget *widget, GdkEventFocus *event, gpointer x)
 {
	gtk_im_context_focus_in(input_method);
	queue_draw_cursor();
	return 0;
 }

 static gboolean focus_out(GtkWidget *widget, GdkEventFocus *event, gpointer x)
 {
	gtk_im_context_focus_out(input_method);
	queue_draw_cursor();
	return 0;
 }

 static void im_commit(GtkIMContext *imcontext, gchar *arg1, gpointer user_data)
 {
	ParseInput(arg1);
 }

 static void update_keyboard_state(GdkEventKey *event, gboolean status)
 {

	if(event->keyval == GDK_Shift_R || event->keyval == GDK_Shift_L)
	{
		oia_shift_state = status;
		update_oia_element(OIA_ELEMENT_SHIFT_STATE);
		Trace("Shift is %s",status ? "Active" : "Inactive");
	}

	if(event->keyval == GDK_Alt_L || event->keyval == GDK_ISO_Level3_Shift)
	{
		oia_alt_state = status;
		update_oia_element(OIA_ELEMENT_ALT_STATE);
		Trace("Alt is %s",status ? "Active" : "Inactive");
	}

#if defined(HAVE_CAPS_STATE) || defined(DEBUG)
	if(event->keyval == GDK_Caps_Lock)
		update_oia_element(OIA_ELEMENT_CAPS_INDICATOR);
#endif // GTK_CHECK_VERSION(2,16,0)

 }

 static gboolean key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {

 	update_keyboard_state(event,TRUE);

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
 	update_keyboard_state(event,FALSE);

	// Check for alt-key again
	if(!(event->state & GDK_ALT_MASK) && oia_alt_state)
	{
		oia_alt_state = FALSE;
		update_oia_element(OIA_ELEMENT_ALT_STATE);
	}

	if(gtk_im_context_filter_keypress(input_method,event))
		return TRUE;

	return FALSE;
 }

 static void set_crosshair(int value, enum toggle_type reason)
 {
	queue_draw_cursor();
 	if(value)
		cMode |= CURSOR_MODE_CROSS;
	else
		cMode &= ~CURSOR_MODE_CROSS;
	queue_draw_cursor();
 }

 static gboolean blink_cursor(gpointer ptr)
 {
 	if(!cursor_blink)
		return FALSE;

 	if(cMode & CURSOR_MODE_ENABLED)
 	{
 		if(GTK_WIDGET_HAS_FOCUS(terminal))
			cMode ^= CURSOR_MODE_SHOW;
		else
			cMode |= CURSOR_MODE_SHOW;
		gtk_widget_queue_draw_area(terminal,rCursor.x,rCursor.y,rCursor.width,rCursor.height);
 	}
	return TRUE;
 }


 static void set_blink(int value, enum toggle_type reason)
 {
	if(value)
	{
		// Activate blinking
		if(cursor_blink)
			return;

		cursor_blink = TRUE;
		g_timeout_add((guint) 750, (GSourceFunc) blink_cursor, 0);

	}
	else
	{
		// Deactivate blinking
		if(!cursor_blink)
			return;

		cursor_blink = FALSE;
		cMode |= CURSOR_MODE_SHOW;

	}

	queue_draw_cursor();
 }

 static void set_insert(int value, enum toggle_type reason)
 {
	gtk_widget_queue_draw_area(terminal,rCursor.x,rCursor.y,rCursor.width,rCursor.height);
	update_oia_element(OIA_ELEMENT_INSERT_INDICATOR);
 }

 static void size_allocate(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
 {
// 	Trace("Terminal changes to %dx%d pixels",allocation->width,allocation->height);
    if(widget->window)
		ResizeTerminal(widget,allocation->width,allocation->height);
 }

 GtkWidget *CreateTerminalWindow(void)
 {
	input_method = gtk_im_multicontext_new();

	load_font_sizes();

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

/**
 * Liga/desliga apresentação da posição do cursor na OIA
 *
 * @param value	Valor modificado (Diferente de zero apresenta o cursor)
 */
 static void set_showcursor(int value, enum toggle_type reason)
 {
	update_oia_element(OIA_ELEMENT_CURSOR_POSITION);
 }

