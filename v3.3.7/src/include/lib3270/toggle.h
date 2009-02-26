/*
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe.
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
 * Este programa está nomeado como @@FILENAME@@ e possui @@LINES@@ linhas de código.
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

	enum _toggle
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

		N_TOGGLES
	};

	extern const char *toggle_names[N_TOGGLES];

	extern void		initialize_toggles(void);
	extern void		shutdown_toggles(void);

	extern int 		register_tchange(int ix, void (*callback)(int value, int reason));
	extern void		do_toggle(int ix);
	extern int			set_toggle(int ix, int value);

	extern const char	*get_toggle_name(int ix);
	extern int			get_toggle_by_name(const char *name);



#endif /* TOGGLE3270_H_INCLUDED */
