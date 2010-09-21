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

 struct _font_list
 {
 	double 		size;
 	gint			ascent;
 	gint			descent;
 	gint			width;
 	gint			height;
	cairo_matrix_t	matrix;
 } *font_list = NULL;

 double *font_size = NULL;

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

 	for(f=0;font_list[f].size;f++)
 	{
		if( ((font_list[f].height*(terminal_rows+1))+2) < height && (font_list[f].width*terminal_cols) < width )
			size = f;
 	}

 	if(size < 0)
		size = 0;

//	terminal_font_info.size		= font_list[size].size;
	terminal_font_info.matrix	= &font_list[size].matrix;
	terminal_font_info.descent	= font_list[size].descent;
	terminal_font_info.ascent	= font_list[size].ascent;

	if(terminal_font_info.width != font_list[size].width || terminal_font_info.height != font_list[size].height)
	{
		terminal_font_info.width	= font_list[size].width;
		terminal_font_info.height 	= font_list[size].height;

		Trace("Font size changed to %dx%d",terminal_font_info.width,terminal_font_info.height);

		for(f=0;f<OIA_PIXMAP_COUNT;f++)
		{
			if(pixmap_oia[f])
			{
				gdk_pixmap_unref(pixmap_oia[f]);
				pixmap_oia[f] = 0;
			}
		}
	}


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
	cairo_t	*cr			= gdk_cairo_create(widget->window);
 	gchar	*fontname	= GetString("Terminal","Font","Courier");
 	int		pos = 0;
    double width = -1;
    double height = -1;

	if(terminal_font_info.face)
		cairo_font_face_destroy(terminal_font_info.face);

	terminal_font_info.face = cairo_toy_font_face_create(fontname,CAIRO_FONT_SLANT_NORMAL,TOGGLED_BOLD ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);

	/* Load font sizes */
	cairo_set_font_face(cr,terminal_font_info.face);

 	for(f=0;font_size[f];f++)
 	{
		cairo_font_extents_t sz;

		cairo_set_font_size(cr,font_size[f]);
		cairo_font_extents(cr,&sz);

		if(width != sz.max_x_advance || height != sz.height)
		{
			// Font size changed

			cairo_get_font_matrix(cr,&font_list[pos].matrix);

			font_list[pos].size 	= font_size[f];
			font_list[pos].width  	= (int) sz.max_x_advance;
			font_list[pos].height 	= (int) sz.height;
			font_list[pos].ascent 	= (int) sz.ascent;
			font_list[pos].descent	= (int) sz.descent;

			Trace("%s size=%d xx=%d yx=%d xy=%d yy=%d x0=%d y0=%d w=%d h=%d+%d",
										fontname,
										(int) font_list[pos].size ,
										(int) font_list[pos].matrix.xx ,
										(int) font_list[pos].matrix.yx ,
										(int) font_list[pos].matrix.xy ,
										(int) font_list[pos].matrix.yy ,
										(int) font_list[pos].matrix.x0 ,
										(int) font_list[pos].matrix.y0,
										(int) font_list[pos].width,
										(int) font_list[pos].ascent,
										(int) font_list[pos].descent);

			width 	= sz.max_x_advance;
			height	= sz.height;

			pos = pos+1;
		}
 	}

	memset(font_list+pos,0,sizeof(struct _font_list));

	terminal_font_info.width	= font_list->width;
	terminal_font_info.height	= font_list->height;
	terminal_font_info.descent	= font_list->descent;
	terminal_font_info.ascent	= font_list->ascent;
//	terminal_font_info.size		= font_list->size;
	terminal_font_info.matrix	= &font_list->matrix;

 	Trace("Minimum terminal size is %dx%d",terminal_cols*font_list->width, (terminal_rows+1)*font_list->height);

	gtk_widget_set_size_request(widget, terminal_cols*font_list->width, ((terminal_rows+2)*font_list->height));

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
//	terminal_font_info.size		= 10;

	vlr = g_strsplit(*list == '*' ? default_sizes : list,",",-1);

	sz = g_strv_length(vlr);

	font_size = g_malloc0((sz+1) * sizeof(double));
	font_list = g_malloc0((sz+1) * sizeof(struct _font_list));

	for(f=0;f<sz;f++)
		font_size[f] = g_ascii_strtod(vlr[f],NULL);

	g_strfreev(vlr);
	g_free(list);

	update_font_info(widget);

 }

