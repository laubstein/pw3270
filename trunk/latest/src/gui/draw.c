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
 * Este programa está nomeado como draw.c e possui 1301 linhas de código.
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


#include <lib3270/config.h>

#include "gui.h"
#include "oia.h"
#include "fonts.h"

#ifndef __APPLE__
	#include <malloc.h>
#endif

#include <string.h>
#include <errno.h>

#ifdef WIN32
	#include <windows.h>
#endif

/*---[ Prototipes ]----------------------------------------------------------------------------------------*/

/*---[ Globals ]-------------------------------------------------------------------------------------------*/

 GdkColor color[TERMINAL_COLOR_COUNT+1];

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

#ifdef ENABLE_PANGO
 /**
  * Draw entire buffer.
  *
  * @param	widget	Widget to be used as reference.
  * @param	clr		List of colors to be used when drawing.
  * @param	draw	The image destination.
  *
  */
 int DrawScreen(GdkColor *clr, GdkDrawable *draw)
 {
	GdkGC			*gc;
	ELEMENT			*el			= screen;
	int				x;
	int				y;
	int				row;
	int				col;
	int				width;
	int				height;

	if(!(el && draw))
		return -1;

	gc = gdk_gc_new(draw);

	// Fill pixmap with background color
	gdk_drawable_get_size(draw,&width,&height);

	gdk_gc_set_foreground(gc,color);
	gdk_draw_rectangle(draw,gc,1,0,0,width,height);

	// TODO (perry#1#): Find a faster way to draw text
	y = top_margin;
	for(row = 0; row < terminal_rows;row++)
	{
		x = left_margin;
		for(col = 0; col < terminal_cols;col++)
		{
			DrawElement(draw,clr,gc,x,y,el);
			el++;
			x += fontWidth;
		}
		y += fontHeight;
	}

	gdk_gc_destroy(gc);

	RedrawCursor();

	return 0;

 }

 static void DrawCorner(GdkDrawable *drawable, GdkGC *gc, int x1, int y1, int x2, int y2, int x3, int y3)
 {
	GdkPoint points[] = { { x1, y1 }, { x2, y2 }, { x3, y3 } };
	gdk_draw_lines(drawable,gc,points,3);
 }

 static void DrawExtendedChar(GdkDrawable *draw, GdkColor *fg, GdkColor *bg, GdkGC *gc, int x, int y, ELEMENT *el)
 {
	gdk_gc_set_foreground(gc,bg);
	gdk_draw_rectangle(draw,gc,TRUE,x,y,fontWidth,fontHeight);
	gdk_gc_set_foreground(gc,fg);

	switch(el->cg)
	{
//	case 0xaf: // CG 0xd1, degree
//		break;

	case 0xd4: // CG 0xac, LR corner
		DrawCorner(draw, gc, x, y+(fontHeight >> 1), x+(fontWidth >> 1), y+(fontHeight >> 1), x+(fontWidth >> 1), y);
		break;

	case 0xd5: // CG 0xad, UR corner
		DrawCorner(draw, gc, x, y+(fontHeight >> 1), x+(fontWidth >> 1), y+(fontHeight >> 1), x+(fontWidth >> 1), y+fontHeight);
		break;

	case 0xc5: // CG 0xa4, UL corner
		DrawCorner(draw, gc, x+fontWidth, y+(fontHeight >> 1), x+(fontWidth >> 1), y+(fontHeight >> 1), x+(fontWidth >> 1), y+fontHeight);
		break;

	case 0xc4: // CG 0xa3, LL corner
		DrawCorner(draw, gc, x+fontWidth, y+(fontHeight >> 1), x+(fontWidth >> 1), y+(fontHeight >> 1), x+(fontWidth >> 1), y);
		break;

	case 0xd3: // CG 0xab, plus
		gdk_draw_line(draw,gc,x,y+(fontHeight >> 1),x+fontWidth,y+(fontHeight >> 1));
		gdk_draw_line(draw,gc,x+(fontWidth >> 1),y,x+(fontWidth >> 1),y+fontHeight);
		break;

	case 0xa2: // CG 0x92, horizontal line
		y += (fontHeight >> 1);
		gdk_draw_line(draw,gc,x,y,x+fontWidth,y);
		break;

	case 0xc6: // CG 0xa5, left tee
		gdk_draw_line(draw,gc,x+(fontWidth >> 1),y+(fontHeight >> 1),x+fontWidth,y+(fontHeight >> 1));
		gdk_draw_line(draw,gc,x+(fontWidth >> 1),y,x+(fontWidth >> 1),y+fontHeight);
		break;

	case 0xd6: // CG 0xae, right tee
		gdk_draw_line(draw,gc,x,y+(fontHeight >> 1),x+(fontWidth >> 1),y+(fontHeight >> 1));
		gdk_draw_line(draw,gc,x+(fontWidth >> 1),y,x+(fontWidth >> 1),y+fontHeight);
		break;

	case 0xc7: // CG 0xa6, bottom tee
		gdk_draw_line(draw,gc,x,y+(fontHeight >> 1),x+fontWidth,y+(fontHeight >> 1));
		gdk_draw_line(draw,gc,x+(fontWidth >> 1),y,x+(fontWidth >> 1),y+(fontHeight>>1));
		break;

	case 0xd7: // CG 0xaf, top tee
		gdk_draw_line(draw,gc,x,y+(fontHeight >> 1),x+fontWidth,y+(fontHeight >> 1));
		gdk_draw_line(draw,gc,x+(fontWidth >> 1),y+(fontHeight >> 1),x+(fontWidth >> 1),y+fontHeight);
		break;

//	case 0xbf: // CG 0x15b, stile
//		pango_layout_set_text(getPangoLayout(),"\u2352",-1);
//		gdk_draw_layout_with_colors(draw,gc,x,y,getPangoLayout(),fg,bg);
//		break;

	case 0x85: // CG 0x184, vertical line
	case 0xbf:
		x += (fontWidth >> 1);
		gdk_draw_line(draw,gc,x,y,x,y+fontHeight);
		break;

	case 0x8c: // CG 0xf7, less or equal
		pango_layout_set_text(getPangoLayout(TEXT_LAYOUT_NORMAL),"≤",-1);
		gdk_draw_layout_with_colors(draw,gc,x,y,getPangoLayout(TEXT_LAYOUT_NORMAL),fg,bg);
		break;

	case 0xae: // CG 0xd9, greater or equal
		pango_layout_set_text(getPangoLayout(TEXT_LAYOUT_NORMAL),"≥",-1);
		gdk_draw_layout_with_colors(draw,gc,x,y,getPangoLayout(TEXT_LAYOUT_NORMAL),fg,bg);
		break;

	case 0xbe: // CG 0x3e, not equal
		pango_layout_set_text(getPangoLayout(TEXT_LAYOUT_NORMAL),"≠",-1);
		gdk_draw_layout_with_colors(draw,gc,x,y,getPangoLayout(TEXT_LAYOUT_NORMAL),fg,bg);
		break;

//	case 0xa3: // CG 0x93, bullet
//		break;

	case 0xad:
		pango_layout_set_text(getPangoLayout(TEXT_LAYOUT_NORMAL),"[",-1);
		gdk_draw_layout_with_colors(draw,gc,x,y,getPangoLayout(TEXT_LAYOUT_NORMAL),fg,bg);
		break;

	case 0xbd:
		pango_layout_set_text(getPangoLayout(TEXT_LAYOUT_NORMAL),"]",-1);
		gdk_draw_layout_with_colors(draw,gc,x,y,getPangoLayout(TEXT_LAYOUT_NORMAL),fg,bg);
		break;

	default:	// Unknown char, draw "?"
		Trace("Unexpected extended char: %02x",el->cg);
		pango_layout_set_text(getPangoLayout(TEXT_LAYOUT_NORMAL),"?",-1);
		gdk_draw_layout_with_colors(draw,gc,x,y,getPangoLayout(TEXT_LAYOUT_NORMAL),fg,bg);
	}
 }

 static void DrawTextChar(GdkDrawable *draw, GdkColor *fg, GdkColor *bg, GdkGC *gc, int x, int y, ELEMENT *el)
 {
	PangoLayout *layout;

	if(TOGGLED_UNDERLINE && (el->fg & COLOR_ATTR_UNDERLINE))
		layout = getPangoLayout(TEXT_LAYOUT_UNDERLINE);
	else
		layout = getPangoLayout(TEXT_LAYOUT_NORMAL);

	if(el->ch && *el->ch != ' ' && *el->ch)
	{
		// Non space
		pango_layout_set_text(layout,el->ch,-1);
		gdk_draw_layout_with_colors(draw,gc,x,y,layout,fg,bg);
	}
	else if(el->fg & COLOR_ATTR_UNDERLINE)
	{
		// It's underlined, use pango
		pango_layout_set_text(layout," ",-1);
		gdk_draw_layout_with_colors(draw,gc,x,y,layout,fg,bg);
	}
	else
	{
		// Empty space, just clear it
		gdk_gc_set_foreground(gc,bg);
		gdk_draw_rectangle(draw,gc,TRUE,x,y,fontWidth,fontHeight);
	}
 }

 void DrawElement(GdkDrawable *draw, GdkColor *clr, GdkGC *gc, int x, int y, ELEMENT *el)
 {
 	// http://www.guntherkrauss.de/computer/xml/daten/edicode.html
	short			fg;
	short 			bg;

 	if(!draw)
		return;

	if(el->status & ELEMENT_STATUS_SELECTED)
	{
		fg = TERMINAL_COLOR_SELECTED_FG;
		bg = TERMINAL_COLOR_SELECTED_BG;
	}
	else
	{
		fg = (el->fg & 0xFF);
		bg = (el->bg & 0xFF);
	}

	if(!el->cg)
	{
		// Standard char or empty space, draw directly
		DrawTextChar(draw, clr+fg, clr+bg, gc, x, y, el);
	}
	else
	{
		DrawExtendedChar(draw, clr+fg, clr+bg, gc, x, y, el);
	}

	if(el->status & ELEMENT_STATUS_SELECTED)
	{
		gdk_gc_set_foreground(gc,clr+TERMINAL_COLOR_SELECTED_BORDER);

		if(el->status & SELECTION_BOX_TOP)
			gdk_draw_line(draw,gc,x,y,x+fontWidth,y);

		if(el->status & SELECTION_BOX_LEFT)
			gdk_draw_line(draw,gc,x,y,x,y+(fontHeight-1));

		if(el->status & SELECTION_BOX_BOTTOM)
			gdk_draw_line(draw,gc,x,y+(fontHeight-1),x+fontWidth,y+(fontHeight-1));

		if(el->status & SELECTION_BOX_RIGHT)
			gdk_draw_line(draw,gc,x+(fontWidth-1),y,x+(fontWidth-1),y+(fontHeight-1));

	}
 }

#else // ENABLE_PANGO

 void draw_element(cairo_t *cr, int x, int y, int baseline, int addr, GdkColor *clr)
 {
	if(clr)
	{
		short	fg;
		short 	bg;

		if(screen[addr].status & ELEMENT_STATUS_SELECTED)
		{
			fg = TERMINAL_COLOR_SELECTED_FG;
			bg = TERMINAL_COLOR_SELECTED_BG;
		}
		else
		{
			fg = (screen[addr].fg & 0xFF);
			bg = (screen[addr].bg & 0xFF);
		}

		cairo_set_3270_color(cr,bg);
		cairo_rectangle(cr, x, y, fontWidth, fontHeight);
		cairo_fill(cr);

		cairo_set_3270_color(cr,fg);
	}

	if(TOGGLED_UNDERLINE && (screen[addr].fg & COLOR_ATTR_UNDERLINE))
	{
		// Draw underline
		int sl = (fontDescent/3);
		if(sl < 1)
			sl = 1;
		cairo_rectangle(cr, x, baseline + (fontDescent/2), fontWidth, sl);
		cairo_fill(cr);
	}

	if(screen[addr].cg)
	{
		// Graphics char
	}
	else if(*screen[addr].ch != ' ' && *screen[addr].ch)
	{
		// Text char
		cairo_move_to(cr,x,baseline);
		cairo_show_text(cr,screen[addr].ch);
	}

 }

 void draw_region(cairo_t *cr, int bstart, int bend, GdkColor *clr)
 {
 	int addr;
 	int col	= (bstart % terminal_cols);
 	int x	= left_margin + (col * fontWidth);
 	int y	= top_margin  + ((bstart / terminal_cols) * fontHeight);
 	int baseline = y + fontAscent;	/**< Baseline for font drawing; it's not the same as font Height */

	for(addr = bstart; addr <= bend; addr++)
	{
		draw_element(cr,x,y,baseline,addr,clr);
		if(++col >= terminal_cols)
		{
			col  = 0;
			x    = left_margin;
			y   += fontHeight;
			baseline += fontHeight;
		}
		else
		{
			x += fontWidth;
		}
	}
 }

/**
 * Get cached graphics context
 *
 */
 GdkGC * get_terminal_cached_gc(void)
 {
 	return GDK_GC(g_object_get_data(G_OBJECT(get_terminal_pixmap()),"cached_gc"));
 }


/**
 * Draw entire terminal contents
 *
 */
 void update_terminal_contents(void)
 {
 	int 	width;
 	int 	height;
 	cairo_t *cr	= get_terminal_cairo_context();

	gdk_drawable_get_size(get_terminal_pixmap(),&width,&height);

	cairo_set_3270_color(cr,TERMINAL_COLOR_BACKGROUND);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);

	draw_region(cr,0,(terminal_cols * terminal_rows)-1,color);
	draw_oia(cr,get_terminal_cached_gc());

	cairo_destroy(cr);

	update_cursor_pixmap();

 }

/**
 * Redraw terminal widget.
 *
 * Update all font information & sizes and refresh entire terminal window.
 *
 */
 void action_Redraw(void)
 {
	if(valid_terminal_window())
	{
		gint width;
		gint height;

		update_font_info(terminal);
		gdk_drawable_get_size(terminal->window,&width,&height);
		update_screen_size(terminal, width, height);
		update_cursor_info();
		update_terminal_contents();
	}
	gtk_widget_queue_draw(terminal);
 }

#endif // ENABLE_PANGO

 cairo_t * get_terminal_cairo_context(void)
 {
 	cairo_t *cr = gdk_cairo_create(pixmap_terminal);

	cairo_set_font_face(cr,terminal_font_info.face);
//	cairo_set_font_size(cr,terminal_font_info.size);
	cairo_set_font_matrix(cr,terminal_font_info.matrix);

	return cr;
 }
