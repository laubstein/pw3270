/*
 * Copyright 2000, 2001, 2002, 2004, 2005, 2006, 2007 by Paul Mattes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * c3270 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/*
 *	screen.c
 *		A Windows console-based 3270 Terminal Emulator
 *		Screen drawing
 */

#include "globals.h"
#include <signal.h>
#include "appres.h"
#include "3270ds.h"
#include "resources.h"
#include "ctlr.h"

#include "actionsc.h"
#include "ctlrc.h"
#include "hostc.h"
#include "keymapc.h"
#include "kybdc.h"
#include "macrosc.h"
#include "screenc.h"
#include "tablesc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "w3miscc.h"
#include "widec.h"
#include "xioc.h"
#include "screen.h"
#include "errno.h"
#include <lib3270/api.h>

#include <windows.h>
#include <wincon.h>
#include "winversc.h"

extern char *profile_name;

static const struct lib3270_screen_callbacks *callbacks = NULL;

#define MAX_COLORS	16
static int cmap_fg[MAX_COLORS] = {
	0,						/* neutral black */
	FOREGROUND_INTENSITY | FOREGROUND_BLUE,		/* blue */
	FOREGROUND_INTENSITY | FOREGROUND_RED,		/* red */
	FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
							/* pink */
	FOREGROUND_INTENSITY | FOREGROUND_GREEN,	/* green */
	FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
							/* turquoise */
	FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED,
							/* yellow */
	FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
	0,						/* black */
	FOREGROUND_BLUE,				/* deep blue */
	FOREGROUND_INTENSITY | FOREGROUND_RED,		/* orange */
	FOREGROUND_RED | FOREGROUND_BLUE,		/* purple */
	FOREGROUND_GREEN,				/* pale green */
	FOREGROUND_GREEN | FOREGROUND_BLUE,		/* pale turquoise */
	FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, /* gray */
	FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,							/* white */

							/* neutral white */
};
static int cmap_bg[MAX_COLORS] = {
	0,						/* neutral black */
	BACKGROUND_INTENSITY | BACKGROUND_BLUE,		/* blue */
	BACKGROUND_INTENSITY | BACKGROUND_RED,		/* red */
	BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE,
							/* pink */
	BACKGROUND_INTENSITY | BACKGROUND_GREEN,	/* green */
	BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE,
							/* turquoise */
	BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_RED,
							/* yellow */
	BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_RED | BACKGROUND_BLUE,
							/* neutral white */
	0,						/* black */
	BACKGROUND_BLUE,				/* deep blue */
	BACKGROUND_INTENSITY | BACKGROUND_RED,		/* orange */
	BACKGROUND_RED | BACKGROUND_BLUE,		/* purple */
	BACKGROUND_GREEN,				/* pale green */
	BACKGROUND_GREEN | BACKGROUND_BLUE,		/* pale turquoise */
	BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE, /* gray */
	BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,							/* white */
};
static int field_colors[4] = {
	COLOR_GREEN,		/* default */
	COLOR_RED,		/* intensified */
	COLOR_BLUE,		/* protected */
	COLOR_NEUTRAL_WHITE	/* protected, intensified */
};
static struct {
	char *name;
	int index;
} host_color[] = {
	{ "NeutralBlack",	COLOR_NEUTRAL_BLACK },
	{ "Blue",		COLOR_BLUE },
	{ "Red",		COLOR_RED },
	{ "Pink",		COLOR_PINK },
	{ "Green",		COLOR_GREEN },
	{ "Turquoise",		COLOR_TURQUOISE },
	{ "Yellow",		COLOR_YELLOW },
	{ "NeutralWhite",	COLOR_NEUTRAL_WHITE },
	{ "Black",		COLOR_BLACK },
	{ "DeepBlue",		COLOR_DEEP_BLUE },
	{ "Orange",		COLOR_ORANGE },
	{ "Purple",		COLOR_PURPLE },
	{ "PaleGreen",		COLOR_PALE_GREEN },
	{ "PaleTurquoise",	COLOR_PALE_TURQUOISE },
	{ "Grey",		COLOR_GREY },
	{ "Gray",		COLOR_GREY }, /* alias */
	{ "White",		COLOR_WHITE },
	{ CN,			0 }
};

static int defattr = 0;
// static unsigned long input_id;

Boolean escaped = True;
Boolean screen_has_changes = FALSE;

enum ts { TS_AUTO, TS_ON, TS_OFF };
enum ts ab_mode = TS_AUTO;

int windows_cp = 0;

#if defined(MAYBE_SOMETIME) /*[*/
/*
 * A bit of a cheat.  We know that Windows console attributes are really just
 * colors, with bits 0-3 for foreground and bits 4-7 for background.  That
 * leaves 8 bits we can play with for our own devious purposes, as long as we
 * don't accidentally pass one of those bits to Windows.
 *
 * The attributes we define are:
 *  WCATTR_UNDERLINE: The character is underlined.  Windows does not support
 *    underlining, but we do, by displaying underlined spaces as underscores.
 *    Some people may find this absolutely maddening.
 */
#endif /*]*/

static CHAR_INFO *onscreen;	/* what's on the screen now */
static CHAR_INFO *toscreen;	/* what's supposed to be on the screen */
static int onscreen_valid = FALSE; /* is onscreen valid? */

// static int status_row = 0;	/* Row to display the status line on */
// static int status_skip = 0;	/* Row to blank above the status line */

static void status_connect(Boolean ignored);
static void status_3270_mode(Boolean ignored);
static void status_printer(Boolean on);
static int get_color_pair(int fg, int bg);
static int color_from_fa(unsigned char fa);
static Boolean ts_value(const char *s, enum ts *tsp);
static int linedraw_to_acs(unsigned char c);
static int apl_to_acs(unsigned char c);
static void relabel(Boolean ignored);
static void check_aplmap(int codepage);
static void init_user_colors(void);
static void init_user_attribute_colors(void);

// static HANDLE chandle;	/* console input handle */
static HANDLE cohandle;	/* console screen buffer handle */

static HANDLE *sbuf;	/* dynamically-allocated screen buffer */

static int console_rows;
static int console_cols;

// static int screen_swapped = FALSE;

/*
 * Console event handler.
 */
static BOOL
cc_handler(DWORD type)
{
	if (type == CTRL_C_EVENT) {
		char *action;

		/* Process it as a Ctrl-C. */
		trace_event("Control-C received via Console Event Handler\n");
		action = lookup_key(0x03, LEFT_CTRL_PRESSED);
		if (action != CN) {
			if (strcmp(action, "[ignore]"))
				push_keymap_action(action);
		} else {
			String params[2];
			Cardinal one;

			params[0] = "0x03";
			params[1] = CN;
			one = 1;
			Key_action(NULL, NULL, params, &one);
		}

		return TRUE;
	} else {
		/* Let Windows have its way with it. */
		return FALSE;
	}
}

/*
 * Get a handle for the console.
 */
static int initscr(void)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	size_t buffer_size;
	CONSOLE_CURSOR_INFO cursor_info;

	cohandle = CreateFile("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, 0, NULL);
	if (cohandle == NULL) {
		fprintf(stderr, "CreateFile(CONOUT$) failed: %s\n",
			win32_strerror(GetLastError()));
		return 0;
	}

	/* Get its dimensions. */
	if (GetConsoleScreenBufferInfo(cohandle, &info) == 0) {
		fprintf(stderr, "GetConsoleScreenBufferInfo failed: %s\n",
			win32_strerror(GetLastError()));
		return 0;
	}
	console_rows = info.srWindow.Bottom - info.srWindow.Top + 1;
	console_cols = info.srWindow.Right - info.srWindow.Left + 1;

	/* Get its cursor configuration. */
	if (GetConsoleCursorInfo(cohandle, &cursor_info) == 0) {
		fprintf(stderr, "GetConsoleCursorInfo failed: %s\n",
			win32_strerror(GetLastError()));
		return 0;
	}

	/* Create the screen buffer. */
	sbuf = CreateConsoleScreenBuffer(
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE,
		NULL,
		CONSOLE_TEXTMODE_BUFFER,
		NULL);
	if (sbuf == NULL) {
		fprintf(stderr,
			"CreateConsoleScreenBuffer failed: %s\n",
			win32_strerror(GetLastError()));
		return 0;
	}

	/* Set its cursor state. */
	if (SetConsoleCursorInfo(sbuf, &cursor_info) == 0) {
		fprintf(stderr, "SetConsoleScreenBufferInfo failed: %s\n",
			win32_strerror(GetLastError()));
		return 0;
	}

	/* Define a console handler. */
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)cc_handler, TRUE)) {
		fprintf(stderr, "SetConsoleCtrlHandler failed: %s\n",
				win32_strerror(GetLastError()));
		return 0;
	}

	/* Allocate and initialize the onscreen and toscreen buffers. */
	buffer_size = sizeof(CHAR_INFO) * console_rows * console_cols;
	onscreen = (CHAR_INFO *)Malloc(buffer_size);
	(void) memset(onscreen, '\0', buffer_size);
	onscreen_valid = FALSE;
	toscreen = (CHAR_INFO *)Malloc(buffer_size);
	(void) memset(toscreen, '\0', buffer_size);

	/* More will no doubt follow. */
	return 1;
}

/* Try to set the console output character set. */
void
set_display_charset(char *dcs)
{
	char *copy;
	char *s;
	char *cs;
	int want_cp = 0;


	if(callbacks && callbacks->charset)
		callbacks->charset(dcs);

	windows_cp = GetConsoleCP();

	copy = strdup(dcs);
	s = copy;
	while ((cs = strtok(s, ",")) != NULL) {
		s = NULL;

		if (!strncmp(cs, "windows-", 8)) {
			want_cp = atoi(cs + 8);
			break;
		} else if (!strncmp(cs, "iso8859-", 8)) {
			want_cp = 28590 + atoi(cs + 8);
			break;
		} else if (!strcmp(cs, "koi8-r")) {
			want_cp = 20866;
			break;
		}
	}
	free(copy);

	if (want_cp != 0 && windows_cp != want_cp) {
	    	if (SetConsoleOutputCP(want_cp)) {
			(void)SetConsoleCP(want_cp);
		    	windows_cp = want_cp;
		} else {
			fprintf(stderr,
				"Unable to set output character set to '%s' "
				"(Windows code page %d).\n",
				dcs, want_cp);
		}
	}

	check_aplmap(windows_cp);
}

/*
 * Vitrual curses functions.
 */
static int cur_row = 0;
static int cur_col = 0;
static int cur_attr = 0;

static void
move(int row, int col)
{
	cur_row = row;
	cur_col = col;
}

static void
attrset(int a)
{
	cur_attr = a;
}

static void
addch(int c)
{
	CHAR_INFO *ch = &toscreen[(cur_row * console_cols) + cur_col];

	if(callbacks && callbacks->addch)
		callbacks->addch(cur_row, cur_col, c, cur_attr);

	/* Save the desired character. */
	if (ch->Char.UnicodeChar != c || ch->Attributes != cur_attr) {
	    	ch->Char.UnicodeChar = c;
		ch->Attributes = cur_attr;
	}

	/* Increment and wrap. */
	if (++cur_col >= console_cols) {
		cur_col = 0;
		if (++cur_row >= console_rows)
			cur_row = 0;
	}
}

/*
static int
ix(int row, int col)
{
	return (row * console_cols) + col;
}
*/

// static char *done_array = NULL;

/*
static void
none_done(void)
{
    	if (done_array == NULL) {
	    	done_array = Malloc(console_rows * console_cols);
	}
	memset(done_array, '\0', console_rows * console_cols);
}
*/

/*
static int
is_done(int row, int col)
{
    	return done_array[ix(row, col)];
}
*/

/*
static void
mark_done(int start_row, int end_row, int start_col, int end_col)
{
    	int row;

	for (row = start_row; row <= end_row; row++) {
	    	memset(&done_array[ix(row, start_col)],
			1, end_col - start_col + 1);
	}
}
*/

/*
static int
tos_a(int row, int col)
{
    	// return toscreen[ix(row, col)].Attributes;
	if (toscreen[ix(row, col)].Char.UnicodeChar & ~0xff)
		return toscreen[ix(row, col)].Attributes | 0x80000000;
	else
	    	return toscreen[ix(row, col)].Attributes;
}
*/

#if defined(DEBUG_SCREEN_DRAW) /*[*/
static int
changed(int row, int col)
{
	return !onscreen_valid ||
		memcmp(&onscreen[ix(row, col)], &toscreen[ix(row, col)],
		       sizeof(CHAR_INFO));
}
#endif /*]*/

/*
 * Draw a rectangle of homogeneous text.
 */
/*
static void
hdraw(int row, int lrow, int col, int lcol)
{
	COORD bufferSize;
	COORD bufferCoord;
	SMALL_RECT writeRegion;
	int xrow;
	int rc;

	// Write it.
	bufferSize.X = console_cols;
	bufferSize.Y = console_rows;
	bufferCoord.X = col;
	bufferCoord.Y = row;
	writeRegion.Left = col;
	writeRegion.Top = row;
	writeRegion.Right = lcol;
	writeRegion.Bottom = lrow;
	if (toscreen[ix(row, col)].Char.UnicodeChar & ~0xff)
	    	rc = WriteConsoleOutputW(sbuf, toscreen, bufferSize,
			bufferCoord, &writeRegion);
	else
	    	rc = WriteConsoleOutputA(sbuf, toscreen, bufferSize,
			bufferCoord, &writeRegion);
	if (rc == 0) {

		fprintf(stderr, "WriteConsoleOutput failed: %s\n",
			win32_strerror(GetLastError()));
		x3270_exit(1);
	}

	// Sync 'onscreen'.
	for (xrow = row; xrow <= lrow; xrow++) {
	    	memcpy(&onscreen[ix(xrow, col)],
		       &toscreen[ix(xrow, col)],
		       sizeof(CHAR_INFO) * (lcol - col + 1));
	}

	// Mark the region as done.
	mark_done(row, lrow, col, lcol);
}
*/

/*
 * Draw a rectanglar region from 'toscreen' onto the screen, without regard to
 * what is already there.
 * If the attributes for the entire region are the same, we can draw it in
 * one go; otherwise we will need to break it into little pieces (fairly
 * stupidly) with common attributes.
 * When done, copy the region from 'toscreen' to 'onscreen'.
 */ /*
static void
draw_rect(int pc_start, int pc_end, int pr_start, int pr_end)
{
    	int a;
	int ul_row, ul_col, xrow, xcol, lr_row, lr_col;


	for (ul_row = pr_start; ul_row <= pr_end; ul_row++) {
	    	for (ul_col = pc_start; ul_col <= pc_end; ul_col++) {
		    	int col_found = 0;

		    	if (is_done(ul_row, ul_col))
			    	continue;

			//
			// [ul_row,ul_col] is the upper left-hand corner of an
			// undrawn region.
			//
			// Find the the lower right-hand corner of the
			// rectangle with common attributes.
			//
			a = tos_a(ul_row, ul_col);
			lr_col = pc_end;
			lr_row = pr_end;
			for (xrow = ul_row;
				!col_found && xrow <= pr_end;
				xrow++) {

				if (is_done(xrow, ul_col) ||
				    tos_a(xrow, ul_col) != a) {
					lr_row = xrow - 1;
					break;
				}
				for (xcol = ul_col; xcol <= lr_col; xcol++) {
				    	if (is_done(xrow, xcol) ||
					    tos_a(xrow, xcol) != a) {
						lr_col = xcol - 1;
						lr_row = xrow;
						col_found = 1;
						break;
					}
				}
			}
			hdraw(ul_row, lr_row, ul_col, lr_col);
		}
	}
}
*/

/*
 * Compare 'onscreen' (what's on the screen right now) with 'toscreen' (what
 * we want on the screen) and draw what's changed.  Hopefully it will be in
 * a reasonably optimized fashion.
 *
 * Windows lets us draw a rectangular areas with one call, provided that the
 * whole area has the same attributes.  We will take advantage of this where
 * it is relatively easy to figure out, by walking row by row, holding on to
 * and widening a vertical band of modified columns and drawing only when we
 * hit a row that needs no modifications.  This will cause us to miss some
 * easy-seeming cases that require recognizing multiple bands per row.
 */
 /*
static void
sync_onscreen(void)
{
    	int row;
	int col;
	int pending = FALSE;	// is there a draw pending?
	int pc_start, pc_end;	// first and last columns in pending band
	int pr_start;		// first row in pending band

	// Clear out the 'what we've seen' array.
    	none_done();


	// Sometimes you have to draw everything.
	if (!onscreen_valid) {
	    	draw_rect(0, console_cols - 1, 0, console_rows - 1);
		onscreen_valid = TRUE;
		return;
	}

	for (row = 0; row < console_rows; row++) {

	    	// Check the whole row for a match first.
	    	if (!memcmp(&onscreen[ix(row, 0)],
			    &toscreen[ix(row, 0)],
			    sizeof(CHAR_INFO) * console_cols)) {
		    if (pending) {
			    draw_rect(pc_start, pc_end, pr_start, row - 1);
			    pending = FALSE;
		    }
		    continue;
		}

		for (col = 0; col < console_cols; col++) {
		    	if (memcmp(&onscreen[ix(row, col)],
				   &toscreen[ix(row, col)],
				   sizeof(CHAR_INFO))) {
				// This column differs.
				// Start or expand the band, and start pending.
				//
			    	if (!pending || col < pc_start)
				    	pc_start = col;
				if (!pending || col > pc_end)
				    	pc_end = col;
				if (!pending) {
				    	pr_start = row;
					pending = TRUE;
				}
			}
		}
	}

	if (pending)
	    	draw_rect(pc_start, pc_end, pr_start, console_rows - 1);

}
	*/

/* Repaint the screen. */
static void
refresh(void)
{
	/*
	COORD coord;

	// Draw the differences between 'onscreen' and 'toscreen' into
	sync_onscreen();

	// Move the cursor.
	coord.X = cur_col;
	coord.Y = cur_row;
	if (SetConsoleCursorPosition(sbuf, coord) == 0) {
		fprintf(stderr,
			"\nrefresh: SetConsoleCursorPosition(x=%d,y=%d) "
			"failed: %s\n",
			coord.X, coord.Y, win32_strerror(GetLastError()));
		x3270_exit(1);
	}

	// Swap in this buffer.
	if (screen_swapped == FALSE) {
		if (SetConsoleActiveScreenBuffer(sbuf) == 0) {
			fprintf(stderr,
				"\nSetConsoleActiveScreenBuffer failed: %s\n",
				win32_strerror(GetLastError()));
			x3270_exit(1);
		}
		screen_swapped = TRUE;
	}
	*/
}

/* Initialize the screen. */
void
screen_init(void)
{
	int want_ov_rows = ov_rows;
	int want_ov_cols = ov_cols;
	Boolean oversize = False;

	if(callbacks && callbacks->init)
		callbacks->init();

	/* Disallow altscreen/defscreen. */
	if ((appres.altscreen != CN) || (appres.defscreen != CN)) {
		(void) fprintf(stderr, "altscreen/defscreen not supported\n");
		x3270_exit(1);
	}
	/* Initialize the console. */
	if (!initscr()) {
		(void) fprintf(stderr, "Can't initialize terminal.\n");
		x3270_exit(1);
	}

	/*
	 * Respect the console size we are given.
	 */
	while (console_rows < maxROWS || console_cols < maxCOLS) {
		char buf[2];

		/*
		 * First, cancel any oversize.  This will get us to the correct
		 * model number, if there is any.
		 */
		if ((ov_cols && ov_cols > console_cols) ||
		    (ov_rows && ov_rows > console_rows)) {
			ov_cols = 0;
			ov_rows = 0;
			oversize = True;
			continue;
		}

		/* If we're at the smallest screen now, give up. */
		if (model_num == 2) {
			(void) fprintf(stderr, "Emulator won't fit on a %dx%d "
			    "display.\n", console_rows, console_cols);
			x3270_exit(1);
		}

		/* Try a smaller model. */
		(void) sprintf(buf, "%d", model_num - 1);
		appres.model = NewString(buf);
		set_rows_cols(model_num - 1, 0, 0);
	}

	/*
	 * Now, if they wanted an oversize, but didn't get it, try applying it
	 * again.
	 */
	if (oversize) {
		if (want_ov_rows > console_rows - 2)
			want_ov_rows = console_rows - 2;
		if (want_ov_rows < maxROWS)
			want_ov_rows = maxROWS;
		if (want_ov_cols > console_cols)
			want_ov_cols = console_cols;
		set_rows_cols(model_num, want_ov_cols, want_ov_rows);
	}

	if(callbacks && callbacks->setsize)
		callbacks->setsize(maxROWS,maxCOLS);

	/* Set up callbacks for state changes. */
	register_schange(ST_CONNECT, status_connect);
	register_schange(ST_3270_MODE, status_3270_mode);
	register_schange(ST_PRINTER, status_printer);

	register_schange(ST_HALF_CONNECT, relabel);
	register_schange(ST_CONNECT, relabel);
	register_schange(ST_3270_MODE, relabel);

	/* See about all-bold behavior. */
	if (appres.all_bold_on)
		ab_mode = TS_ON;
	else if (!ts_value(appres.all_bold, &ab_mode))
		(void) fprintf(stderr, "invalid %s value: '%s', "
		    "assuming 'auto'\n", ResAllBold, appres.all_bold);
	if (ab_mode == TS_AUTO)
		ab_mode = appres.m3279? TS_ON: TS_OFF;

	/* If the want monochrome, assume they want green. */
	if (!appres.m3279) {
	    	defattr |= FOREGROUND_GREEN;
		if (ab_mode == TS_ON)
			defattr |= FOREGROUND_INTENSITY;
	}

	/* Pull in the user's color mappings. */
	init_user_colors();
	init_user_attribute_colors();

	/* Set up the controller. */
	ctlr_init(-1);
	ctlr_reinit(-1);

	/* Set the window label. */
	if (appres.title != CN)
		screen_title(appres.title);
	else if (profile_name != CN)
		screen_title(profile_name);
	else
		screen_title(NULL);

	/* Finish screen initialization. */
	screen_suspend();
}

/*
 * Parse a tri-state resource value.
 * Returns True for success, False for failure.
 */
static Boolean
ts_value(const char *s, enum ts *tsp)
{
	*tsp = TS_AUTO;

	if (s != CN && s[0]) {
		int sl = strlen(s);

		if (!strncasecmp(s, "true", sl))
			*tsp = TS_ON;
		else if (!strncasecmp(s, "false", sl))
			*tsp = TS_OFF;
		else if (strncasecmp(s, "auto", sl))
			return False;
	}
	return True;
}

/* Allocate a color pair. */
static int
get_color_pair(int fg, int bg)
{
    	int mfg = fg & 0xf;
    	int mbg = bg & 0xf;

	if (mfg >= MAX_COLORS)
	    	mfg = 0;
	if (mbg >= MAX_COLORS)
	    	mbg = 0;

	return cmap_fg[mfg] | cmap_bg[mbg];
}

/*
 * Initialize the user-specified attribute color mappings.
 */
static void
init_user_attribute_color(int *a, const char *resname)
{
	char *r;
	unsigned long l;
	char *ptr;
	int i;

	if ((r = get_resource(resname)) == CN)
		return;
	for (i = 0; host_color[i].name != CN; i++) {
	    	if (!strcasecmp(r, host_color[i].name)) {
		    	*a = host_color[i].index;
			return;
		}
	}
	l = strtoul(r, &ptr, 0);
	if (ptr == r || *ptr != '\0' || l >= MAX_COLORS) {
		xs_warning("Invalid %s value: %s", resname, r);
		return;
	}
	*a = (int)l;
}

static void
init_user_attribute_colors(void)
{
	init_user_attribute_color(&field_colors[0],
		ResHostColorForDefault);
	init_user_attribute_color(&field_colors[1],
		ResHostColorForIntensified);
	init_user_attribute_color(&field_colors[2],
		ResHostColorForProtected);
	init_user_attribute_color(&field_colors[3],
		ResHostColorForProtectedIntensified);
}

/*
 * Map a field attribute to a 3270 color index.
 * Applies only to m3270 mode -- does not work for mono.
 */
static int
color3270_from_fa(unsigned char fa)
{
#	define DEFCOLOR_MAP(f) \
		((((f) & FA_PROTECT) >> 4) | (((f) & FA_INT_HIGH_SEL) >> 3))

	return field_colors[DEFCOLOR_MAP(fa)];
}

/* Map a field attribute to its default colors. */
static int
color_from_fa(unsigned char fa)
{
	if (appres.m3279) {
		int fg;

		fg = color3270_from_fa(fa);
		return get_color_pair(fg, COLOR_NEUTRAL_BLACK);
	} else
		return FOREGROUND_GREEN |
		    (((ab_mode == TS_ON) || FA_IS_HIGH(fa))?
		     FOREGROUND_INTENSITY: 0);
}

static int
reverse_colors(int a)
{
    	int rv = 0;

	/* Move foreground colors to background colors. */
	if (a & FOREGROUND_RED)
	    	rv |= BACKGROUND_RED;
	if (a & FOREGROUND_BLUE)
	    	rv |= BACKGROUND_BLUE;
	if (a & FOREGROUND_GREEN)
	    	rv |= BACKGROUND_GREEN;
	if (a & FOREGROUND_INTENSITY)
	    	rv |= BACKGROUND_INTENSITY;

	/* And vice versa. */
	if (a & BACKGROUND_RED)
	    	rv |= FOREGROUND_RED;
	if (a & BACKGROUND_BLUE)
	    	rv |= FOREGROUND_BLUE;
	if (a & BACKGROUND_GREEN)
	    	rv |= FOREGROUND_GREEN;
	if (a & BACKGROUND_INTENSITY)
	    	rv |= FOREGROUND_INTENSITY;

	return rv;
}

/*
 * Set up the user-specified color mappings.
 */
static void
init_user_color(const char *name, int ix)
{
    	char *r;
	unsigned long l;
	char *ptr;

	r = get_fresource("%s%s", ResConsoleColorForHostColor, name);
	if (r == CN)
		r = get_fresource("%s%d", ResConsoleColorForHostColor, ix);
	if (r == CN)
	    	return;

	l = strtoul(r, &ptr, 0);
	if (ptr != r && *ptr == '\0' && l <= 15) {
	    	cmap_fg[ix] = (int)l;
	    	cmap_bg[ix] = (int)l + 16;
		return;
	}

	xs_warning("Invalid %s value '%s'", ResConsoleColorForHostColor, r);
}

static void
init_user_colors(void)
{
	int i;

	for (i = 0; host_color[i].name != CN; i++) {
	    	init_user_color(host_color[i].name, host_color[i].index);
	}
}

/*
 * Find the display attributes for a baddr, fa_addr and fa.
 */
static int
calc_attrs(int baddr, int fa_addr, int fa)
{
    	int fg, bg, gr, a;

	/* Compute the color. */

	/* Monochrome is easy, and so is color if nothing is specified. */
	if (!appres.m3279 ||
		(!ea_buf[baddr].fg &&
		 !ea_buf[fa_addr].fg &&
		 !ea_buf[baddr].bg &&
		 !ea_buf[fa_addr].bg)) {

	    	a = color_from_fa(fa);

	} else {

		/* The current location or the fa specifies the fg or bg. */
		if (ea_buf[baddr].fg)
			fg = ea_buf[baddr].fg & 0x0f;
		else if (ea_buf[fa_addr].fg)
			fg = ea_buf[fa_addr].fg & 0x0f;
		else
			fg = color3270_from_fa(fa);

		if (ea_buf[baddr].bg)
			bg = ea_buf[baddr].bg & 0x0f;
		else if (ea_buf[fa_addr].bg)
			bg = ea_buf[fa_addr].bg & 0x0f;
		else
			bg = COLOR_NEUTRAL_BLACK;

		a = get_color_pair(fg, bg);
	}

	/* Compute the display attributes. */

	if (ea_buf[baddr].gr)
		gr = ea_buf[baddr].gr;
	else if (ea_buf[fa_addr].gr)
		gr = ea_buf[fa_addr].gr;
	else
		gr = 0;

	if (appres.highlight_underline &&
		appres.m3279 &&
		(gr & (GR_BLINK | GR_UNDERLINE)) &&
		!(gr & GR_REVERSE) &&
		!bg) {

	    	a |= BACKGROUND_INTENSITY;
	}

	if (!appres.m3279 &&
		((gr & GR_INTENSIFY) || (ab_mode == TS_ON) || FA_IS_HIGH(fa))) {

		a |= FOREGROUND_INTENSITY;
	}

	if (gr & GR_REVERSE)
		a = reverse_colors(a);

	return a;
}

/* Erase screen */
void screen_erase(void)
{
	/* If the application supplies a callback use it! */
	if(callbacks && callbacks->erase)
	{
		callbacks->erase();
		screen_has_changes = FALSE;
		return;
	}

	/* No callback, just redraw */
	screen_disp();
}

/* Display what's in the buffer. */
void screen_disp(void)
{
	int row, col;
	int a;
	int c;
	unsigned char fa;
#if defined(X3270_DBCS) /*[*/
	enum dbcs_state d;
#endif /*]*/
	int fa_addr;

	if (!screen_has_changes)
	{
		/* Move the cursor. */
		if (flipped)
			move(cursor_addr / cCOLS,cCOLS-1 - (cursor_addr % cCOLS));
		else
			move(cursor_addr / cCOLS, cursor_addr % cCOLS);
		return;
	}

	fa = get_field_attribute(0);
	a = color_from_fa(fa);
	fa_addr = find_field_attribute(0); /* may be -1, that's okay */
	for (row = 0; row < ROWS; row++) {
		int baddr;

		if (!flipped)
			move(row, 0);
		for (col = 0; col < cCOLS; col++) {
			if (flipped)
				move(row, cCOLS-1 - col);
			baddr = row*cCOLS+col;
			if (ea_buf[baddr].fa) {
			    	/* Field attribute. */
			    	fa_addr = baddr;
				fa = ea_buf[baddr].fa;
				a = calc_attrs(baddr, baddr, fa);
				attrset(defattr);
				addch(' ');
			} else if (FA_IS_ZERO(fa)) {
			    	/* Blank. */
				attrset(a);
				addch(' ');
			} else {
			    	/* Normal text. */
				if (!(ea_buf[baddr].gr ||
				      ea_buf[baddr].fg ||
				      ea_buf[baddr].bg)) {
					attrset(a);
				} else {
					int b;

					/*
					 * Override some of the field
					 * attributes.
					 */
					b = calc_attrs(baddr, fa_addr, fa);
					attrset(b);
				}
#if defined(X3270_DBCS) /*[*/
				d = ctlr_dbcs_state(baddr);
				if (IS_LEFT(d)) {
					int xaddr = baddr;
					char mb[16];
					int len;
					int i;

					INC_BA(xaddr);
					len = dbcs_to_mb(ea_buf[baddr].cc,
					    ea_buf[xaddr].cc,
					    mb);
					for (i = 0; i < len; i++) {
						addch(mb[i] & 0xff);
					}
				} else if (!IS_RIGHT(d)) {
#endif /*]*/
					if (ea_buf[baddr].cs == CS_LINEDRAW) {
						c = linedraw_to_acs(ea_buf[baddr].cc);
						if (c != -1)
							addch(c);
						else
							addch(' ');
					} else if (ea_buf[baddr].cs == CS_APL ||
						   (ea_buf[baddr].cs & CS_GE)) {
						c = apl_to_acs(ea_buf[baddr].cc);
						if (c != -1)
							addch(c);
						else
							addch(' ');
					} else {
						if (toggled(MONOCASE))
							addch(asc2uc[ebc2asc[ea_buf[baddr].cc]]);
						else
							addch(ebc2asc[ea_buf[baddr].cc]);
					}
#if defined(X3270_DBCS) /*[*/
				}
#endif /*]*/
			}
		}
	}
	attrset(defattr);
	if (flipped)
		move(cursor_addr / cCOLS, cCOLS-1 - (cursor_addr % cCOLS));
	else
		move(cursor_addr / cCOLS, cursor_addr % cCOLS);
	refresh();

	screen_has_changes = FALSE;
}

void screen_suspend(void)
{
	if(callbacks && callbacks->suspend)
		callbacks->suspend();
}

void screen_resume(void)
{
	screen_disp();
	refresh();

	if(callbacks && callbacks->resume)
		callbacks->resume();

}

void cursor_move(int baddr)
{
	if(cursor_addr == baddr) // Prevent unnecessary calls
		return;

	cursor_addr = baddr;

	if(callbacks && callbacks->move_cursor)
		callbacks->move_cursor(baddr/cCOLS, baddr%cCOLS);
}

void toggle_monocase(struct toggle *t unused, enum toggle_type tt unused)
{
	screen_disp();
}

/* Status line stuff. */

static Boolean status_im = False;
// static Boolean status_secure = False;
static Boolean oia_boxsolid = False;
static Boolean oia_undera = True;

// static char *status_msg = "";

void
status_ctlr_done(void)
{
	oia_undera = True;
	if(callbacks && callbacks->set)
		callbacks->set(OIA_FLAG_UNDERA,oia_undera);
}

void
status_insert_mode(Boolean on)
{
	status_im = on;
	if(callbacks && callbacks->set)
		callbacks->set(OIA_FLAG_INSERT,on);
}

void
status_minus(void)
{
	if(callbacks && callbacks->status)
		callbacks->status(STATUS_CODE_MINUS);
}

void
status_oerr(int error_type)
{
	STATUS_CODE sts = STATUS_CODE_USER;

	switch (error_type)
	{
	case KL_OERR_PROTECTED:
		sts = STATUS_CODE_PROTECTED;
		break;
	case KL_OERR_NUMERIC:
		sts = STATUS_CODE_NUMERIC;
		break;
	case KL_OERR_OVERFLOW:
		sts = STATUS_CODE_OVERFLOW;
		break;

	default:
		return;
	}

	if(callbacks && callbacks->status)
		callbacks->status(sts);

}

void status_resolving(Boolean on)
{
	if(callbacks)
	{
		if(callbacks->cursor)
			callbacks->cursor(on ? CURSOR_MODE_LOCKED : CURSOR_MODE_NORMAL);

		if(callbacks->status)
			callbacks->status(on ? STATUS_CODE_RESOLVING : STATUS_CODE_BLANK);
	}
}

void status_connecting(Boolean on)
{
	if(callbacks)
	{
		if(callbacks->cursor)
			callbacks->cursor(on ? CURSOR_MODE_LOCKED : CURSOR_MODE_NORMAL);

		if(callbacks->status)
			callbacks->status(on ? STATUS_CODE_CONNECTING : STATUS_CODE_BLANK);
	}
}

void
status_reset(void)
{
	if (kybdlock & KL_ENTER_INHIBIT)
	{
		if(callbacks && callbacks->status)
			callbacks->status(STATUS_CODE_INHIBIT);
	}
	else if (kybdlock & KL_DEFERRED_UNLOCK)
	{
		if(callbacks && callbacks->status)
			callbacks->status(STATUS_CODE_X);
	}
	else
	{
		if(callbacks && callbacks->status)
			callbacks->status(STATUS_CODE_BLANK);
	}

	if(screen_has_changes)
		screen_disp();

	if(callbacks && callbacks->reset)
		callbacks->reset(kybdlock);
}

void
status_reverse_mode(Boolean on)
{
	if(callbacks && callbacks->set)
		callbacks->set(OIA_FLAG_REVERSE,on);
}

void
status_syswait(void)
{
	if(callbacks && callbacks->status)
		callbacks->status(STATUS_CODE_SYSWAIT);
}

void
status_twait(void)
{
	oia_undera = False;
	if(callbacks && callbacks->status)
		callbacks->status(STATUS_CODE_TWAIT);
}

void
status_typeahead(Boolean on)
{
	if(callbacks && callbacks->set)
		callbacks->set(OIA_FLAG_TYPEAHEAD,on);
}

void
status_compose(Boolean on, unsigned char c, enum keytype keytype)
{
	if(callbacks && callbacks->compose)
		callbacks->compose(on,c,keytype);
}

void
status_lu(const char *lu)
{
	if(callbacks && callbacks->lu)
		callbacks->lu(lu);
}

static void
status_connect(Boolean connected)
{
	STATUS_CODE id = STATUS_CODE_USER;

	if (connected) {
		oia_boxsolid = IN_3270 && !IN_SSCP;

		if(callbacks && callbacks->set)
			callbacks->set(OIA_FLAG_BOXSOLID,oia_boxsolid);

		if (kybdlock & KL_AWAITING_FIRST)
			id = STATUS_CODE_AWAITING_FIRST;
		else
			id = STATUS_CODE_CONNECTED;

#if defined(HAVE_LIBSSL) /*[*/
		if(callbacks && callbacks->set)
			callbacks->set(OIA_FLAG_SECURE,secure_connection);
#endif /*]*/

	} else {
		oia_boxsolid = False;

		if(callbacks && callbacks->set)
			callbacks->set(OIA_FLAG_BOXSOLID,oia_boxsolid);

		if(callbacks && callbacks->set)
			callbacks->set(OIA_FLAG_SECURE,False);

		id = STATUS_CODE_DISCONNECTED;
	}

	if(callbacks && callbacks->status)
		callbacks->status(id);

}

static void
status_3270_mode(Boolean ignored unused)
{
	oia_boxsolid = IN_3270 && !IN_SSCP;
	if (oia_boxsolid)
		oia_undera = True;

	if(callbacks && callbacks->set)
	{
		callbacks->set(OIA_FLAG_BOXSOLID,oia_boxsolid);
		callbacks->set(OIA_FLAG_UNDERA,oia_undera);
	}

}

static void
status_printer(Boolean on)
{
	if(callbacks && callbacks->set)
		callbacks->set(OIA_FLAG_PRINTER,on);
}

void Redraw_action(Widget w unused, XEvent *event unused, String *params unused, Cardinal *num_params unused)
{
	if(callbacks && callbacks->redraw)
		callbacks->redraw();
	else
		screen_disp();
}

void ring_bell(void)
{
	if(callbacks && callbacks->ring_bell)
		callbacks->ring_bell();
}

void
screen_flip(void)
{
	flipped = !flipped;
	screen_disp();
}

void
screen_132(void)
{
}

void
screen_80(void)
{
}

/*
 * Translate an x3270 font line-drawing character (the first two rows of a
 * standard X11 fixed-width font) to an ASCII-art equivalent.
 *
 * Returns -1 if there is no translation.
 */
static int
linedraw_to_acs(unsigned char c)
{
    	int r;

	/* FIXME: Need aplmap equivalent functionality for xterm linedraw. */

	/* Use Unicode. */
	switch (c) {
	case 0x0:	/* '_', block */
		r = -1;
		break;
	case 0x1:	/* '`', diamond */
		r = 0x25c6;
		break;
	case 0x2:	/* 'a', checkerboard */
		r = -1;
		break;
	case 0x7:	/* 'f', degree */
		r = 0xb0;
		break;
	case 0x8:	/* 'g', plusminus */
		r = 0xb1;
		break;
	case 0x9:	/* 'h', board? */
		r = -1;
		break;
	case 0xa:	/* 'i', lantern? */
		r = -1;
		break;
	case 0xb:	/* 'j', LR corner */
		r = 0x2518;
		break;
	case 0xc:	/* 'k', UR corner */
		r = 0x2510;
		break;
	case 0xd:	/* 'l', UL corner */
		r = 0x250c;
		break;
	case 0xe:	/* 'm', LL corner */
		r = 0x2514;
		break;
	case 0xf:	/* 'n', plus */
		r = 0x253c;
		break;
	case 0x10:	/* 'o', top horizontal */
		r = '-';
		break;
	case 0x11:	/* 'p', row 3 horizontal */
		r = '-';
		break;
	case 0x12:	/* 'q', middle horizontal */
		r = 0x2500;
		break;
	case 0x13:	/* 'r', row 7 horizontal */
		r = '-';
		break;
	case 0x14:	/* 's', bottom horizontal */
		r = '_';
		break;
	case 0x15:	/* 't', left tee */
		r = 0x251c;
		break;
	case 0x16:	/* 'u', right tee */
		r = 0x2524;
		break;
	case 0x17:	/* 'v', bottom tee */
		r = 0x2534;
		break;
	case 0x18:	/* 'w', top tee */
		r = 0x252c;
		break;
	case 0x19:	/* 'x', vertical line */
		r = 0x2502;
		break;
	case 0x1a:	/* 'y', less or equal */
		r = 0x2264;
		break;
	case 0x1b:	/* 'z', greater or equal */
		r = 0x2265;
		break;
	case 0x1c:	/* '{', pi */
		r = 0x03c0;
		break;
	case 0x1d:	/* '|', not equal */
		r = 0x2260;
		break;
	case 0x1e:	/* '}', sterling */
		r = 0xa3;
		break;
	case 0x1f:	/* '~', bullet */
		r = 0x2022;
		break;
	default:
		r = -1;
		break;
	}

	/* If we're pre-NT, we can't assume that Unicode works. */
	if (!is_nt && (r & ~0xff))
	    	r = -1;

	return r;
}

int have_aplmap = 0;
unsigned char aplmap[256];

static int
apl_to_acs(unsigned char c)
{
    	int r;

	/* If there's an explicit map for this Windows code page, use it. */
	if (have_aplmap) {
	    	r = aplmap[c];
		return r? r: -1;
	}

	/* Use Unicode. */
	switch (c) {
	case 0xaf: /* CG 0xd1, degree */
		r = 0xb0;	/* XXX may not map to bullet in current
				       codepage */
		break;
	case 0xd4: /* CG 0xac, LR corner */
		r = 0x2518;
		break;
	case 0xd5: /* CG 0xad, UR corner */
		r = 0x2510;
		break;
	case 0xc5: /* CG 0xa4, UL corner */
		r = 0x250c;
		break;
	case 0xc4: /* CG 0xa3, LL corner */
		r = 0x2514;
		break;
	case 0xd3: /* CG 0xab, plus */
		r = 0x253c;
		break;
	case 0xa2: /* CG 0x92, horizontal line */
		r = 0x2500;
		break;
	case 0xc6: /* CG 0xa5, left tee */
		r = 0x251c;
		break;
	case 0xd6: /* CG 0xae, right tee */
		r = 0x2524;
		break;
	case 0xc7: /* CG 0xa6, bottom tee */
		r = 0x2534;
		break;
	case 0xd7: /* CG 0xaf, top tee */
		r = 0x252c;
		break;
	case 0xbf: /* CG 0x15b, stile */
	case 0x85: /* CG 0x184, vertical line */
		r = 0x2502;
		break;
	case 0x8c: /* CG 0xf7, less or equal */
		r = 0x2264;
		break;
	case 0xae: /* CG 0xd9, greater or equal */
		r = 0x2265;
		break;
	case 0xbe: /* CG 0x3e, not equal */
		r = 0x2260;
		break;
	case 0xa3: /* CG 0x93, bullet */
		r = 0x2022;
		break;
	case 0xad:
		r = '[';
		break;
	case 0xbd:
		r = ']';
		break;
	default:
		r = -1;
		break;
	}

	/* If pre-NT, we can't assume that Unicode works. */
	if (!is_nt && (r & ~0xff))
	    	r = -1;

	return r;
}

/* Read the aplMap.<windows-codepage> resource into aplmap[]. */
static void
check_aplmap(int codepage)
{
	char *r = get_fresource("%s.%d", ResAplMap, codepage);
	char *s;
	char *left, *right;

	if (r == CN) {
	    	return;
	}

	have_aplmap = 1;
	r = NewString(r);
	s = r;
	while (split_dresource(&s, &left, &right) == 1) {
	    	unsigned long l, r;

		l = strtoul(left, NULL, 0);
		r = strtoul(right, NULL, 0);
		if (l > 0 && l <= 0xff && r > 0 && r <= 0xff) {
		    	aplmap[l] = (unsigned char)r;
		}
	}
	Free(r);
}

void
Paste_action(Widget w unused, XEvent *event, String *params,
    Cardinal *num_params)
{
}

/* Set the window title. */
void
screen_title(char *text)
{
	if(callbacks && callbacks->title)
		callbacks->title(text);
}

void
Title_action(Widget w unused, XEvent *event, String *params,
    Cardinal *num_params)
{
	action_debug(Title_action, event, params, num_params);

	if (check_usage(Title_action, *num_params, 1, 1) < 0)
		return;

	screen_title(params[0]);
}

static void
relabel(Boolean ignored unused)
{
	if (appres.title != CN)
	    	return;

	if (PCONNECTED) {

		if (profile_name != CN)
			screen_title(profile_name);
		else
			screen_title(reconnect_host);

	} else {
	    	screen_title(0);
	}
}

void screen_changed(int bstart, int bend)
{
	screen_has_changes = TRUE;

	if(!callbacks)
		return;

	/* If the application can manage screen changes, let it do it */
	if(callbacks->changed)
	{
		callbacks->changed(bstart,bend);
		return;
	}

}

int Register3270ScreenCallbacks(const struct lib3270_screen_callbacks *cbk)
{
	if(!cbk)
		return EINVAL;

	if(cbk->sz != sizeof(struct lib3270_screen_callbacks))
		return -EINVAL;

	callbacks = cbk;

	return 0;
}

void Error(const char *s)
{
	WriteLog("Error","%s",s);

	if(callbacks && callbacks->Error)
		callbacks->Error(s);
	else
		exit(1);
}

void Warning(const char *s)
{
	WriteLog("Warning","%s",s);

	if(callbacks && callbacks->Warning)
		callbacks->Warning(s);
}

void mcursor_locked()
{
	if(callbacks && callbacks->cursor)
		callbacks->cursor(CURSOR_MODE_LOCKED);
}

extern void mcursor_normal()
{
	if(callbacks && callbacks->cursor)
		callbacks->cursor(CURSOR_MODE_NORMAL);
}

extern void mcursor_waiting()
{
	if(callbacks && callbacks->cursor)
		callbacks->cursor(CURSOR_MODE_WAITING);
}

/* Pop up an error dialog. */
extern void popup_an_error(const char *fmt, ...)
{
	char 	vmsgbuf[4096];
	va_list	args;

	va_start(args, fmt);
	(void) vsprintf(vmsgbuf, fmt, args);
	va_end(args);

	if(callbacks && callbacks->popup_an_error)
		callbacks->popup_an_error(vmsgbuf);
	else
		WriteLog("3270","Error Popup: %s",vmsgbuf);

}
