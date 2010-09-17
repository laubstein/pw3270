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
 * Este programa está nomeado como fonts.c e possui - linhas de código.
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

/*---[ Globals ]------------------------------------------------------------------------------------------------*/

 struct font_size
 {
 	double size;
 	gint	ascent;
 	gint	descent;
 	gint	width;
 	gint	height;
 } *font_size = NULL;

 PW3270_FONT_INFO terminal_font_info = { 0 };

/*---[ Implement ]----------------------------------------------------------------------------------------------*/

/**
 * Update font & coordinates for screen elements.
 *
 * @param widget	Terminal widget.
 * @param width	New widget width.
 * @param height	New widget height.
 *
 * @return TRUE if the image needs update
 */
 gboolean update_screen_size(GtkWidget *widget, gint width, gint height)
 {
 	int size = -1;
 	int f;

 	for(f=0;font_size[f].size;f++)
 	{
		if( ((font_size[f].height*(terminal_rows+1))+2) < height && (font_size[f].width*terminal_cols) < width )
			size = f;
 	}

 	if(size < 0)
		size = 0;

	terminal_font_info.size		= font_size[size].size;
	terminal_font_info.width	= font_size[size].width;
	terminal_font_info.height 	= font_size[size].height;
	terminal_font_info.descent	= font_size[size].descent;
	terminal_font_info.ascent	= font_size[size].ascent;

//	Trace("%s - screen=%p",__FUNCTION__,screen);

	// Center image
	left_margin = (width >> 1) - ((terminal_cols * fontWidth) >> 1);
	if(left_margin < 0)
		left_margin = 0;

	top_margin = (height >> 1) - (((terminal_rows+1) * fontHeight) >> 1);
	if(top_margin < 0)
		top_margin = 0;

	return TRUE;
 }

/**
 * Load terminal font info
 *
 */
 void update_font_info(GtkWidget *widget)
 {
 	int		f;
 	cairo_t	*cr	= gdk_cairo_create(widget->window);
 	gchar	*fontname = GetString("Terminal","Font","Courier");

	if(terminal_font_info.face)
		cairo_font_face_destroy(terminal_font_info.face);

	terminal_font_info.face = cairo_toy_font_face_create(fontname,CAIRO_FONT_SLANT_NORMAL,TOGGLED_BOLD ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);

	/* Load font sizes */
	cairo_set_font_face(cr,terminal_font_info.face);

 	for(f=0;font_size[f].size;f++)
 	{
		cairo_font_extents_t sz;

 		cairo_set_font_size(cr,font_size[f].size);
 		cairo_font_extents(cr,&sz);

 		font_size[f].width  = (int) sz.max_x_advance;
 		font_size[f].height = (int) sz.height;
 		font_size[f].ascent = (int) sz.ascent;
 		font_size[f].descent = (int) sz.descent;

		Trace("%s(%d): %dx%d descent: %d ascent: %d",fontname,(int) font_size[f].size, font_size[f].width, font_size[f].height, font_size[f].descent, font_size[f].ascent);

 	}

	terminal_font_info.width	= font_size->width;
	terminal_font_info.height	= font_size->height;
	terminal_font_info.descent	= font_size->descent;
	terminal_font_info.ascent	= font_size->ascent;
	terminal_font_info.size		= font_size->size;

 	Trace("Minimum terminal size is %dx%d",terminal_cols*font_size->width, (terminal_rows+1)*font_size->height);

	gtk_widget_set_size_request(widget, terminal_cols*font_size->width, (terminal_rows+1)*font_size->height);

 	g_free(fontname);
	cairo_destroy(cr);

 }


/**
 * Load font sizes.
 *
 * @param widget	Widget to set.
 *
 */
 void init_terminal_font(GtkWidget *widget)
 {
 	static const gchar *default_sizes = "6,7,8,9,10,11,12,13,14,16,18,20,22,24,26,28,32,36,40,48,56,64,72";

 	gchar 				*list = GetString("Terminal","FontSizes",default_sizes);
 	gchar 				**vlr;
 	gint				sz;
 	int					f;

	Trace("%s",__FUNCTION__);

	memset(&terminal_font_info,0,sizeof(terminal_font_info));
	terminal_font_info.width	= 10;
	terminal_font_info.height	= 12;
	terminal_font_info.descent	=  2;
	terminal_font_info.ascent	= 10;
	terminal_font_info.size		= 10;

	vlr = g_strsplit(*list == '*' ? default_sizes : list,",",-1);

	sz = g_strv_length(vlr);

	font_size = g_malloc0((sz+1) * sizeof(struct font_size));

	for(f=0;f<sz;f++)
		font_size[f].size = g_ascii_strtod(vlr[f],NULL);

	g_strfreev(vlr);
	g_free(list);

	update_font_info(widget);

 }

