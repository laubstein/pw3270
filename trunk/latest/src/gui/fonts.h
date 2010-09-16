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

	// Globals
	LOCAL_EXTERN gint					  fontWidth;	/**< Current font width, in pixels */
	LOCAL_EXTERN gint					  fontHeight;	/**< Current font height, in pixels */
	LOCAL_EXTERN gint					  fontDescent;	/**< Current font descent, in pixels */
	LOCAL_EXTERN gint					  fontAscent;	/**< Current font ascent, in pixels */
	LOCAL_EXTERN cairo_font_face_t 		* fontFace;		/**< Current font face */
	LOCAL_EXTERN double			  	  fontSize;		/**< Current font size */

	// Prototipes
	LOCAL_EXTERN void		init_terminal_font(GtkWidget *widget);
	LOCAL_EXTERN void		update_font_info(GtkWidget *widget);
	LOCAL_EXTERN gboolean	update_screen_size(GtkWidget *widget, gint width, gint height);

#endif // FONTS_H_INCLUDED
