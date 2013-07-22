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
 * Este programa está nomeado como oia.c e possui - linhas de código.
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

/*
 * The status line is laid out thusly (M is maxCOLS):
 *
 *	0			"4" in a square
 *	1			"A" underlined
 *	2			solid box if connected, "?" in a box if not
 *	5...M-47	message area
 *	M-46...45	SSL Status
 *	M-			Meta indication ("M" or blank)
 *	M-42		Alt indication ("A" or blank)
 *	M-			Compose indication ("C" or blank)
 *	M-			Compose first character
 *	M-40		Caps indication ("A" or blank)
 *	M-39...38	Shift Status
 *	M-36		Typeahead indication ("T" or blank)
 *	M-			Alternate keymap indication ("K" or blank)
 *	M-			Reverse input mode indication ("R" or blank)
 *	M-34		Insert mode indication (Special symbol/"I" or blank)
 *	M-32		Script indication
 *	M-			Printer indication ("P" or blank)
 *	M-29...		LU Name
 *	M-16..15	commang timing spinner
 *	M-14		command timing (Clock symbol and m:ss, or blank)
 *	M-7...m		cursor position (rrr/ccc or blank)
 *
 */

#include <math.h>
#include "gui.h"
#include "fonts.h"
#include "oia.h"

#ifndef __APPLE__
	#define ENABLE_BM_PIXMAPS 1
#endif

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

 static void oia_draw_cursor_position(cairo_t *cr, GdkGC *gc, GdkRectangle *r);
 static void oia_draw_lu_name(cairo_t *cr, GdkGC *gc, GdkRectangle *r);

 static void oia_draw_insert_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r);
 static void oia_draw_typeahead_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r);
 static void oia_draw_script_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r);
 static void oia_undera_indicator(cairo_t *cr, GdkGC *gc, GdkRectangle *r);
 static void oia_connection_status(cairo_t *cr, GdkGC *gc, GdkRectangle *r);

 static void oia_draw_shift_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r);
 static void oia_draw_alt_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r);
 static void oia_draw_ssl_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r);
 static void oia_draw_timer(cairo_t *cr, GdkGC *gc, GdkRectangle *r);

#if defined(HAVE_CAPS_STATE) || defined(DEBUG)
 static void oia_draw_caps_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r);
#endif // HAVE_CAPS_STATE || DEBUG

 static void oia_four_in_a_square(cairo_t *cr, GdkGC *gc, GdkRectangle *r);

 static void oia_draw_spinner(cairo_t *cr, GdkGC *gc, GdkRectangle *r);

 static void oia_message_area(cairo_t *cr, GdkGC *gc, GdkRectangle *r);

// static void dunno(cairo_t *cr, GdkGC *gc, GdkRectangle *r);

 static void oia_clear_rect(cairo_t *cr, GdkRectangle *r);
 static void oia_show_text(cairo_t *cr, GdkRectangle *r, const gchar *text, enum TERMINAL_COLOR c);

/*---[ Statics ]------------------------------------------------------------------------------------------------*/

#ifdef ENABLE_BM_PIXMAPS
 static GdkPixmap * pixmap_oia[OIA_PIXMAP_COUNT] = { NULL, NULL, NULL};
#endif // ENABLE_BM_PIXMAPS

 #define OIAROW	(view.top+4+(terminal_font_info.spacing*view.rows))

 gboolean		  oia_flag[OIA_FLAG_USER];
 LIB3270_STATUS	  terminal_message_id = (LIB3270_STATUS) -1;

 SCRIPT_STATE	  oia_script_state = SCRIPT_STATE_NONE;

#if defined(HAVE_CAPS_STATE) || defined(DEBUG)
 gboolean		  oia_caps_state = FALSE;
#endif // HAVE_CAPS_STATE || DEBUG

 gboolean		  oia_script_blink = TRUE;

 gboolean		  oia_shift_state	= FALSE;
 gboolean		  oia_alt_state = FALSE;

 gint			  oia_timer = -1;
 gint			  oia_spinner_step = 0;

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 static const struct _oia_calls
 {
	void (*update)(cairo_t *cr, GdkGC *gc, GdkRectangle *r);
 } oia_call[OIA_ELEMENT_COUNT] =
 {
	{ oia_four_in_a_square		},	// "4" in a square
	{ oia_undera_indicator		},	// "A" underlined
	{ oia_connection_status		},	// solid box if connected, "?" in a box if not
	{ oia_message_area			},	// message area
	{ oia_draw_ssl_state		},	// SSL Status
	{ oia_draw_alt_state		},	// Alt indication ("A" or blank)
	{ oia_draw_shift_state		},	// Shift Status
	{ oia_draw_typeahead_state	},	// Typeahead indication ("T" or blank)
	{ oia_draw_insert_state		},	// Insert mode indication (Special symbol/"I" or blank)
	{ oia_draw_script_state		},	// Script indication ("S" or blank)
	{ oia_draw_lu_name				},	// LU Name
	{ oia_draw_timer			},	// command timing (Clock symbol and m:ss, or blank)
	{ oia_draw_spinner			},  // Spinner indicator
	{ oia_draw_cursor_position	},	// cursor position (rrr/ccc or blank)

#if defined(HAVE_CAPS_STATE) || defined(DEBUG)
	{ oia_draw_caps_state		},	// Caps indications ("A" or blank)
#endif // HAVE_CAPS_STATE || DEBUG

 };


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 void update_oia(void)
 {
 	if(valid_terminal_window())
	{
		cairo_t *cr	= get_terminal_cairo_context();
		GdkGC *gc = get_terminal_cached_gc();
		draw_oia(cr,gc);
		cairo_destroy(cr);
		gtk_widget_queue_draw_area(terminal,OIAROW,view.left,view.cols*fontWidth,terminal_font_info.spacing+2);
	}
 }

 void draw_oia(cairo_t *cr, GdkGC *gc)
 {
 	int	f;
 	int row   = OIAROW;
 	int width = (view.cols * terminal_font_info.width);

	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, view.left, row, width, terminal_font_info.height);
	cairo_fill(cr);

	width += view.left;
	for(f=0;f<OIA_ELEMENT_COUNT;f++)
	{
		GdkRectangle rect;

		memset(&rect,0,sizeof(rect));
		rect.y = row;
		rect.width = width;
		rect.height = terminal_font_info.height;
		oia_call[f].update(cr,gc,&rect);
	}

	// http://cairographics.org/FAQ/#sharp_lines
	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_SEPARATOR);
	cairo_rectangle(cr, view.left, row-2, (view.cols * fontWidth), 1);
	cairo_fill(cr);

 }

 void update_oia_element(OIA_ELEMENT el)
 {
 	if(valid_terminal_window())
 	{
		cairo_t *cr = get_terminal_cairo_context();
		GdkRectangle rect;

		memset(&rect,0,sizeof(rect));
		rect.y = OIAROW;
		rect.height = terminal_font_info.height;
		rect.width = view.left + (view.cols * fontWidth);

		oia_call[el].update(cr,get_terminal_cached_gc(),&rect);

		cairo_destroy(cr);

		if(rect.width > 0)
			gtk_widget_queue_draw_area(terminal,rect.x,rect.y,rect.width,rect.height);

 	}
 }

 static void oia_clear_rect(cairo_t *cr, GdkRectangle *r)
 {
	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, r->x, r->y, r->width,r->height);
	cairo_fill(cr);

#ifdef DEBUG
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.4);
//	cairo_move_to(cr,r->x,r->y+r->height);
//	cairo_line_to(cr,r->x+r->width,r->y+r->height);
	cairo_rectangle(cr, r->x, r->y, r->width,r->height);
	cairo_stroke(cr);
#endif
 }

 static void oia_show_text(cairo_t *cr, GdkRectangle *r, const gchar *text, enum TERMINAL_COLOR c)
 {
	cairo_set_3270_color(cr,c);
	cairo_move_to(cr,r->x,r->y+fontAscent);
	cairo_show_text(cr,text);
 }

 static void oia_clear_icon(cairo_t *cr, GdkRectangle *r)
 {
	if(r->width > r->height)
	{
		r->x += (r->width - r->height)/2;
		r->width = r->height;
	}
	else if(r->height > r->width)
	{
		r->y += (r->height - r->width)/2;
		r->height = r->width;
	}

	oia_clear_rect(cr,r);

 }

 static gint draw_spinner(cairo_t *cr, GdkRectangle *r, gint step)
 {
	static const guint num_steps	= 10;

	gdouble dx = r->width/2;
	gdouble dy = r->height/2;
	gdouble radius = MIN (r->width / 2, r->height / 2);
	gdouble half = num_steps / 2;
	gint i;


	cairo_save(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	cairo_rectangle(cr, r->x, r->y, r->width, r->height);
	cairo_clip(cr);
	cairo_translate(cr, r->x, r->y);

	step %= num_steps;

	for (i = 0; i < num_steps; i++)
	{
		gint inset = 0.7 * radius;

		/* transparency is a function of time and intial value */
		gdouble t = (gdouble) ((i + num_steps - step) % num_steps) / num_steps;

		cairo_save(cr);

		cairo_set_source_rgba (cr,
							 color[TERMINAL_COLOR_OIA_SPINNER].red / 65535.,
							 color[TERMINAL_COLOR_OIA_SPINNER].green / 65535.,
							 color[TERMINAL_COLOR_OIA_SPINNER].blue / 65535.,
							 t);

		cairo_set_line_width (cr, 2.0);
		cairo_move_to (cr,
					 dx + (radius - inset) * cos (i * G_PI / half),
					 dy + (radius - inset) * sin (i * G_PI / half));
		cairo_line_to (cr,
					 dx + radius * cos (i * G_PI / half),
					 dy + radius * sin (i * G_PI / half));
		cairo_stroke (cr);

		cairo_restore (cr);
	}

	cairo_restore(cr);

 	return step;
 }

 static void draw_border(cairo_t *cr, GdkGC *gc, GdkRectangle *r, enum TERMINAL_COLOR clr)
 {
	cairo_set_3270_color(cr,clr);

	cairo_rectangle(cr, r->x, r->y, r->width,1);
	cairo_rectangle(cr, r->x, r->y+r->height, r->width,1);
	cairo_rectangle(cr, r->x, r->y, 1, r->height);
	cairo_rectangle(cr, r->x+r->width, r->y, 1, r->height+1);

	cairo_fill(cr);
 }

/*
 * Update calls.
 */

 static void oia_draw_alt_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
	// *	M-42	Alt Status

	r->x = (r->width - (42*terminal_font_info.width));
	r->width = terminal_font_info.width;

	oia_clear_rect(cr,r);

	if(oia_alt_state)
		oia_show_text(cr,r,"A",TERMINAL_COLOR_OIA_ALT_STATE);

 }

 static void oia_draw_shift_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
	// *	M-39...38	Shift Status

	r->x = (r->width - (39*terminal_font_info.width));
	r->width = terminal_font_info.width*2;

	oia_clear_icon(cr,r);

	r->x++;
	r->y++;
	r->width -= 2;
	r->height -= 2;

	if(r->width < 3 || r->height < 3)
		return;

	if(oia_shift_state)
	{
/*
		http://4umi.com/web/javascript/xbm.htm

		  00000000001
		  01234567890
		00     * ....
		01    * *....
		02   *   *...
		03  *     *..
		04 *       *.
		05****   ****
		06   *   *
		07   *   *
		08   *   *
		09   *   *
		10   *   *
		11   *   *
		12   *   *
		13   *   *
		14   *   *
		15   *****
*/

		// Scale image
		int b,x,y,w,h,l;

		if(r->height > r->width)
		{
			w = r->width;
			h = w*1.5;
		}
		else // width > height
		{
			h = r->height;
			w = h/1.5;
		}

		// Set image position
		x = r->x + ((r->width - w)/2);
		y = r->y + ((r->height - h)/2);
		l = (w/3);
		b = y+(w/1.5);

		cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_SHIFT_STATE);

		cairo_move_to(cr,x+(w/2),y);
		cairo_line_to(cr,x+w,b);
		cairo_line_to(cr,(x+w)-l,b);
		cairo_line_to(cr,(x+w)-l,y+h);
		cairo_line_to(cr,x+l,y+h);
		cairo_line_to(cr,x+l,b);
		cairo_line_to(cr,x,b);
		cairo_close_path(cr);

		cairo_stroke(cr);

	}

 }

 static void oia_draw_typeahead_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
	r->x = (r->width - (36*terminal_font_info.width));
	r->width = terminal_font_info.width;

	oia_clear_rect(cr,r);

	if(oia_flag[OIA_FLAG_TYPEAHEAD])
		oia_show_text(cr,r,"T",TERMINAL_COLOR_OIA_TYPEAHEAD_STATE);
 }

#if defined(HAVE_CAPS_STATE) || defined(DEBUG)
 static void oia_draw_caps_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
	r->x = (r->width - (40*terminal_font_info.width));
	r->width = terminal_font_info.width;
	r->height = terminal_font_info.height;

	oia_clear_rect(cr,r);

#if GTK_CHECK_VERSION(2,16,0)
	// http://library.gnome.org/devel/gdk/stable/gdk-Keyboard-Handling.html#gdk-keymap-get-caps-lock-state
	oia_caps_state = gdk_keymap_get_caps_lock_state(gdk_keymap_get_default());
#endif

	if(oia_caps_state)
		oia_show_text(cr,r,"A",TERMINAL_COLOR_OIA_CAPS_STATE);

 }
#endif // HAVE_CAPS_STATE || DEBUG

#ifdef ENABLE_BM_PIXMAPS
 static GdkPixmap * oia_create_scaled_pixmap(GdkRectangle *r, GdkGC *gc, const unsigned char *data, gint width, gint height, enum TERMINAL_COLOR cl)
 {
 	GdkPixbuf *buf;
 	GdkPixbuf *pix;
 	GdkPixmap *ret;
	GdkPixmap *tmp = gdk_pixmap_create_from_data(	get_terminal_pixmap(),
													(const char *) data,width,height,
													gdk_drawable_get_depth(get_terminal_pixmap()),
													color+cl,
													color+TERMINAL_COLOR_OIA_BACKGROUND );

	buf = gdk_pixbuf_get_from_drawable(0, tmp, gdk_drawable_get_colormap(get_terminal_pixmap()),0, 0, 0, 0, width, height);

	gdk_pixmap_unref(tmp);

	pix = gdk_pixbuf_scale_simple(buf,r->width,r->height,GDK_INTERP_HYPER);

	ret = gdk_pixmap_new(get_terminal_pixmap(),r->width,r->height,gdk_drawable_get_depth(get_terminal_pixmap()));

	gdk_pixbuf_render_to_drawable(pix,GDK_DRAWABLE(ret),gc,0,0,0,0,-1,-1,GDK_RGB_DITHER_NORMAL,0,0);

    gdk_pixbuf_unref(buf);
    gdk_pixbuf_unref(pix);

    return ret;

 }

#endif //  ENABLE_BM_PIXMAPS

 static void oia_draw_ssl_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
/*
  00000000001111111
  01234567890123456
00        xxxxxxx
01       x       x
02      x  xxxxx  x
03      x x     x x
04      x x     x x
05      x x     x x
06      x x     x x
07      x x     x x
08      x x     xxx
09xxxxxxxxxxx
10xxxxxxxxxxx
11xxxxxxxxxxx
12xxxxxxxxxxx
13xxxxxxxxxxx
14xxxxxxxxxxx
15x         x
16xxxxxxxxxxx

  00000000001111111
  01234567890123456
00     xxxxxxx
01    x       x
02   x  xxxxx  x
03   x x     x x
04   x x     x x
05   x x     x x
06   x x     x x
07   x x     x x
08   x x     x x
09..xxxxxxxxxxxxx
10  xxxxxxxxxxxxx
11  xxxxxxxxxxxxx
12  xxxxxxxxxxxxx
13  xxxxxxxxxxxxx
14  xxxxxxxxxxxxx
15  x           x
16  xxxxxxxxxxxxx

*/

#ifdef ENABLE_BM_PIXMAPS

	// Pixmap version

	#include "locked.bm"
	#include "unlocked.bm"
	#include "warning.bm"

	static const struct _imagedata
	{
		const unsigned char	*data;
		gint					width;
		gint					height;
	} imagedata[] =
	{
		{ locked_bits,		locked_width,	locked_height	},
		{ unlocked_bits,	unlocked_width,	unlocked_height	},
		{ warning_bits,		warning_width,	warning_height	},

	};

	int idx = query_secure_connection(hSession) ? OIA_PIXMAP_LOCKED : OIA_PIXMAP_UNLOCKED;
	int color = TERMINAL_COLOR_OIA_SSL_STATE;


	r->x = (r->width - (46*terminal_font_info.width))+1;
	r->y++;
	r->width = (terminal_font_info.width*2)-1;
	r->height--;

	oia_clear_icon(cr,r);

	if(!query_ssl_cert_check_status(hSession))
	{
		idx = OIA_PIXMAP_WARNING;
		color = TERMINAL_COLOR_OIA_STATUS_WARNING;
	}

	if(!pixmap_oia[idx])
		pixmap_oia[idx] = oia_create_scaled_pixmap(r,gc,imagedata[idx].data,imagedata[idx].width,imagedata[idx].height,color);

	gdk_cairo_set_source_pixmap(cr, pixmap_oia[idx], r->x, r->y);
	gdk_cairo_rectangle(cr,r);
	cairo_fill(cr);

#else

	// Non Pixmap version

	r->x = (r->width - (46*terminal_font_info.width))+1;
	r->y++;
	r->width = (terminal_font_info.width*2)-1;
	r->height--;

	oia_clear_icon(cr,r);

	if(query_secure_connection(hSession))
	{
		cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_SSL_STATE);
		cairo_arc(cr,r->x+(r->width/2),r->y+(r->height/2),r->width/(2.5),0,2*M_PI);
		cairo_fill(cr);
	}

#endif // ENABLE_BM_PIXMAPS

 }

 static void oia_draw_insert_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
	r->x = (r->width - (34*terminal_font_info.width));
	r->width = terminal_font_info.width;

	oia_clear_icon(cr,r);

	if(Toggled(INSERT))
	{
		double y = r->y+(r->height-2);

		cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_INSERT_STATE);

		cairo_move_to(cr,r->x,y);
		cairo_rel_line_to(cr,r->width/2,-(r->height/1.7));
		cairo_line_to(cr,r->x+r->width,y);

		cairo_stroke(cr);
	}

/*
	oia_clear_rect(cr,r);

	if(Toggled(INSERT))
		oia_show_text(cr,r,"I",TERMINAL_COLOR_OIA_INDICATORS);
*/
 }

 static void oia_draw_script_state(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
	r->x = (r->width - (32*terminal_font_info.width));
	r->width = fontWidth;

	oia_clear_rect(cr,r);

	if(oia_script_state != SCRIPT_STATE_NONE && oia_script_blink)
		oia_show_text(cr,r,"S",TERMINAL_COLOR_OIA_SCRIPT_STATE);

 }

 static void oia_four_in_a_square(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
	//  0          "4" in a square
	r->x = view.left;
	r->width = terminal_font_info.width+2;

	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, r->x, r->y+1, r->width,r->height);
	cairo_fill(cr);

	draw_border(cr,gc,r,TERMINAL_COLOR_OIA_FOREGROUND);

	cairo_move_to(cr,r->x+2,r->y+fontAscent);
	cairo_show_text(cr,"4");
 }

 static void oia_undera_indicator(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
	//  1          "A" underlined
	r->x = (view.left+terminal_font_info.width+4);
	r->width = terminal_font_info.width;

	oia_clear_rect(cr,r);

	if(oia_flag[OIA_FLAG_UNDERA])
	{
		oia_show_text(cr,r,(IN_E) ? "B" : "A", TERMINAL_COLOR_OIA_UNDERA);

		cairo_move_to(cr,r->x,r->y+fontAscent+(fontDescent/2));
		cairo_rel_line_to(cr,terminal_font_info.width,0);
		cairo_stroke(cr);
	}

 }

 static void oia_connection_status(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
	// 2          solid box if connected, "?" in a box if not
	r->x = (view.left+(terminal_font_info.width*2)+6);
	r->width = terminal_font_info.width+2;

	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, r->x, r->y+1, r->width,r->height);
	cairo_fill(cr);

	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_FOREGROUND);

	if(IN_ANSI)
	{
		draw_border(cr,gc,r,TERMINAL_COLOR_OIA_FOREGROUND);
		cairo_move_to(cr,r->x+2,r->y+terminal_font_info.ascent);
		cairo_show_text(cr,"N");
	}
	else if(oia_flag[OIA_FLAG_BOXSOLID])
	{
		cairo_rectangle(cr, r->x, r->y, r->width,r->height+1);
		cairo_fill(cr);
	}
	else if(IN_SSCP)
	{
		draw_border(cr,gc,r,TERMINAL_COLOR_OIA_FOREGROUND);
		cairo_move_to(cr,r->x+2,r->y+terminal_font_info.ascent);
		cairo_show_text(cr,"S");
	}
	else
	{
		draw_border(cr,gc,r,TERMINAL_COLOR_OIA_FOREGROUND);
		cairo_move_to(cr,r->x+2,r->y+terminal_font_info.ascent);
		cairo_show_text(cr,"?");
	}

 }

 static void oia_draw_cursor_position(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
	// M-7..M		cursor position (rrr/ccc or blank)
	gint width = r->width;

	// For some reason the smallest size of "xos4 terminus bold" isn't drawing with the exact size;
	// that's the reason why I'm reserving a bigger space for this element
 	r->width = (terminal_font_info.width*9)+(terminal_font_info.width/2);
 	r->x = width - r->width;

	oia_clear_rect(cr,r);

	if(Toggled(CURSOR_POS))
	{
		gchar *text;
		cairo_text_extents_t s;

		text = g_strdup_printf("%03d/%03d",(cursor_position/screen->cols)+1,(cursor_position%screen->cols)+1);

		cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_CURSOR);
		cairo_text_extents(cr,text,&s);

		cairo_move_to(cr,width-(s.width+2),r->y+fontAscent);
		cairo_show_text(cr,text);

		g_free(text);
	}

 }

 static void oia_draw_lu_name(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
 	const char *luname = get_connected_lu(hSession);

 	r->x = (r->width - (terminal_font_info.width*29));
 	r->width = terminal_font_info.width*11;

	oia_clear_rect(cr,r);

	if(luname)
		oia_show_text(cr, r,luname, TERMINAL_COLOR_OIA_LUNAME);
 }

 static void oia_draw_timer(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
 	r->x = (r->width - (terminal_font_info.width*15));
 	r->width = terminal_font_info.width*6;

	oia_clear_rect(cr,r);

	if(oia_timer >= 0)
	{
		cairo_text_extents_t s;
		double x;
		gchar *text = g_strdup_printf("%02d:%02d",oia_timer/60,oia_timer % 60);

		cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_TIMER);

		cairo_text_extents(cr,text,&s);

		x = (r->width - s.width)/2;
		if(x > 0)
			x += r->x;
		else
			x = r->x;

		cairo_move_to(cr,x,r->y+fontAscent);
		cairo_show_text(cr,text);

		g_free(text);
	}

 }

 static void oia_draw_spinner(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
 	r->x = r->width - ((terminal_font_info.width*17)+(terminal_font_info.width/2));
	r->width = terminal_font_info.width*2;

	oia_clear_icon(cr,r);
	r->x++;
	r->y++;
	r->width--;
	r->height--;

	if(r->width < 2)
		return;

#ifdef DEBUG
	oia_spinner_step = draw_spinner(cr,r,oia_spinner_step);
#else
	if(oia_timer >= 0)
		oia_spinner_step = draw_spinner(cr,r,oia_spinner_step);
#endif

 }


 static void oia_message_area(cairo_t *cr, GdkGC *gc, GdkRectangle *r)
 {
	#ifdef DEBUG
		#define OIA_MESSAGE(x,c,y) { #x, c, y }
	#else
		#define OIA_MESSAGE(x,c,y) { c, y }
	#endif

	static const struct _message
	{
	#ifdef DEBUG
		const gchar	* dbg;
	#endif
		int				  color;
		const gchar	* string;
	} message[LIB3270_STATUS_USER] =
 	{
		OIA_MESSAGE(	LIB3270_STATUS_BLANK,
						TERMINAL_COLOR_OIA_STATUS_OK,
						"" ),

		OIA_MESSAGE(	LIB3270_STATUS_SYSWAIT,
						TERMINAL_COLOR_OIA_STATUS_OK,
						N_( "X System" ) ),

		OIA_MESSAGE(	LIB3270_STATUS_TWAIT,
						TERMINAL_COLOR_OIA_STATUS_OK,
						N_( "X Wait" ) ),

		OIA_MESSAGE(	LIB3270_STATUS_CONNECTED,
						TERMINAL_COLOR_OIA_STATUS_OK,
						NULL ),

		OIA_MESSAGE(	LIB3270_STATUS_DISCONNECTED,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X Not Connected" ) ),

		OIA_MESSAGE(	LIB3270_STATUS_AWAITING_FIRST,
						TERMINAL_COLOR_OIA_STATUS_OK,
						N_( "X" ) ),

		OIA_MESSAGE(	LIB3270_STATUS_MINUS,
						TERMINAL_COLOR_OIA_STATUS_OK,
						N_( "X -f" ) ),

		OIA_MESSAGE(	LIB3270_STATUS_PROTECTED,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X Protected" ) ),

		OIA_MESSAGE(	LIB3270_STATUS_NUMERIC,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X Numeric" ) ),

		OIA_MESSAGE(	LIB3270_STATUS_OVERFLOW,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X Overflow" ) ),

		OIA_MESSAGE(	LIB3270_STATUS_INHIBIT,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X Inhibit" ) ),

		OIA_MESSAGE(	LIB3270_STATUS_KYBDLOCK,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						NULL ),

		OIA_MESSAGE(	LIB3270_STATUS_X,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X" ) ),

		OIA_MESSAGE(	LIB3270_STATUS_RESOLVING,
						TERMINAL_COLOR_OIA_STATUS_WARNING,
						N_( "X Resolving" ) ),

		OIA_MESSAGE(	LIB3270_STATUS_CONNECTING,
						TERMINAL_COLOR_OIA_STATUS_WARNING,
						N_( "X Connecting" ) ),


	};

	static gint current_message_id = 0;

	const gchar *msg = NULL;

	if(message[terminal_message_id].string)
		current_message_id = terminal_message_id;

	msg = message[current_message_id].string;

	r->x = view.left + (terminal_font_info.width * 5);
	r->width = (r->width - (terminal_font_info.width * 47)) - r->x;

	if(r->width < 0)
		return;

	oia_clear_rect(cr,r);

#ifdef DEBUG
	if(!*msg)
		msg = message[terminal_message_id].dbg;
#endif

	if(*msg)
	{
		int x = r->x;
		msg = gettext(msg);

		cairo_set_3270_color(cr,message[terminal_message_id].color);

		if(*msg == 'X')
		{
			cairo_save(cr);

			cairo_move_to(cr,x+1,r->y+1);
			cairo_rel_line_to(cr,fontWidth,fontAscent);
			cairo_rel_move_to(cr,-fontWidth,0);
			cairo_rel_line_to(cr,fontWidth,-fontAscent);

			cairo_stroke(cr);
			x += fontWidth;
			msg++;

			cairo_restore(cr);
		}

		cairo_move_to(cr,x,r->y+fontAscent);
		cairo_show_text(cr,msg);

	}
 }

 static gboolean update_timer_spinner(gpointer dunno)
 {
	oia_spinner_step++;
	update_oia_element(OIA_ELEMENT_COMMAND_SPINNER);
	return (oia_timer >= 0);
 }

 void oia_set_timer(long seconds)
 {
 	if(seconds < 0)
 	{
 		oia_timer = -1;
 	}
 	else if(seconds == oia_timer)
 	{
 		return;
 	}
 	else
 	{
 		if(oia_timer < 0)
			g_timeout_add((guint) 150, (GSourceFunc) update_timer_spinner, 0);
		oia_timer = (seconds % 5940);
 	}
	update_oia_element(OIA_ELEMENT_COMMAND_SPINNER);
 	update_oia_element(OIA_ELEMENT_COMMAND_TIMER);
 }

 void oia_release_pixmaps(void)
 {
#ifdef ENABLE_BM_PIXMAPS
	 int f;

	for(f=0;f<G_N_ELEMENTS(pixmap_oia);f++)
	{
		if(pixmap_oia[f])
		{
			gdk_pixmap_unref(pixmap_oia[f]);
			pixmap_oia[f] = NULL;
		}
	}
#endif // ENABLE_BM_PIXMAPS
 }
