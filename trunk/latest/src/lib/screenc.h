/*
 * Copyright 1999, 2000, 2002 by Paul Mattes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * c3270 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/* c3270 version of screenc.h */

#define blink_start()
#define display_heightMM()	100
#define display_height()	1
#define display_widthMM()	100
#define display_width()		1
#define screen_obscured()	False
#define screen_scroll()

LIB3270_INTERNAL void ring_bell(void);
LIB3270_INTERNAL void screen_132(void);
LIB3270_INTERNAL void screen_80(void);
LIB3270_INTERNAL void screen_erase(void);
LIB3270_INTERNAL int screen_init(void);
LIB3270_INTERNAL void screen_flip(void);
LIB3270_INTERNAL FILE *start_pager(void);
LIB3270_INTERNAL Boolean screen_new_display_charsets(char *cslist, char *csname);
LIB3270_INTERNAL void mcursor_locked();
LIB3270_INTERNAL void mcursor_normal();
LIB3270_INTERNAL void mcursor_waiting();
LIB3270_INTERNAL void notify_toggle_changed(int ix, int value, int reason);
LIB3270_INTERNAL void set_viewsize(H3270 *session, int rows, int cols);

LIB3270_INTERNAL Boolean escaped;

/*
LIB3270_INTERNAL void Escape_action(Widget w, XEvent *event, String *params, Cardinal *num_params) __attribute__ ((deprecated));
LIB3270_INTERNAL void Help_action(Widget w, XEvent *event, String *params, Cardinal *num_params) __attribute__ ((deprecated));
LIB3270_INTERNAL void Redraw_action(Widget w, XEvent *event, String *params, Cardinal *num_params) __attribute__ ((deprecated));
LIB3270_INTERNAL void Trace_action(Widget w, XEvent *event, String *params, Cardinal *num_params) __attribute__ ((deprecated));
LIB3270_INTERNAL void Show_action(Widget w, XEvent *event, String *params, Cardinal *num_params) __attribute__ ((deprecated));
*/

/*
#if defined(WC3270)
LIB3270_INTERNAL void Paste_action(Widget w, XEvent *event, String *params, Cardinal *num_params) __attribute__ ((deprecated));
LIB3270_INTERNAL void Title_action(Widget w, XEvent *event, String *params, Cardinal *num_params) __attribute__ ((deprecated));
LIB3270_INTERNAL int windows_cp;
#endif
*/

LIB3270_INTERNAL void screen_title(char *text);
