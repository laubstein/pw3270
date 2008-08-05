/*
 * Copyright 2000, 2001, 2002, 2004, 2005, 2006, 2007 by Paul Mattes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * lib3270 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/*
 *	screen.c
 *		A callback based 3270 Terminal Emulator
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
#include "statusc.h"
#include <lib3270/api.h>

#include <windows.h>
#include <wincon.h>
#include "winversc.h"


#define get_color_pair(fg,bg) (((bg&0x0F) << 4) | (fg&0x0F))
#define DEFCOLOR_MAP(f) ((((f) & FA_PROTECT) >> 4) | (((f) & FA_INT_HIGH_SEL) >> 3))

extern char *profile_name;

static const struct lib3270_screen_callbacks *callbacks = NULL;


static int defattr = 0;
// static unsigned long input_id;

// Boolean escaped = True;
Boolean screen_has_changes = FALSE;

enum ts { TS_AUTO, TS_ON, TS_OFF };
enum ts ab_mode = TS_AUTO;

int windows_cp = 0;

static void status_connect(Boolean ignored);
static void status_3270_mode(Boolean ignored);
static void status_printer(Boolean on);
static int color_from_fa(unsigned char fa);
static Boolean ts_value(const char *s, enum ts *tsp);
static int linedraw_to_acs(unsigned char c);
static int apl_to_acs(unsigned char c);
static void relabel(Boolean ignored);
// static void check_aplmap(int codepage);

/* Try to set the console output character set. */
void
set_display_charset(char *dcs)
{
	if(callbacks && callbacks->charset)
		callbacks->charset(dcs);
}

/*
 * Vitrual curses functions.
 */
// static int cur_row = 0;
// static int cur_col = 0;

/*
static int cur_attr = 0;

static void
attrset(int a)
{
	cur_attr = a;
}
*/

static void
addch(int row, int col, int c, int attr)
{
	if(callbacks && callbacks->addch)
		callbacks->addch(row, col, c, attr);

	// Increment and wrap.
	/*
	if (++cur_col >= maxCOLS)
	{
		cur_col = 0;
		if (++cur_row >= maxROWS)
			cur_row = 0;
	}
	*/
}

/* Initialize the screen. */
void
screen_init(void)
{

	/* Disallow altscreen/defscreen. */
	if ((appres.altscreen != CN) || (appres.defscreen != CN)) {
		(void) fprintf(stderr, "altscreen/defscreen not supported\n");
		x3270_exit(1);
	}

	/* Initialize the console. */
	if(callbacks && callbacks->init)
	{
		if(callbacks->init()) {
			(void) fprintf(stderr, "Can't initialize terminal.\n");
			x3270_exit(1);
		}
	}
	else
	{
		// FIXME (perry#1#): Do it in the right way (command-line parameter or configuration file).
		set_rows_cols(2, 80, 24);
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
//	init_user_colors();

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


/* Map a field attribute to its default colors. */
static int
color_from_fa(unsigned char fa)
{
	if (appres.m3279)
		return get_color_pair(DEFCOLOR_MAP(fa),0) | COLOR_ATTR_FIELD;

	// Green on black
	return get_color_pair(0,0) | COLOR_ATTR_FIELD | (((ab_mode == TS_ON) || FA_IS_HIGH(fa)) ? COLOR_ATTR_INTENSIFY : 0);
}

static int reverse_colors(int a)
{
	int bg = (a & 0xF0) >> 4;
	int fg = (a & 0x0F);
	return get_color_pair(bg,fg) | (a&0xFF00);
}

/*
 * Find the display attributes for a baddr, fa_addr and fa.
 */
static int
calc_attrs(int baddr, int fa_addr, int fa)
{
    	int fg = 0, bg = 0, gr, a;

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
		{
			fg = ea_buf[baddr].fg & 0x0f;
		}
		else if (ea_buf[fa_addr].fg)
		{
			fg = ea_buf[fa_addr].fg & 0x0f;
		}
		else
		{
			fg = DEFCOLOR_MAP(fa);
		}

		if (ea_buf[baddr].bg)
			bg = ea_buf[baddr].bg & 0x0f;
		else if (ea_buf[fa_addr].bg)
			bg = ea_buf[fa_addr].bg & 0x0f;
		else
			bg = 0;

		a = get_color_pair(fg, bg);
	}

	/* Compute the display attributes. */

	if (ea_buf[baddr].gr)
		gr = ea_buf[baddr].gr;
	else if (ea_buf[fa_addr].gr)
		gr = ea_buf[fa_addr].gr;
	else
		gr = 0;

	if(!(gr & GR_REVERSE) && !bg)
	{
		if(gr & GR_BLINK)
			a |= COLOR_ATTR_BLINK;

		if(gr & GR_UNDERLINE)
			a |= COLOR_ATTR_UNDERLINE;
	}


/*
	if (appres.highlight_underline &&
		appres.m3279 &&
		(gr & (GR_BLINK | GR_UNDERLINE)) &&
		!(gr & GR_REVERSE) &&
		!bg) {

	    	a |= BACKGROUND_INTENSITY;
	}
*/

	if(!appres.m3279 &&	((gr & GR_INTENSIFY) || (ab_mode == TS_ON) || FA_IS_HIGH(fa)))
		a |= COLOR_ATTR_INTENSIFY;

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
	int attr = defattr;
	unsigned char fa;
#if defined(X3270_DBCS) /*[*/
	enum dbcs_state d;
#endif /*]*/
	int fa_addr;

	fa = get_field_attribute(0);
	a = color_from_fa(fa);
	fa_addr = find_field_attribute(0); /* may be -1, that's okay */
	for (row = 0; row < ROWS; row++) {
		int baddr;

		for (col = 0; col < cCOLS; col++) {
			baddr = row*cCOLS+col;
			if (ea_buf[baddr].fa) {
			    	/* Field attribute. */
			    	fa_addr = baddr;
				fa = ea_buf[baddr].fa;
				a = calc_attrs(baddr, baddr, fa);
//				attrset(defattr);
				addch(row,col,' ',attr = defattr);
			} else if (FA_IS_ZERO(fa)) {
			    	/* Blank. */
//				attrset(a);
				addch(row,col,' ',attr=a);
			} else {
			    	/* Normal text. */
				if (!(ea_buf[baddr].gr ||
				      ea_buf[baddr].fg ||
				      ea_buf[baddr].bg)) {
					// attrset(a);
					attr = a;
				} else {
					int b;

					/*
					 * Override some of the field
					 * attributes.
					 */
					attr = b = calc_attrs(baddr, fa_addr, fa);
//					attrset(b);
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
							addch(row,col,c,attr);
						else
							addch(row,col,' ',attr);
					} else if (ea_buf[baddr].cs == CS_APL ||
						   (ea_buf[baddr].cs & CS_GE)) {
						c = apl_to_acs(ea_buf[baddr].cc);
						if (c != -1)
							addch(row,col,c,attr);
						else
							addch(row,col,' ',attr);
					} else {
						if (toggled(MONOCASE))
							addch(row,col,asc2uc[ebc2asc[ea_buf[baddr].cc]],attr);
						else
							addch(row,col,ebc2asc[ea_buf[baddr].cc],attr);
					}
#if defined(X3270_DBCS) /*[*/
				}
#endif /*]*/
			}
		}
	}
//	attrset(defattr);

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

static void set(OIA_FLAG id, Boolean on)
{
	if(callbacks && callbacks->set)
		callbacks->set(id,on);
}

void
status_ctlr_done(void)
{
	set(OIA_FLAG_UNDERA,True);
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
	set(OIA_FLAG_REVERSE,on);
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
	set(OIA_FLAG_UNDERA,False);
	if(callbacks && callbacks->status)
		callbacks->status(STATUS_CODE_TWAIT);
}

void
status_typeahead(Boolean on)
{
	set(OIA_FLAG_TYPEAHEAD,on);
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

		set(OIA_FLAG_BOXSOLID,IN_3270 && !IN_SSCP);

		if (kybdlock & KL_AWAITING_FIRST)
			id = STATUS_CODE_AWAITING_FIRST;
		else
			id = STATUS_CODE_CONNECTED;

#if defined(HAVE_LIBSSL) /*[*/
		set(OIA_FLAG_SECURE,secure_connection);
#endif /*]*/

	} else {
		set(OIA_FLAG_BOXSOLID,False);
		set(OIA_FLAG_SECURE,False);

		id = STATUS_CODE_DISCONNECTED;
	}

	if(callbacks && callbacks->status)
		callbacks->status(id);

}

static void
status_3270_mode(Boolean ignored unused)
{
	Boolean oia_boxsolid = (IN_3270 && !IN_SSCP);
	if(oia_boxsolid)
		set(OIA_FLAG_UNDERA,True);
	set(OIA_FLAG_BOXSOLID,oia_boxsolid);

}

static void
status_printer(Boolean on)
{
	set(OIA_FLAG_PRINTER,on);
}

void status_script(Boolean on)
{
	set(OIA_FLAG_SCRIPT,on);
}

void status_timing(struct timeval *t0, struct timeval *t1)
{
	if(callbacks && callbacks->show_timer)
		callbacks->show_timer(t1->tv_sec - t0->tv_sec);
}

void status_untiming(void)
{
	if(callbacks && callbacks->show_timer)
		callbacks->show_timer(-1);
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

/* Read the aplMap.<windows-codepage> resource into aplmap[]. */ /*
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
*/

/*
void
Paste_action(Widget w unused, XEvent *event, String *params,
    Cardinal *num_params)
{
}
*/

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

	/* If the application can manage screen changes, let it do it */
	if(callbacks && callbacks->changed)
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

#if defined(LIB3270)
void notify_toggle_changed(int ix, int value, int reason)
{
	if(callbacks && callbacks->toggle_changed)
		callbacks->toggle_changed(ix,value,reason,toggle_names[ix]);
}
#endif

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

struct ea * copy_device_buffer(int *el)
{
	int			sz		=  sizeof(struct ea) * (maxROWS * maxCOLS);
	struct ea	*ret	=  malloc(sz);
	memcpy(ret,ea_buf,sz);
	if(el)
		*el = (maxROWS * maxCOLS);
	return ret;
}
