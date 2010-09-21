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
		gint				  width;	/**< Font width, in pixels */
		gint				  height;	/**< Font height, in pixels */
		gint				  descent;	/**< Font descent, in pixels */
		gint				  ascent;	/**< Font ascent, in pixels */
		cairo_font_face_t	* face;		/**< Font face */
		cairo_matrix_t		* matrix;	/**< Font matrix */
//		double			  	  size;		/**< Font size */
	} PW3270_FONT_INFO;

	// Globals
	LOCAL_EXTERN PW3270_FONT_INFO	terminal_font_info;

	// Prototipes
	LOCAL_EXTERN void		init_terminal_font(GtkWidget *widget);
	LOCAL_EXTERN void		update_font_info(GtkWidget *widget);
	LOCAL_EXTERN gboolean	update_screen_size(GtkWidget *widget, gint width, gint height);

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
