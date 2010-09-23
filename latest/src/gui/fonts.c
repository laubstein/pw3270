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
#include "oia.h"
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
 * @param info	Font description data.
 * @param width	Widget width.
 * @param height	Widget height.
 *
 * @return TRUE if the image needs update
 */
 gboolean update_terminal_font_size(gint width, gint height)
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

		terminal_font_info.height 	=
		terminal_font_info.spacing = font_list[size].height;

		Trace("Font size changed to %dx%d",terminal_font_info.width,terminal_font_info.height);

		if(terminal_font_info.font)
		{
			cairo_scaled_font_destroy(terminal_font_info.font);
			terminal_font_info.font = NULL;
		}

		oia_release_pixmaps();

	}

	// Adjust line spacing

	terminal_font_info.spacing = height / (terminal_rows+2);

//	Trace("Spacing: %d  height: %d",terminal_font_info.spacing, terminal_font_info.height);

	if(terminal_font_info.spacing < terminal_font_info.height)
		terminal_font_info.spacing = terminal_font_info.height;

	// Center image
	left_margin = (width >> 1) - ((terminal_cols * terminal_font_info.width) >> 1);
	if(left_margin < 0)
		left_margin = 0;

	top_margin = (height >> 1) - (((terminal_rows+1) * terminal_font_info.spacing) >> 1);
	if(top_margin < 0)
		top_margin = 0;

	return TRUE;
 }

/**
 * Release font info.
 *
 */
 void release_font_info(PW3270_FONT_INFO *info)
 {
	if(info->face)
	{
		cairo_font_face_destroy(info->face);
		info->face = NULL;
	}

	if(info->font)
	{
		cairo_scaled_font_destroy(info->font);
		info->font = NULL;
	}

 }

/**
 * Load terminal font info
 *
 */
 void update_font_info(cairo_t *cr, const gchar *fontname, PW3270_FONT_INFO *info)
 {
 	int		f;
 	int		pos = 0;
    double width = -1;
    double height = -1;

	release_font_info(info);

	info->face = cairo_toy_font_face_create(fontname,CAIRO_FONT_SLANT_NORMAL,TOGGLED_BOLD ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);

	/* Load font sizes */
	cairo_set_font_face(cr,info->face);

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

	info->width		= font_list->width;

	info->spacing 	=
	info->height	= font_list->height;

	info->descent	= font_list->descent;
	info->ascent	= font_list->ascent;
//	info->size		= font_list->size;
	info->matrix	= &font_list->matrix;

 	Trace("Minimum terminal size is %dx%d",terminal_cols*font_list->width, (terminal_rows+1)*font_list->height);

 }


/**
 * Load font sizes.
 *
 */
 void load_font_sizes(void)
 {
 	static const gchar *default_sizes = "6,7,8,9,10,11,12,13,14,16,18,20,22,24,26,28,32,36,40,48,56,64,72";

 	gchar 				*list = GetString("Terminal","FontSizes",default_sizes);
 	gchar 				**vlr;
 	gint				sz;
 	int					f;

	Trace("%s",__FUNCTION__);

	memset(&terminal_font_info,0,sizeof(terminal_font_info));

	vlr = g_strsplit(*list == '*' ? default_sizes : list,",",-1);

	sz = g_strv_length(vlr);

	font_size = g_malloc0((sz+1) * sizeof(double));
	font_list = g_malloc0((sz+1) * sizeof(struct _font_list));

	for(f=0;f<sz;f++)
		font_size[f] = g_ascii_strtod(vlr[f],NULL);

	g_strfreev(vlr);
	g_free(list);

 }

