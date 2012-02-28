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
 * Este programa está nomeado como cursor.c e possui - linhas de código.
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
#include "oia.h"

/*---[ Statics ]------------------------------------------------------------------------------------------------*/



/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 GdkPixmap		* pixmap_cursor  		= NULL;
 gint			  cMode					= CURSOR_MODE_ENABLED|CURSOR_MODE_BASE|CURSOR_MODE_SHOW;
 gint			  cursor_position		= 0;
 GdkRectangle	  rCursor;
 gboolean		  cursor_blink			= FALSE;

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

 void queue_draw_cursor(void)
 {

	if(cMode & CURSOR_MODE_CROSS)
	{
		int	 width;
		int	 height;

		gdk_drawable_get_size(terminal->window,&width,&height);

		gtk_widget_queue_draw_area(terminal,0,rCursor.y+fontAscent,width,1);
		gtk_widget_queue_draw_area(terminal,rCursor.x,0,1,height);

	}

	gtk_widget_queue_draw_area(terminal,rCursor.x,rCursor.y,rCursor.width,rCursor.height);

 }

 void update_cursor_pixmap(void)
 {
 	cairo_t *cr;

 	if(!get_cursor_pixmap())
		return;

 	cr = gdk_cairo_create(get_cursor_pixmap());

	cairo_set_font_face(cr,terminal_font_info.face);
	cairo_set_font_matrix(cr,terminal_font_info.matrix);

	cairo_set_3270_color(cr,TERMINAL_COLOR_CURSOR_BACKGROUND);
	cairo_rectangle(cr, 0, 0, rCursor.width, rCursor.height);
	cairo_fill(cr);

	cairo_set_3270_color(cr,TERMINAL_COLOR_CURSOR_FOREGROUND);
	draw_element(cr,&terminal_font_info,0,0,terminal_font_info.ascent,cursor_position,NULL);

    cairo_destroy(cr);
 }

 void update_cursor_info(void)
 {
 	int row = cursor_position / screen->cols;
 	int col = cursor_position % screen->cols;

	rCursor.x 		= view.left + (col * terminal_font_info.width);
	rCursor.y 		= view.top + (row * terminal_font_info.spacing);
	rCursor.width 	= fontWidth;
	rCursor.height	= terminal_font_info.height;
 }

 void update_cursor_position(H3270 *session, unsigned short row, unsigned short col, unsigned char c, unsigned short attr)
 {
 	int addr = (row * screen->cols) + col;

	if(addr == cursor_position)
		return;

	cursor_position = addr;

	gtk_im_context_reset(input_method);

 	if(screen_updates_enabled)
 	{
		int width;
		int height;

		cMode |= CURSOR_MODE_SHOW;

		gdk_drawable_get_size(terminal->window,&width,&height);
		gtk_widget_queue_draw_area(terminal,0,rCursor.y+fontAscent,width,1);
		gtk_widget_queue_draw_area(terminal,rCursor.x,0,1,height);
		gtk_widget_queue_draw_area(terminal,rCursor.x,rCursor.y,rCursor.width,rCursor.height);

 		update_cursor_info();
 		update_cursor_pixmap();
 		gtk_im_context_set_cursor_location(input_method,&rCursor);

		gtk_widget_queue_draw_area(terminal,0,rCursor.y+fontAscent,width,1);
		gtk_widget_queue_draw_area(terminal,rCursor.x,0,1,height);
		gtk_widget_queue_draw_area(terminal,rCursor.x,rCursor.y,rCursor.width,rCursor.height);

		update_oia_element(OIA_ELEMENT_CURSOR_POSITION);
 	}
 }

