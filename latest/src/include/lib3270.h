/*
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
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
 * Este programa está nomeado como lib3270.h e possui 38 linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas de Mendonça)
 * licinio@bb.com.br		(Licínio Luis Branco)
 * kraucer@bb.com.br		(Kraucer Fernandes Mazuco)
 * macmiranda@bb.com.br		(Marco Aurélio Caldas Miranda)
 *
 */

#ifndef LIB3270_H_INCLUDED

	/**
	 * Character attributes
	 */
	typedef enum _lib3270_attr
	{
		LIB3270_ATTR_COLOR_BACKGROUND		= 0x0000,

		LIB3270_ATTR_COLOR_BLUE				= 0x0001,
		LIB3270_ATTR_COLOR_RED				= 0x0002,
		LIB3270_ATTR_COLOR_PINK				= 0x0003,
		LIB3270_ATTR_COLOR_GREEN			= 0x0004,
		LIB3270_ATTR_COLOR_TURQUOISE		= 0x0005,
		LIB3270_ATTR_COLOR_YELLOW			= 0x0006,
		LIB3270_ATTR_COLOR_WHITE			= 0x0007,
		LIB3270_ATTR_COLOR_BLACK			= 0x0008,
		LIB3270_ATTR_COLOR_DARK_BLUE		= 0x0009,
		LIB3270_ATTR_COLOR_ORANGE			= 0x000A,
		LIB3270_ATTR_COLOR_PURPLE			= 0x000B,
		LIB3270_ATTR_COLOR_DARK_GREEN		= 0x000C,
		LIB3270_ATTR_COLOR_DARK_TURQUOISE	= 0x000D,
		LIB3270_ATTR_COLOR_MUSTARD			= 0x000E,
		LIB3270_ATTR_COLOR_GRAY				= 0x000F,

		LIB3270_ATTR_COLOR					= 0x00FF,

		LIB3270_ATTR_FIELD					= 0x0100,
		LIB3270_ATTR_BLINK					= 0x0200,
		LIB3270_ATTR_UNDERLINE				= 0x0400,
		LIB3270_ATTR_INTENSIFY				= 0x0800,

		LIB3270_ATTR_CG						= 0x1000,
		LIB3270_ATTR_MARKER					= 0x2000,
		LIB3270_ATTR_BACKGROUND_INTENSITY	= 0x4000,

	} LIB3270_ATTR;


	#include <lib3270/api.h>

	/**
	 * Get current screen size.
	 *
	 * Get the size of the terminal in rows/cols; this value can differ from
	 * the model if there's an active "altscreen" with diferent size.
	 *
	 * @param h	Handle of the desired session.
	 * @param r Pointer to screen rows.
	 * @param c Pointer to screen columns.
	 *
	 */
	LIB3270_EXPORT void lib3270_get_screen_size(H3270 *h, int *r, int *c);

	/**
	 * Start a new session (INCOMPLETE).
	 *
	 * Initialize session structure, opens a new session.
	 * WARNING: Multi session ins't yet supported in lib3270, because of this
	 * this call always return the handle of the same session.
	 *
	 * @param model	Terminal model.
	 *
	 * @return lib3270 internal session structure.
	 *
	 */
	LIB3270_EXPORT H3270 * lib3270_session_new(const char *model);

	/**
	 * Destroy session, release memory
	 *
	 * @param h		Session handle.
	 *
	 */
	LIB3270_EXPORT void lib3270_session_free(H3270 *h);

	/**
	 * Register a state change callback
	 *
	 * @param h		Session handle.
	 * @param tx	State ID
	 * @param func	Callback
	 * @param data	Data
	 *
	 */
	LIB3270_EXPORT void lib3270_register_schange(H3270 *h,LIB3270_STATE_CHANGE tx, void (*func)(H3270 *, int, void *),void *data);

	/**
	 * Network connect operation, keep main loop running
	 *
	 * Sets 'reconnect_host', 'current_host' and 'full_current_host' as
	 * side-effects.
	 *
	 * @param h		Session handle.
	 * @param n		Host ID
	 * @param wait	Non zero to wait for connection to be ok.
	 *
	 * @return 0 for success, EAGAIN if auto-reconnect is in progress, EBUSY if connected, ENOTCONN if connection has failed, -1 on unexpected failure.
	 *
	 */
	LIB3270_EXPORT int lib3270_connect(H3270 *h,const char *n, int wait);

	/**
	 * Reconnect.
	 *
	 * @param h		Session handle.
	 * @param wait	Non zero to wait for connection to be ok.
	 */
	LIB3270_EXPORT int lib3270_reconnect(H3270 *h,int wait);

	/**
	 * Get connection state.
	 *
	 * @param h		Session handle.
	 *
	 * @return Connection state.
	 *
	 */
	LIB3270_EXPORT enum cstate lib3270_get_connection_state(H3270 *h);


	/**
	 * Set string at current cursor position.
	 *
	 * Returns are ignored; newlines mean "move to beginning of next line";
	 * tabs and formfeeds become spaces.  Backslashes are not special
	 *
	 * @param h		Session handle.
	 * @param s		String to input.
	 *
	 * @return Negative if error or number of processed characters.
	 *
	 */
	LIB3270_EXPORT int lib3270_set_string(H3270 *h, const unsigned char *str);

	/**
	 * Set cursor address.
	 *
	 * @param h		Session handle.
	 * @param baddr	New cursor address.
	 *
	 * @return last cursor address.
	 *
	 */
	LIB3270_EXPORT int lib3270_set_cursor_address(H3270 *h, int baddr);

	/**
	 * get cursor address.
	 *
	 * @param h		Session handle.
	 *
	 * @return Cursor address.
	 *
	 */
	LIB3270_EXPORT int lib3270_get_cursor_address(H3270 *h);

	/**
	 * Get buffer contents.
	 *
	 * @param h		Session handle.
	 * @param first	First element to get.
	 * @param last	Last element to get.
	 * @param chr	Pointer to buffer which will receive the read chars.
	 * @param attr	Pointer to buffer which will receive the chars attributes.
	 *
	 */
	LIB3270_EXPORT int lib3270_get_contents(H3270 *h, int first, int last, unsigned char *chr, unsigned short *attr);

	LIB3270_EXPORT STATUS_CODE	  lib3270_get_oia_status(H3270 *h);
	LIB3270_EXPORT const char	* lib3270_get_luname(H3270 *h);
	LIB3270_EXPORT const char	* lib3270_get_host(H3270 *h);



#endif // LIB3270_H_INCLUDED
