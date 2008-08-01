/*
 * Modifications and original code Copyright 1993, 1994, 1995, 1996,
 *    2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Paul Mattes.
 * Original X11 Port Copyright 1990 by Jeff Sparkes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * Copyright 1989 by Georgia Tech Research Corporation, Atlanta, GA 30332.
 *   All Rights Reserved.  GTRC hereby grants public use of this software.
 *   Derivative works based on this software must incorporate this copyright
 *   notice.
 *
 * c3270 and wc3270 are distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/*
 *	toggle.h
 *		lib3270 toggle calls
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

		N_TOGGLES
	};

	extern const char *toggle_names[N_TOGGLES];

	int 		register_tchange(int ix, void (*callback)(int value, int reason));
	void		do_toggle(int ix);
	int			set_toggle(int ix, int value);

	const char	*get_toggle_name(int ix);
	int			get_toggle_by_name(const char *name);



#endif /* TOGGLE3270_H_INCLUDED */
