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
 * Este programa está nomeado como toggle.h e possui 77 linhas de código.
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

#ifndef TOGGLE3270_H_INCLUDED

	#define TOGGLE3270_H_INCLUDED 1

	#include <lib3270/api.h>

	enum toggle_type { TT_INITIAL, TT_INTERACTIVE, TT_ACTION, TT_FINAL, TT_UPDATE };

	typedef enum _lib3270_toggle_id
	{
		MONOCASE,
		ALT_CURSOR,
		CURSOR_BLINK,
		SHOW_TIMING,
		CURSOR_POS,
		DS_TRACE,
		SCROLL_BAR,
		LINE_WRAP,
		BLANK_FILL,
		SCREEN_TRACE,
		EVENT_TRACE,
		MARGINED_PASTE,
		RECTANGLE_SELECT,
		CROSSHAIR,
		VISIBLE_CONTROL,
		AID_WAIT,
		FULL_SCREEN,
		RECONNECT,
		INSERT,
		KEYPAD,
		SMART_PASTE,

		N_TOGGLES
	} LIB3270_TOGGLE_ID;

	LIB3270_EXPORT void					  register_3270_toggle_monitor(LIB3270_TOGGLE_ID ix, void (*callback)(int value, enum toggle_type reason));
	LIB3270_EXPORT int					  do_3270_toggle(LIB3270_TOGGLE_ID ix);
	LIB3270_EXPORT int					  set_3270_toggle(LIB3270_TOGGLE_ID ix, int value);

	LIB3270_EXPORT const char			* get_3270_toggle_name(LIB3270_TOGGLE_ID ix);
	LIB3270_EXPORT LIB3270_TOGGLE_ID	  get_3270_toggle_by_name(const char *name);

	LIB3270_EXPORT void					  update_toggle_actions(void);

	// Compatibility macros
	#define register_tchange(ix,callback) register_3270_toggle_monitor(ix,callback)
	#define do_toggle(ix) do_3270_toggle(ix)
	#define get_toggle_name(ix) get_3270_toggle_name(ix)
	#define set_toggle(ix,value) set_3270_toggle(ix,value)
	#define get_toggle_by_name(name) get_3270_toggle_by_name(name)

#endif /* TOGGLE3270_H_INCLUDED */
