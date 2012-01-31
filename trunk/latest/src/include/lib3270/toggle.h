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

	#include <lib3270.h>

	enum toggle_type { TT_INITIAL, TT_INTERACTIVE, TT_ACTION, TT_FINAL, TT_UPDATE };

	#define MONOCASE			LIB3270_TOGGLE_MONOCASE
	#define ALT_CURSOR			LIB3270_TOGGLE_ALT_CURSOR
	#define CURSOR_BLINK		LIB3270_TOGGLE_CURSOR_BLINK
	#define SHOW_TIMING			LIB3270_TOGGLE_SHOW_TIMING
	#define CURSOR_POS			LIB3270_TOGGLE_CURSOR_POS
	#define DS_TRACE			LIB3270_TOGGLE_DS_TRACE
	#define SCROLL_BAR			LIB3270_TOGGLE_SCROLL_BAR
	#define LINE_WRAP			LIB3270_TOGGLE_LINE_WRAP
	#define BLANK_FILL			LIB3270_TOGGLE_BLANK_FILL
	#define SCREEN_TRACE		LIB3270_TOGGLE_SCREEN_TRACE
	#define EVENT_TRACE			LIB3270_TOGGLE_EVENT_TRACE
	#define MARGINED_PASTE		LIB3270_TOGGLE_MARGINED_PASTE
	#define RECTANGLE_SELECT	LIB3270_TOGGLE_RECTANGLE_SELECT
	#define CROSSHAIR			LIB3270_TOGGLE_CROSSHAIR
	#define VISIBLE_CONTROL		LIB3270_TOGGLE_VISIBLE_CONTROL
	#define AID_WAIT			LIB3270_TOGGLE_AID_WAIT
	#define FULL_SCREEN			LIB3270_TOGGLE_FULL_SCREEN
	#define RECONNECT			LIB3270_TOGGLE_RECONNECT
	#define INSERT				LIB3270_TOGGLE_INSERT
	#define KEYPAD				LIB3270_TOGGLE_KEYPAD
	#define SMART_PASTE			LIB3270_TOGGLE_SMART_PASTE
	#define N_TOGGLES			LIB3270_TOGGLE_COUNT

	#define LIB3270_TOGGLE_ID LIB3270_TOGGLE


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
