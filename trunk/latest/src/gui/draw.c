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

//#ifndef __APPLE__
//	#include <malloc.h>
//#endif

#include <string.h>
#include <errno.h>

#ifdef WIN32
	#include <windows.h>
#endif

/*---[ Prototipes ]----------------------------------------------------------------------------------------*/

/*---[ Globals ]-------------------------------------------------------------------------------------------*/

 GdkColor color[TERMINAL_COLOR_COUNT+1];

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

 static void draw_cg(cairo_t *cr, unsigned short ch, int x, int y, int width, int height)
 {
	switch(ch)
	{
	case 0xd3: // CG 0xab, plus
		cairo_move_to(cr,x+(width/2),y);
		cairo_rel_line_to(cr,0,height);
		cairo_move_to(cr,x,y+(height/2));
		cairo_rel_line_to(cr,width,0);
		break;

	case 0xa2: // CG 0x92, horizontal line
		cairo_move_to(cr,x,y+(height/2));
		cairo_rel_line_to(cr,width,0);
		break;

	case 0x85: // CG 0x184, vertical line
		cairo_move_to(cr,x+(width/2),y);
		cairo_rel_line_to(cr,0,height);
		break;

	case 0xd4: // CG 0xac, LR corner
		cairo_move_to(cr,x, y+(height/2));
		cairo_rel_line_to(cr,width/2,0);
		cairo_rel_line_to(cr,0,-(height/2));
		break;

	case 0xd5: // CG 0xad, UR corner
		cairo_move_to(cr,x, y+(height/2));
		cairo_rel_line_to(cr,width/2,0);
		cairo_rel_line_to(cr,0,height/2);
		break;

	case 0xc5: // CG 0xa4, UL corner
		cairo_move_to(cr,x+width,y+(height/2));
		cairo_rel_line_to(cr,-(width/2),0);
		cairo_rel_line_to(cr,0,(height/2));
		break;

	case 0xc4: // CG 0xa3, LL corner
		cairo_move_to(cr,x+width,y+(height/2));
		cairo_rel_line_to(cr,-(width/2),0);
		cairo_rel_line_to(cr,0,-(height/2));
		break;

	case 0xc6: // CG 0xa5, left tee
		cairo_move_to(cr,x+(width/2),y+(height/2));
		cairo_rel_line_to(cr,width/2,0);
		cairo_move_to(cr,x+(width/2),y);
		cairo_rel_line_to(cr,0,height);
		break;

	case 0xd6: // CG 0xae, right tee
		cairo_move_to(cr,x+(width/2),y+(height/2));
		cairo_rel_line_to(cr,-(width/2),0);
		cairo_move_to(cr,x+(width/2),y);
		cairo_rel_line_to(cr,0,height);
		break;

	case 0xc7: // CG 0xa6, bottom tee
		cairo_move_to(cr,x+(width/2),y+(height/2));
		cairo_rel_line_to(cr,0,-(height/2));
		cairo_move_to(cr,x,y+(height/2));
		cairo_rel_line_to(cr,width,0);
		break;

	case 0xd7: // CG 0xaf, top tee
		cairo_move_to(cr,x+(width/2),y+(height/2));
		cairo_rel_line_to(cr,0,height/2);
		cairo_move_to(cr,x,y+(height/2));
		cairo_rel_line_to(cr,width,0);
		break;

	default:
		cairo_rectangle(cr, x+1, y+1, width-2, height-2);
	}

	cairo_stroke(cr);
 }

 void draw_element(cairo_t *cr, PW3270_FONT_INFO *font, int x, int y, int baseline, int addr, GdkColor *clr)
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
		cairo_rectangle(cr, x, y, font->width, font->spacing);
		cairo_fill(cr);

		cairo_set_3270_color(cr,fg);
	}

	if(TOGGLED_UNDERLINE && (screen[addr].fg & COLOR_ATTR_UNDERLINE))
	{
		// Draw underline
		int sl = (fontDescent/3);
		if(sl < 1)
			sl = 1;
		cairo_rectangle(cr, x, baseline + (font->descent/2), font->width, sl);
		cairo_fill(cr);
	}

	if(screen[addr].cg)
	{
		// Graphics char
		draw_cg(cr, screen[addr].cg, x, y,font->width, font->height);
	}
	else if(*screen[addr].ch != ' ' && *screen[addr].ch)
	{
		// Text char
		cairo_move_to(cr,x,baseline);
		cairo_show_text(cr,screen[addr].ch);
	}

	if(screen[addr].status & ELEMENT_STATUS_SELECTED)
	{
		cairo_set_3270_color(cr,TERMINAL_COLOR_SELECTED_BORDER);

		if(screen[addr].status & SELECTION_BOX_TOP)
		{
			cairo_rectangle(cr, x, y, font->width, 1);
			cairo_fill(cr);
		}

		if(screen[addr].status & SELECTION_BOX_BOTTOM)
		{
			cairo_rectangle(cr, x, y+(font->spacing-1), font->width, 1);
			cairo_fill(cr);
		}

		if(screen[addr].status & SELECTION_BOX_LEFT)
		{
			cairo_rectangle(cr, x, y, 1, font->spacing);
			cairo_fill(cr);
		}

		if(screen[addr].status & SELECTION_BOX_RIGHT)
		{
			cairo_rectangle(cr, x+(font->width-1), y, 1, font->spacing);
			cairo_fill(cr);
		}
	}

 }

 void draw_region(cairo_t *cr, int bstart, int bend, GdkColor *clr, GdkRectangle *r)
 {
 	int addr;
 	int col	= (bstart % terminal_cols);
 	int x	= left_margin + (col * fontWidth);
 	int y	= top_margin  + ((bstart / terminal_cols) * terminal_font_info.spacing);
 	int baseline = y + terminal_font_info.ascent;

	memset(r,0,sizeof(GdkRectangle));
	r->x = r->width = x;
	r->y = r->height = y;

	for(addr = bstart; addr <= bend; addr++)
	{
		draw_element(cr,&terminal_font_info,x,y,baseline,addr,clr);
		if(++col >= terminal_cols)
		{
			col  = 0;
			x    = left_margin;
			y   += terminal_font_info.spacing;
			baseline += terminal_font_info.spacing;
		}
		else
		{
			x += terminal_font_info.width;
		}

		if(x > r->width)
			r->width = x;

		if(y > r->height)
			r->height = y;

	}

	r->width  = (r->width - r->x)  + terminal_font_info.width;
	r->height = (r->height - r->y) + terminal_font_info.spacing;
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
 	int 		width;
 	int 		 height;
 	GdkRectangle r;
 	cairo_t *cr	= get_terminal_cairo_context();

	gdk_drawable_get_size(get_terminal_pixmap(),&width,&height);

	cairo_set_3270_color(cr,TERMINAL_COLOR_BACKGROUND);
	cairo_rectangle(cr, 0, 0, width, height);
	cairo_fill(cr);

	draw_region(cr,0,(terminal_cols * terminal_rows)-1,color,&r);
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

		cairo_t	*cr			= gdk_cairo_create(terminal->window);
		gchar	*fontname	= GetString("Terminal","Font","Courier");

		update_font_info(cr, fontname, &terminal_font_info);

		gtk_widget_set_size_request(terminal, terminal_cols*terminal_font_info.width, ((terminal_rows+2)*terminal_font_info.spacing));

		g_free(fontname);
		cairo_destroy(cr);

		gdk_drawable_get_size(terminal->window,&width,&height);
		update_terminal_font_size(width, height);
		update_cursor_info();
		update_terminal_contents();
	}
	gtk_widget_queue_draw(terminal);
 }

 cairo_t * get_terminal_cairo_context(void)
 {
 	cairo_t *cr = gdk_cairo_create(pixmap_terminal);

	if(terminal_font_info.font)
	{
		cairo_set_scaled_font(cr,terminal_font_info.font);
	}
	else
	{
		cairo_set_font_face(cr,terminal_font_info.face);
		cairo_set_font_matrix(cr,terminal_font_info.matrix);
		terminal_font_info.font = cairo_scaled_font_reference(cairo_get_scaled_font(cr));
	}

	return cr;
 }
