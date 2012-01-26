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


#include "gui.h"
#include <lib3270/config.h>
#include <malloc.h>
#include <string.h>
#include <errno.h>

#ifdef WIN32
	#include <windows.h>
#endif

/*---[ Prototipes ]----------------------------------------------------------------------------------------*/

/*---[ Statics ]-------------------------------------------------------------------------------------------*/

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

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