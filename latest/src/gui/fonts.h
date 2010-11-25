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
 * Este programa está nomeado como fonts.h e possui - linhas de código.
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

#ifndef FONTS_H_INCLUDED

	#define FONTS_H_INCLUDED

	typedef struct _pw3270_font_info
	{
		gint				  width;		/**< Font width, in pixels */
		gint				  height;		/**< Font height, in pixels */
		gint				  descent;		/**< Font descent, in pixels */
		gint				  ascent;		/**< Font ascent, in pixels */
		gint				  spacing;		/**< Line height, in pixels */
		cairo_font_face_t	* face;			/**< Font face */
		cairo_scaled_font_t * font;			/**< Scaled font - http://library.gnome.org/devel/cairo/stable/cairo-scaled-font.html */
		cairo_matrix_t		* matrix;		/**< Font matrix */
	} PW3270_FONT_INFO;

	// Globals
	LOCAL_EXTERN PW3270_FONT_INFO	terminal_font_info;

	// Prototipes
	LOCAL_EXTERN void		load_font_sizes(void);
	LOCAL_EXTERN void		load_font_menu(GtkWidget *widget, GtkWidget *topmenu, const gchar *selected);
	LOCAL_EXTERN void		update_font_info(cairo_t *cr, const gchar *fontname, PW3270_FONT_INFO *info);
	LOCAL_EXTERN void 		release_font_info(PW3270_FONT_INFO *info);

	LOCAL_EXTERN gboolean	update_terminal_font_size(gint width, gint height);

	LOCAL_EXTERN void		draw_element(cairo_t *cr, PW3270_FONT_INFO *font, int x, int y, int baseline, int addr, GdkColor *clr);


	// Terminal font macros
	#define terminal_font_width() terminal_font_info.width
	#define terminal_font_height() terminal_font_info.height
	#define terminal_font_descent() terminal_font_info.descent
	#define terminal_font_face() terminal_font_info.face
	#define terminal_font_size() terminal_font_info.size

	// Compatibility macros
	#define fontAscent	terminal_font_info.ascent
	#define fontWidth	terminal_font_info.width
	#define fontHeight	terminal_font_info.height
	#define fontDescent	terminal_font_info.descent

#endif // FONTS_H_INCLUDED
