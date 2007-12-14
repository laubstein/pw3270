/*
 * Modifications Copyright 1993, 1994, 1995, 1996, 1999,
 *   2000, 2001, 2002, 2004 by Paul Mattes.
 * Copyright 1990 by Jeff Sparkes.
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for any purpose and without fee is hereby granted,
 *  provided that the above copyright notice appear in all copies and that
 *  both that copyright notice and this permission notice appear in
 *  supporting documentation.
 *
 * x3270, c3270, s3270 and tcl3270 are distributed in the hope that they will
 * be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE
 * for more details.
 */

/*
 *	appres.h
 *		Application resource definitions for x3270, c3270, s3270 and
 *		tcl3270.
 */

/* Toggles */

enum toggle_type { TT_INITIAL, TT_INTERACTIVE, TT_ACTION, TT_FINAL };
struct toggle {
	Boolean value;		/* toggle value */
	Boolean changed;	/* has the value changed since init */
	Widget w[2];		/* the menu item widgets */
	const char *label[2];	/* labels */
	void (*upcall)(struct toggle *, enum toggle_type); /* change value */
};

enum TOGGLES_3270
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
	RECONNECT,
	FULLSCREEN,

	N_TOGGLES
};

#define toggled(ix)		(appres.toggle[ix].value)
#define toggle_toggle(t) \
	{ (t)->value = !(t)->value; (t)->changed = True; }

/* Application resources */

typedef struct {
	/* Basic colors */
#if defined(X3270_DISPLAY) /*[*/
	Pixel	foreground;
	Pixel	background;
#endif /*]*/

	/* Options (not toggles) */
	Boolean mono;
	Boolean extended;
	Boolean m3279;
	Boolean modified_sel;
	Boolean	once;
#if defined(X3270_DISPLAY) /*[*/
	Boolean visual_bell;
	Boolean menubar;
	Boolean active_icon;
	Boolean label_icon;
	Boolean invert_kpshift;
	Boolean use_cursor_color;
	Boolean allow_resize;
	Boolean no_other;
	Boolean do_confirms;
	Boolean reconnect;
	Boolean visual_select;
	Boolean suppress_host;
	Boolean suppress_font_menu;
# if defined(X3270_KEYPAD) /*[*/
	Boolean	keypad_on;
# endif /*]*/
#endif /*]*/
#if defined(C3270) /*[*/
	Boolean all_bold_on;
	Boolean	curses_keypad;
	Boolean cbreak_mode;
#endif /*]*/
	Boolean	apl_mode;
	Boolean scripted;
	Boolean numeric_lock;
	Boolean secure;
	Boolean oerr_lock;
	Boolean	typeahead;
	Boolean debug_tracing;
	Boolean disconnect_clear;
	Boolean highlight_bold;
	Boolean color8;
	Boolean bsd_tm;
	Boolean unlock_delay;

	/* Named resources */
#if defined(X3270_KEYPAD) /*[*/
	char	*keypad;
#endif /*]*/
#if defined(X3270_DISPLAY) || defined(C3270) /*[*/
	char	*key_map;
	char	*compose_map;
	char	*printer_lu;
#endif /*]*/
#if defined(X3270_DISPLAY) /*[*/
	char	*efontname;
	char	*fixed_size;
	char	*debug_font;
	char	*icon_font;
	char	*icon_label_font;
	int	save_lines;
	char	*normal_name;
	char	*select_name;
	char	*bold_name;
	char	*colorbg_name;
	char	*keypadbg_name;
	char	*selbg_name;
	char	*cursor_color_name;
	char    *color_scheme;
	int	bell_volume;
	char	*char_class;
	int	modified_sel_color;
	int	visual_select_color;
#if defined(X3270_DBCS) /*[*/
	char	*input_method;
	char	*preedit_type;
#endif /*]*/
#endif /*]*/
#if defined(X3270_DBCS) /*[*/
	char	*local_encoding;
#endif /*]*/
#if defined(C3270) /*[*/
	char	*meta_escape;
	char	*all_bold;
	char	*altscreen;
	char	*defscreen;
#endif /*]*/
	char	*conf_dir;
	char	*model;
	char	*hostsfile;
	char	*port;
	char	*charset;
	char	*termname;
	char	*macros;
	char	*trace_dir;
#if defined(X3270_TRACE) /*[*/
	char	*trace_file;
	char	*screentrace_file;
	char	*trace_file_size;
# if defined(X3270_DISPLAY) /*[*/
	Boolean	trace_monitor;
# endif /*]*/
#endif /*]*/
	char	*oversize;
#if defined(X3270_FT) /*[*/
	char	*ft_command;
	int	dft_buffer_size;
#endif /*]*/
	char	*connectfile_name;
	char	*idle_command;
	Boolean idle_command_enabled;
	char	*idle_timeout;

#if defined(HAVE_LIBSSL) /*[*/
	char	*cert_file;
#endif /*]*/

	/* Toggles */
	struct toggle toggle[N_TOGGLES];

#if defined(X3270_DISPLAY) /*[*/
	/* Simple widget resources */
	Cursor	normal_mcursor;
	Cursor	wait_mcursor;
	Cursor	locked_mcursor;
#endif /*]*/

#if defined(X3270_ANSI) /*[*/
	/* Line-mode TTY parameters */
	Boolean	icrnl;
	Boolean	inlcr;
	Boolean	onlcr;
	char	*erase;
	char	*kill;
	char	*werase;
	char	*rprnt;
	char	*lnext;
	char	*intr;
	char	*quit;
	char	*eof;
#endif /*]*/

#if defined(USE_APP_DEFAULTS) /*[*/
	/* App-defaults version */
	char	*ad_version;
#endif /*]*/

} AppRes, *AppResptr;

extern AppRes appres;
