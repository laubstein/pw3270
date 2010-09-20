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

#include <math.h>
#include "gui.h"
#include "fonts.h"
#include "oia.h"

/*---[ Prototipes ]---------------------------------------------------------------------------------------------*/

 static void oia_cursor_position(cairo_t *cr, GdkRectangle *r);
 static void oia_lu_name(cairo_t *cr, GdkRectangle *r);

 static void oia_insert_indicator(cairo_t *cr, GdkRectangle *r);
 static void oia_draw_typeahead_state(cairo_t *cr, GdkRectangle *r);
 static void oia_script_indicator(cairo_t *cr, GdkRectangle *r);
 static void oia_undera_indicator(cairo_t *cr, GdkRectangle *r);
 static void oia_connection_status(cairo_t *cr, GdkRectangle *r);

 static void oia_draw_shift_state(cairo_t *cr, GdkRectangle *r);
 static void oia_draw_alt_state(cairo_t *cr, GdkRectangle *r);
 static void oia_draw_ssl_state(cairo_t *cr, GdkRectangle *r);

#if GTK_CHECK_VERSION(2,16,0)
 static void oia_draw_caps_state(cairo_t *cr, GdkRectangle *r);
#endif // GTK 2.16.0

 static void oia_four_in_a_square(cairo_t *cr, GdkRectangle *r);

 static void oia_update_timer(cairo_t *cr, GdkRectangle *r);
 static void oia_update_spinner(cairo_t *cr, GdkRectangle *r);

 static void oia_message_area(cairo_t *cr, GdkRectangle *r);

 static void dunno(cairo_t *cr, GdkRectangle *r);

 static void oia_clear_rect(cairo_t *cr, GdkRectangle *r);
 static void oia_show_text(cairo_t *cr, GdkRectangle *r, const gchar *text, enum TERMINAL_COLOR c);

/*---[ Statics ]------------------------------------------------------------------------------------------------*/

 gboolean		oia_flag[OIA_FLAG_USER];
 STATUS_CODE	terminal_message_id = (STATUS_CODE) -1;

 SCRIPT_STATE 	oia_script_state = SCRIPT_STATE_NONE;
#if GTK_CHECK_VERSION(2,16,0)
 gboolean		oia_caps_state;
#endif

 gboolean		oia_script_blink = TRUE;

 gboolean		oia_shift_state	= FALSE;
 gboolean		oia_alt_state = FALSE;

 gint			oia_timer = -1;
 gint			oia_spinner_step = 0;

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 static const struct _oia_calls
 {
	void (*update)(cairo_t *cr, GdkRectangle *r);
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
	{ oia_insert_indicator		},	// Insert mode indication (Special symbol/"I" or blank)
	{ oia_script_indicator		},	// Script indication ("S" or blank)
	{ oia_lu_name				},	// LU Name
	{ oia_update_timer			},	// command timing (Clock symbol and m:ss, or blank)
	{ oia_update_spinner		},  // Spinner indicator
	{ oia_cursor_position		},	// cursor position (rrr/ccc or blank)

#if GTK_CHECK_VERSION(2,16,0)
	{ oia_draw_caps_state		},	// Caps indications ("A" or blank)
#endif


 };


/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 static void dunno(cairo_t *cr, GdkRectangle *r)
 {
 }

/*
 void update_oia_contents(void)
 {
 	if(valid_terminal_window())
	{
		cairo_t *cr	= get_terminal_cairo_context();
		draw_oia(cr);
		cairo_destroy(cr);
		gtk_widget_queue_draw_area(terminal,OIAROW,left_margin,terminal_cols*fontWidth,fontHeight+1);
	}
 }
*/

 void draw_oia(cairo_t *cr)
 {
 	int	f;
 	int row = OIAROW;
 	int width = (terminal_cols * fontWidth);

	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, left_margin, row, width, fontHeight);
	cairo_fill(cr);

	// http://cairographics.org/FAQ/#sharp_lines
	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_SEPARATOR);
	cairo_rectangle(cr, left_margin, row-1, width, 1);
	cairo_fill(cr);

	width += left_margin;
	for(f=0;f<OIA_ELEMENT_COUNT;f++)
	{
		GdkRectangle rect;

		memset(&rect,0,sizeof(rect));
		rect.y = row;
		rect.width = width;
		rect.height = fontHeight;
		oia_call[f].update(cr,&rect);
	}
 }

 void update_oia_element(OIA_ELEMENT el)
 {
 	if(valid_terminal_window())
 	{
		cairo_t *cr = get_terminal_cairo_context();
		GdkRectangle rect;

		memset(&rect,0,sizeof(rect));
		rect.y = OIAROW;
		rect.height = fontHeight;
		rect.width = left_margin + (terminal_cols * fontWidth);

		oia_call[el].update(cr,&rect);

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
	cairo_set_source_rgb(cr, 0.0, 0.3, 0.0);
	cairo_move_to(cr,r->x,r->y+r->height);
	cairo_line_to(cr,r->x+r->width,r->y+r->height);
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

 static gint oia_draw_spinner(cairo_t *cr, GdkRectangle *r, gint step)
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

 static void draw_border(cairo_t *cr, GdkRectangle *r)
 {
/*
	cairo_move_to (cr, r->x, r->y);
	cairo_rel_line_to (cr, r->width, 0);
	cairo_rel_line_to (cr, 0, r->height);
	cairo_rel_line_to (cr, -r->width, 0);
	cairo_close_path(cr);
*/

	cairo_rectangle(cr, r->x, r->y, r->width,1);
	cairo_rectangle(cr, r->x, r->y+r->height, r->width,1);
	cairo_rectangle(cr, r->x, r->y, 1, r->height);
	cairo_rectangle(cr, r->x+r->width, r->y, 1, r->height+1);

	cairo_fill(cr);
 }

/*
 * Update calls.
 */

 static void oia_draw_alt_state(cairo_t *cr, GdkRectangle *r)
 {
	// *	M-41	Alt Status

	r->x = (r->width - (41*terminal_font_info.width));
	r->width = terminal_font_info.width;
	r->height = terminal_font_info.height;

	oia_clear_rect(cr,r);

#ifdef DEBUG
	if(oia_alt_state)
		oia_show_text(cr,r,"A",TERMINAL_COLOR_ALT_STATE);
#endif

 }

 static void oia_draw_shift_state(cairo_t *cr, GdkRectangle *r)
 {
	// *	M-37...36	Shift Status

	r->x = (r->width - (37*terminal_font_info.width));
	r->width = terminal_font_info.width*2;

	oia_clear_rect(cr,r);

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
		double b,x,y,w,h,l;

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

		cairo_set_3270_color(cr,TERMINAL_COLOR_SHIFT_STATE);

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

 static void oia_draw_typeahead_state(cairo_t *cr, GdkRectangle *r)
 {
 	// * M-33 Typeahead indication ("T" or blank)
	r->x = (r->width - (34*terminal_font_info.width));
	r->width = terminal_font_info.width;
	r->height = terminal_font_info.height;

	oia_clear_rect(cr,r);

	if(oia_flag[OIA_FLAG_TYPEAHEAD])
		oia_show_text(cr,r,"T",TERMINAL_COLOR_TYPEAHEAD_STATE);
 }

#if GTK_CHECK_VERSION(2,16,0)
 static void oia_draw_caps_state(cairo_t *cr, GdkRectangle *r)
 {
	r->x = (r->width - (39*terminal_font_info.width));
	r->width = terminal_font_info.width;
	r->height = terminal_font_info.height;

	oia_clear_rect(cr,r);

	if(oia_caps_state)
		oia_show_text(cr,r,"A",TERMINAL_COLOR_OIA_INDICATORS);

 }
#endif // GTK 2.16.0

 static void oia_draw_ssl_state(cairo_t *cr, GdkRectangle *r)
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
	// * M-44...42	SSL Status
	r->x = (r->width - (44*terminal_font_info.width));
	r->width = terminal_font_info.width *2;
	r->height = terminal_font_info.height;

	oia_clear_rect(cr,r);

	if(query_secure_connection(hSession))
	{
		oia_show_text(cr,r,"S",TERMINAL_COLOR_SSL_STATE);
	}
 }

 static void oia_insert_indicator(cairo_t *cr, GdkRectangle *r)
 {
	r->x = (r->width - (32*terminal_font_info.width));
	r->width = terminal_font_info.width;
	r->height = fontHeight;

	oia_clear_rect(cr,r);

	if(Toggled(INSERT))
		oia_show_text(cr,r,"I",TERMINAL_COLOR_OIA_INDICATORS);
 }

 static void oia_script_indicator(cairo_t *cr, GdkRectangle *r)
 {
	r->x = (r->width - (30*terminal_font_info.width));
	r->width = fontWidth;
	r->height = fontHeight;

	oia_clear_rect(cr,r);

	if(oia_script_state != SCRIPT_STATE_NONE && oia_script_blink)
		oia_show_text(cr,r,"S",TERMINAL_COLOR_OIA_SCRIPT_STATE);

 }

 static void oia_four_in_a_square(cairo_t *cr, GdkRectangle *r)
 {
	//  0          "4" in a square
	r->x = left_margin;
	r->width = terminal_font_info.width+2;
	r->height = terminal_font_info.height;

	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, r->x, r->y+1, r->width,r->height);
	cairo_fill(cr);

	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_INDICATORS);
	draw_border(cr,r);

	cairo_move_to(cr,r->x+2,r->y+fontAscent);
	cairo_show_text(cr,"4");
 }

 static void oia_undera_indicator(cairo_t *cr, GdkRectangle *r)
 {
	//  1          "A" underlined
	r->x = (left_margin+terminal_font_info.width+4);
	r->width = terminal_font_info.width;
	r->height = terminal_font_info.height;

	oia_clear_rect(cr,r);

	if(oia_flag[OIA_FLAG_UNDERA])
	{
		oia_show_text(cr,r,(IN_E) ? "B" : "A", TERMINAL_COLOR_OIA_INDICATORS);

		cairo_move_to(cr,r->x,r->y+fontAscent+(fontDescent/2));
		cairo_rel_line_to(cr,terminal_font_info.width,0);
		cairo_stroke(cr);

//		cairo_rectangle(cr, r->x, r->y+fontAscent + (fontDescent/2), fontWidth, sl);
//		cairo_fill(cr);
	}


 }

 static void oia_connection_status(cairo_t *cr, GdkRectangle *r)
 {
	// 2          solid box if connected, "?" in a box if not
	r->x = (left_margin+(terminal_font_info.width*2)+6);
	r->width = terminal_font_info.width+2;
	r->height = terminal_font_info.height;

	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_BACKGROUND);
	cairo_rectangle(cr, r->x, r->y+1, r->width,r->height);
	cairo_fill(cr);

	cairo_set_3270_color(cr,TERMINAL_COLOR_OIA_INDICATORS);

	Trace("%s box_solid=%s", __FUNCTION__, oia_flag[OIA_FLAG_BOXSOLID] ? "Yes" : "No" );

	if(IN_ANSI)
	{
		draw_border(cr,r);
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
		draw_border(cr,r);
		cairo_move_to(cr,r->x+2,r->y+terminal_font_info.ascent);
		cairo_show_text(cr,"S");
	}
	else
	{
		draw_border(cr,r);
		cairo_move_to(cr,r->x+2,r->y+terminal_font_info.ascent);
		cairo_show_text(cr,"?");
	}

 }


 static void oia_cursor_position(cairo_t *cr, GdkRectangle *r)
 {
	// M-7..M		cursor position (rrr/ccc or blank)
 	int width = 8*fontWidth;

	r->x = (r->width - width);
	r->width = width;

	oia_clear_rect(cr,r);

	if(Toggled(CURSOR_POS))
	{
		gchar *text;

		text = g_strdup_printf("%03d/%03d",(cursor_position/terminal_cols)+1,(cursor_position%terminal_cols)+1);
		oia_show_text(cr,r,text,TERMINAL_COLOR_OIA_CURSOR);

		g_free(text);
	}

 }

 static void oia_lu_name(cairo_t *cr, GdkRectangle *r)
 {
 	const char *luname = get_connected_lu(hSession);

 	r->x = (r->width - (fontWidth*28));
 	r->width = fontWidth*11;

	oia_clear_rect(cr,r);

	if(luname)
		oia_show_text(cr, r,luname, TERMINAL_COLOR_OIA_LUNAME);
 }

 static void oia_update_timer(cairo_t *cr, GdkRectangle *r)
 {
 	r->x = (r->width - (fontWidth*14));
 	r->width = fontWidth*5;

	oia_clear_rect(cr,r);

	if(oia_timer >= 0)
	{
		gchar *text;
		text = g_strdup_printf("%02d:%02d",oia_timer/60,oia_timer % 60);
		oia_show_text(cr,r,text,TERMINAL_COLOR_OIA_TIMER);
		g_free(text);
	}
 }

 static void oia_update_spinner(cairo_t *cr, GdkRectangle *r)
 {
 	r->x = (r->width - (fontWidth*16));
	r->width = fontWidth*2;
	r->height = fontHeight;

	oia_clear_icon(cr,r);
	r->x++;
	r->y++;
	r->width--;
	r->height--;

	if(r->width < 2)
		return;

	if(oia_timer >= 0)
		oia_spinner_step = oia_draw_spinner(cr,r,oia_spinner_step);

 }


 static void oia_message_area(cairo_t *cr, GdkRectangle *r)
 {
 	// From 6 to M - 46 - Status message
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
	} message[STATUS_CODE_USER] =
 	{
		OIA_MESSAGE(	STATUS_CODE_BLANK,
						TERMINAL_COLOR_OIA_STATUS_OK,
						"" ),

		OIA_MESSAGE(	STATUS_CODE_SYSWAIT,
						TERMINAL_COLOR_OIA_STATUS_OK,
						N_( "X System" ) ),

		OIA_MESSAGE(	STATUS_CODE_TWAIT,
						TERMINAL_COLOR_OIA_STATUS_OK,
						N_( "X Wait" ) ),

		OIA_MESSAGE(	STATUS_CODE_CONNECTED,
						TERMINAL_COLOR_OIA_STATUS_OK,
						NULL ),

		OIA_MESSAGE(	STATUS_CODE_DISCONNECTED,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X Not Connected" ) ),

		OIA_MESSAGE(	STATUS_CODE_AWAITING_FIRST,
						TERMINAL_COLOR_OIA_STATUS_OK,
						N_( "X" ) ),

		OIA_MESSAGE(	STATUS_CODE_MINUS,
						TERMINAL_COLOR_OIA_STATUS_OK,
						N_( "X -f" ) ),

		OIA_MESSAGE(	STATUS_CODE_PROTECTED,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X Protected" ) ),

		OIA_MESSAGE(	STATUS_CODE_NUMERIC,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X Numeric" ) ),

		OIA_MESSAGE(	STATUS_CODE_OVERFLOW,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X Overflow" ) ),

		OIA_MESSAGE(	STATUS_CODE_INHIBIT,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X Inhibit" ) ),

		OIA_MESSAGE(	STATUS_CODE_KYBDLOCK,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						NULL ),

		OIA_MESSAGE(	STATUS_CODE_X,
						TERMINAL_COLOR_OIA_STATUS_INVALID,
						N_( "X" ) ),

		OIA_MESSAGE(	STATUS_CODE_RESOLVING,
						TERMINAL_COLOR_OIA_STATUS_WARNING,
						N_( "X Resolving" ) ),

		OIA_MESSAGE(	STATUS_CODE_CONNECTING,
						TERMINAL_COLOR_OIA_STATUS_WARNING,
						N_( "X Connecting" ) ),


	};

	static gint current_message_id = 0;

	const gchar *msg = NULL;

	if(message[terminal_message_id].string)
		current_message_id = terminal_message_id;

	msg = message[current_message_id].string;

	r->x = left_margin + (terminal_font_info.width * 7);
	r->width = (r->width - (terminal_font_info.width * 45)) - r->x;

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

