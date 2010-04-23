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
#include <lib3270/toggle.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

// TODO (perry#7#): Find a better way to get font sizes!!!
#define MAX_FONT_SIZES	54


#define KEYBOARD_STATE_MASK GDK_SHIFT_MASK
// |GDK_ALT_MASK|GDK_LOCK_MASK)

/*---[ Structs ]------------------------------------------------------------------------------------------------*/

 typedef struct _fontsize
 {
 	int 	size;
 	int 	width;
	int 	height;
 } FONTSIZE;

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 GtkWidget						*terminal					= NULL;
 GdkPixmap						*pixmap						= NULL;

 GdkColor						color[TERMINAL_COLOR_COUNT+1];

 static int 					lWidth 			= -1;
 static int 					lHeight 		= -1;
 static int 					lFont 			= -1;

 static gint					sWidth			= 0;
 static gint					sHeight			= 0;

 static PangoFontDescription	*terminal_font_descr		= NULL;

 static GtkIMContext			*im;

 static const gchar 			*layout_name[] = { "PangoLayout_normal", "PangoLayout_underline" };

 static int					szFonts			= MAX_FONT_SIZES;
 static FONTSIZE				fsize[MAX_FONT_SIZES];
 gint							fontWidth		= 0;
 gint							fontHeight		= 0;

 // Cursor info
 gint							cMode			= CURSOR_MODE_ENABLED|CURSOR_MODE_BASE|CURSOR_MODE_SHOW;
 gint							cCol			= 0;
 gint							cRow			= 0;
 static GdkPixmap				*pCursor		= NULL;
 static GdkRectangle			rCursor;
 static int					blink_enabled	= 0;

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

 static void set_showcursor(int value, enum toggle_type reason);
 static void set_blink(int value, enum toggle_type reason);

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static GdkPixmap * GetPixmap(GtkWidget *widget)
 {
	GdkPixmap	*pix;

	if(!widget->window)
		return NULL;

	gdk_drawable_get_size(widget->window,&sWidth,&sHeight);

	Trace("Creating pixmap with %dx%d",sWidth,sHeight);

	pix = gdk_pixmap_new(widget->window,sWidth,sHeight,-1);

	getCachedGC(pix);

	DrawScreen(color, pix);
	DrawOIA(pix,color);
	RedrawCursor();
	return pix;
 }

 static gboolean expose(GtkWidget *widget, GdkEventExpose *event, void *t)
 {
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

	return 0;
 }

 static void UpdateFontData(int sel)
 {
	pango_font_description_set_size(terminal_font_descr,fsize[sel].size);

//	Trace("Updating font data (sel: %d)",sel);
	gtk_widget_modify_font(terminal,terminal_font_descr);

 	fsize[sel].width = fontWidth;
 	fsize[sel].height = fontHeight;

//	Trace("Font(%d): Width=%d Height: %d",sel,fontWidth,fontHeight);

 }

 static void LoadFontSizes(void)
 {
	/* Load all font sizes */
	gchar	*conf;
	gchar	**ptr;
	int 	f;

	memset(fsize,0,MAX_FONT_SIZES * sizeof(FONTSIZE));

	// Default font sizes from http://svn.gnome.org/svn/gtk+/trunk/gtk/gtkfontsel.c
	conf = GetString("Terminal","FontSizes","6,7,8,9,10,11,12,13,14,16,18,20,22,24,26,28,32,36,40,48,56,64,72");

	if(conf && *conf && *conf != '*') // "*" in .conf reverts to default behavior of "all" font sizes
	{
		Trace("Font sizes: %s",conf);
		ptr = g_strsplit(conf,",",MAX_FONT_SIZES);
		for(f=0;ptr[f];f++)
		{
			fsize[f].size = atoi(ptr[f]) * PANGO_SCALE;
			UpdateFontData(f);
		}
		szFonts = f;
		g_strfreev(ptr);
	}
	else
	{
		Trace("Loading font sizes from 0 to %d",MAX_FONT_SIZES);
		szFonts = MAX_FONT_SIZES;

		for(f=0;f<MAX_FONT_SIZES;f++)
		{
			fsize[f].size = (f+1) * PANGO_SCALE;
			UpdateFontData(f);
		}
	}

	g_free(conf);
 }

 static void ResizeTerminal(GtkWidget *widget, gint width, gint height)
 {
 	int				f;
	int 			left	= left_margin;
	int 			top		= top_margin;;
	GdkPixmap		*pix;
	GdkGC			*gc;

	if(lWidth == width && lHeight == height && lFont > 0)
		return;

	lWidth = width;
	lHeight = height;

	if(lFont == -1)
	{
		lFont = -2;
		LoadFontSizes();
		gtk_widget_set_usize(widget,(terminal_cols*fsize->width),((terminal_rows+1)*fsize->height));
	}

	/* Get the best font for the current window size */
	for(f=0;f<szFonts && ((fsize[f].height*(terminal_rows+1))+2) < height && (fsize[f].width*terminal_cols) < width;f++);

	if(f >= MAX_FONT_SIZES)
		f = (MAX_FONT_SIZES-1);
	else if(f > 0)
		f--;

	Trace("LastFont: %d NEwFont: %d",lFont,f);

	if(f != lFont)
	{
		Trace("Font size changes from %d to %d (%dx%d)",lFont,f,terminal_cols*fsize[f].width,terminal_rows*fsize[f].height);
		lFont = f;
		pango_font_description_set_size(terminal_font_descr,fsize[f].size);
		gtk_widget_modify_font(terminal,terminal_font_descr);
	}

	/* Center image */
	left_margin = (width >> 1) - ((terminal_cols * fontWidth) >> 1);
	if(left_margin < 0)
		left_margin = 0;

	top_margin = (height >> 1) - (((terminal_rows+1) * fontHeight) >> 1);
	if(top_margin < 0)
		top_margin = 0;

	/* No pixmap, will create a new one in the next expose */
	if(!pixmap)
		return;

	/* Font size hasn't changed, rebuild pixmap using the saved image */
	pix = gdk_pixmap_new(widget->window,sWidth = width,sHeight = height,-1);
	gc = getCachedGC(pix);

	gdk_gc_set_foreground(gc,color);
	gdk_draw_rectangle(pix,gc,1,0,0,width,height);

	gdk_draw_drawable(pix,gc,pixmap,
								left,top,
								left_margin,top_margin,
								(terminal_cols * fsize[lFont].width),OIAROW+1+fsize[lFont].height);

	/* Set the new pixmap */
	gdk_pixmap_unref(pixmap);
	pixmap = pix;
	RedrawCursor();
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

	if(pixmap)
	{
		gdk_pixmap_unref(pixmap);
		pixmap = NULL;
	}

 	if(terminal_font_descr)
 	{
		pango_font_description_free(terminal_font_descr);
		terminal_font_descr = NULL;
 	}

 	Trace("%s","Terminal destroyed");

 }

 static void realize(GtkWidget *widget, void *t)
 {
 	gint width, height;

    Trace("Realizing %p",widget);

	// Configure im context
    gtk_im_context_set_client_window(im,widget->window);

#ifdef MOUSE_POINTER_CHANGE
    gdk_window_set_cursor(widget->window,wCursor[0]);
#endif

    // Load imagems
    LoadImages(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE(widget)]);

	// Set terminal size
	gdk_drawable_get_size(widget->window,&width,&height);
	ResizeTerminal(widget, width, height);

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
	UpdateKeyboardState(event->state & KEYBOARD_STATE_MASK);

	if(gtk_im_context_filter_keypress(im,event))
		return TRUE;

	if(KeyboardAction(widget,event,user_data))
	{
		gtk_im_context_reset(im);
		return TRUE;
	}

	return FALSE;
 }

 static gboolean key_release(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
	UpdateKeyboardState(event->state & KEYBOARD_STATE_MASK);

	if(gtk_im_context_filter_keypress(im,event))
		return TRUE;

	return FALSE;
 }

 static void set_crosshair(int value, enum toggle_type reason)
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
 		if(GTK_WIDGET_HAS_FOCUS(terminal))
			cMode ^= CURSOR_MODE_SHOW;
		else
			cMode |= CURSOR_MODE_SHOW;
		InvalidateCursor();
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


	gtk_widget_queue_draw(terminal);
 }

 static void set_insert(int value, enum toggle_type reason)
 {
	DrawOIA(pixmap,color);

	if(terminal)
		gtk_widget_queue_draw(terminal);

 	RedrawCursor();
 }

 static void size_allocate(GtkWidget *widget, GtkAllocation *allocation, gpointer user_data)
 {
 	Trace("Terminal changes to %dx%d pixels",allocation->width,allocation->height);
    if(widget->window)
		ResizeTerminal(widget,allocation->width,allocation->height);
 }

 void FontChanged(void)
 {
 	gchar	*font = GetString("Terminal","Font","Courier");
 	gchar 	vlr[1024];

	g_snprintf(vlr,1023,"%s%s",	font, TOGGLED_BOLD ? " Bold" : "" );

	g_free(font);

	SetTerminalFont(vlr);

 }

 void SetTerminalFont(const gchar *fontname)
 {
 	Trace("Selected font: %s (last: %p)",fontname,terminal_font_descr);

	lFont = -1;

 	if(terminal_font_descr)
		pango_font_description_free(terminal_font_descr);

	terminal_font_descr = pango_font_description_from_string(fontname);

	if(terminal && terminal->window)
	{
		gdk_drawable_get_size(terminal->window,&sWidth,&sHeight);
		ResizeTerminal(terminal,sWidth,sHeight);
		gtk_widget_queue_draw(terminal);
	}

 }

 static void update_font_layout(void)
 {
 	PangoLayout *layout;

 	// Font context changed, invalidate cached info
	if(pixmap)
	{
		gdk_pixmap_unref(pixmap);
		pixmap = NULL;
	}

	if(pCursor)
	{
		gdk_pixmap_unref(pCursor);
		pCursor = NULL;
	}

	// Update metrics
	layout = getPangoLayout(TEXT_LAYOUT_UNDERLINE);
	pango_layout_context_changed(layout);

	layout = getPangoLayout(TEXT_LAYOUT_NORMAL);
	pango_layout_context_changed(layout);

	// FIXME (perry#1#): Is there any better way to get the font size in pixels?
	pango_layout_set_text(layout,"A",1);
	pango_layout_get_pixel_size(layout,&fontWidth,&fontHeight);

//	Trace("Font size changes to %dx%d",fontWidth,fontHeight);

 }

 static void direction_changed(GtkWidget *widget, GtkTextDirection previous_direction, gpointer user_data)
 {
 	update_font_layout();
 }

 static void style_set(GtkWidget *widget, GtkStyle  *previous_style, gpointer user_data)
 {
 	update_font_layout();
 }

 GtkWidget *CreateTerminalWindow(void)
 {
	memset(fsize,0,MAX_FONT_SIZES * sizeof(FONTSIZE));
	memset(color,0,sizeof(GdkColor)*TERMINAL_COLOR_COUNT);

 	LoadColors();
 	FontChanged();

	im = gtk_im_context_simple_new();

	terminal = gtk_event_box_new();
	gtk_widget_set_app_paintable(terminal,TRUE);
	gtk_widget_set_redraw_on_allocate(terminal,TRUE);

	g_signal_connect(G_OBJECT(terminal), "destroy", 			G_CALLBACK(destroy), NULL);
	g_signal_connect(G_OBJECT(terminal), "direction-changed",	G_CALLBACK(direction_changed), NULL);
	g_signal_connect(G_OBJECT(terminal), "style-set",			G_CALLBACK(style_set), NULL);

	// Configure terminal widget
    GTK_WIDGET_SET_FLAGS(terminal, GTK_CAN_DEFAULT|GTK_CAN_FOCUS);

    // http://developer.gnome.org/doc/API/2.0/gdk/gdk-Events.html#GdkEventMask
    gtk_widget_add_events(terminal,GDK_KEY_PRESS_MASK|GDK_KEY_RELEASE_MASK|GDK_BUTTON_PRESS_MASK|GDK_BUTTON_MOTION_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK|GDK_ENTER_NOTIFY_MASK);

    g_signal_connect(G_OBJECT(terminal),	"expose_event",  		G_CALLBACK(expose),					0);
    g_signal_connect(G_OBJECT(terminal),	"configure-event",		G_CALLBACK(configure),				0);
    g_signal_connect(G_OBJECT(terminal),	"size-allocate",		G_CALLBACK(size_allocate),			0);
    g_signal_connect(G_OBJECT(terminal),	"key-press-event",		G_CALLBACK(key_press),				0);
    g_signal_connect(G_OBJECT(terminal),	"key-release-event",	G_CALLBACK(key_release),			0);
    g_signal_connect(G_OBJECT(terminal),	"realize",				G_CALLBACK(realize),				0);
    g_signal_connect(G_OBJECT(terminal),	"focus-in-event",		G_CALLBACK(focus_in),				0);
    g_signal_connect(G_OBJECT(terminal),	"focus-out-event",		G_CALLBACK(focus_out),				0);
    g_signal_connect(G_OBJECT(im),			"commit",				G_CALLBACK(im_commit),				0);

    // Connect mouse events
    g_signal_connect(G_OBJECT(terminal), 	"button-press-event",	G_CALLBACK(mouse_button_press),		0);
    g_signal_connect(G_OBJECT(terminal), 	"button-release-event",	G_CALLBACK(mouse_button_release),	0);
    g_signal_connect(G_OBJECT(terminal), 	"motion-notify-event",	G_CALLBACK(mouse_motion),    		0);
    g_signal_connect(G_OBJECT(terminal), 	"enter-notify-event",	G_CALLBACK(mouse_enter),    		0);
    g_signal_connect(G_OBJECT(terminal), 	"scroll-event",			G_CALLBACK(mouse_scroll),			0);

	register_tchange(CROSSHAIR,set_crosshair);
	register_tchange(CURSOR_POS,set_showcursor);
	register_tchange(CURSOR_BLINK,set_blink);
	register_tchange(RECTANGLE_SELECT,set_rectangle_select);
	register_tchange(INSERT,set_insert);

	return terminal;
 }

 void InvalidateCursor(void)
 {
	gtk_widget_queue_draw_area(terminal,rCursor.x,rCursor.y+rCursor.height,rCursor.width,fontHeight-rCursor.height);
	if(cMode & CURSOR_MODE_CROSS)
	{
		gtk_widget_queue_draw_area(terminal,rCursor.x,0,rCursor.x,OIAROW-1);
		gtk_widget_queue_draw_area(terminal,0,rCursor.y+fontHeight,sWidth,rCursor.y+fontHeight);
	}
 }

 void DrawCursorPosition(void)
 {
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

 }

 static void set_showcursor(int value, enum toggle_type reason)
 {
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

	gtk_widget_queue_draw(terminal);
 }

 void MoveCursor(int row, int col)
 {
	if(row == cRow && col == cCol)
		return;

	gtk_im_context_reset(im);

 	if(!screen_suspended)
 	{
		cMode |= CURSOR_MODE_SHOW;
		InvalidateCursor();

		cCol			= col;
		cRow			= row;

		RedrawCursor();

		if(Toggled(CURSOR_POS) && terminal && pixmap)
			DrawCursorPosition();
 	}
 	else
 	{
		cCol			= col;
		cRow			= row;
 	}

 }

 void RedrawCursor(void)
 {
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
	gtk_im_context_set_cursor_location(im,&rCursor);
	InvalidateCursor();
 }

 PangoLayout * getPangoLayout(enum text_layout id)
 {
	PangoAttribute	*attr;
	PangoAttrList 	*attrlist;
 	PangoLayout		*rc;

 	if(!terminal)
		return NULL;

	rc = (PangoLayout *) g_object_get_data(G_OBJECT(terminal),layout_name[id]);

 	if(!rc)
 	{
		rc = gtk_widget_create_pango_layout(terminal,"");

		if(id == TEXT_LAYOUT_UNDERLINE)
		{
			attrlist = pango_layout_get_attributes(rc);
			if(!attrlist)
				attrlist = pango_attr_list_new();

			attr = pango_attr_underline_new(PANGO_UNDERLINE_SINGLE);
			pango_attr_list_change(attrlist,attr);
			pango_layout_set_attributes(rc,attrlist);
		}

		g_object_set_data_full(G_OBJECT(terminal),layout_name[id],rc,g_object_unref);

 	}

	return rc;
 }


