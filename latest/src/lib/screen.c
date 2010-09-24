/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270. Registro no INPI sob o nome G3270.
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
 * Este programa está nomeado como screen.c e possui 894 linhas de código.
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
// #include "keymapc.h"
#include "kybdc.h"
// #include "macrosc.h"
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
#include "togglesc.h"
#include <lib3270/api.h>

#if defined(_WIN32)
	#include <windows.h>
	#include <wincon.h>
	#include "winversc.h"
#else
	#include <stdarg.h>
#endif


#define get_color_pair(fg,bg) (((bg&0x0F) << 4) | (fg&0x0F))
#define DEFCOLOR_MAP(f) ((((f) & FA_PROTECT) >> 4) | (((f) & FA_INT_HIGH_SEL) >> 3))

#if defined(WC3270) /*[*/
extern char *profile_name;
#endif

static const struct lib3270_screen_callbacks *callbacks = NULL;

int lib3270_event_counter[COUNTER_ID_USER] = { 0, 0, 0 };

Boolean screen_has_changes = 0;

enum ts { TS_AUTO, TS_ON, TS_OFF };
enum ts ab_mode = TS_AUTO;

// int windows_cp = 0;

static void status_connect(int ignored);
static void status_3270_mode(int ignored);
static void status_printer(int on);
static int color_from_fa(unsigned char fa);
static Boolean ts_value(const char *s, enum ts *tsp);
static void relabel(int ignored);

void set_display_charset(char *dcs)
{
	if(callbacks && callbacks->charset)
		callbacks->charset(dcs);
}

static void
addch(int row, int col, int c, int attr)
{
	if(callbacks && callbacks->addch)
		callbacks->addch(row, col, c, attr);
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

	/* Pull in the user's color mappings. */

	/* Set up the controller. */
	ctlr_init(-1);
	ctlr_reinit(-1);

	/* Set the window label. */
#if defined(WC3270) /*[*/

	if (appres.title != CN)
		screen_title(appres.title);
	else if (profile_name != CN)
		screen_title(profile_name);
	else
		screen_title(NULL);
#endif

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
		screen_has_changes = 0;
		return;
	}

	/* No callback, just redraw */
	screen_disp();
}

void screen_size(int *rows, int *cols)
{
	*rows = ROWS;
	*cols = cCOLS;
}

/* Get screen contents */
int screen_read(char *dest, int baddr, int count)
{
	unsigned char fa	= get_field_attribute(baddr);
	int 			max = (maxROWS * maxCOLS);

	*dest = 0;

	while(count-- > 0)
	{
		if(baddr > max)
		{
			*dest = 0;
			return EFAULT;
		}

		if (ea_buf[baddr].fa)
			fa = ea_buf[baddr].fa;

		if(FA_IS_ZERO(fa))
			*dest = ' ';
		else if(ea_buf[baddr].cc)
			*dest = ebc2asc[ea_buf[baddr].cc];
		else
			*dest = ' ';

		dest++;
		baddr++;
	}
	*dest = 0;

	return 0;
}

/* Display what's in the buffer. */
void screen_disp(void)
{
	int row, col;
	int a;
	int attr = COLOR_GREEN;
	unsigned char fa;
#if defined(X3270_DBCS) /*[*/
	enum dbcs_state d;
#endif /*]*/
	int fa_addr;

	fa = get_field_attribute(0);
	a = color_from_fa(fa);
	fa_addr = find_field_attribute(0); /* may be -1, that's okay */
	for (row = 0; row < ROWS; row++)
	{
		int baddr;

		for (col = 0; col < cCOLS; col++)
		{
			baddr = row*cCOLS+col;
			if(ea_buf[baddr].fa)
			{
			    /* Field attribute. */
				fa_addr = baddr;
				fa = ea_buf[baddr].fa;
				a = calc_attrs(baddr, baddr, fa);
				addch(row,col,' ',(attr = COLOR_GREEN)|CHAR_ATTR_MARKER);
			}
			else if (FA_IS_ZERO(fa))
			{
			   	/* Blank. */
				addch(row,col,' ',attr=a);
			}
			else
			{
			    	/* Normal text. */
				if (!(ea_buf[baddr].gr ||
				      ea_buf[baddr].fg ||
				      ea_buf[baddr].bg)) {
					attr = a;
				} else {
					int b;

					/*
					 * Override some of the field
					 * attributes.
					 */
					attr = b = calc_attrs(baddr, fa_addr, fa);
				}

				if (ea_buf[baddr].cs == CS_LINEDRAW)
				{
					addch(row,col,ea_buf[baddr].cc,attr);
				}
				else if (ea_buf[baddr].cs == CS_APL || (ea_buf[baddr].cs & CS_GE))
				{
					addch(row,col,ea_buf[baddr].cc,attr|CHAR_ATTR_CG);
				}
				else
				{
					if (toggled(MONOCASE))
						addch(row,col,asc2uc[ebc2asc[ea_buf[baddr].cc]],attr);
					else
						addch(row,col,ebc2asc[ea_buf[baddr].cc],attr);
				}
			}
		}
	}

	if(callbacks && callbacks->display)
		callbacks->display();

	screen_has_changes = 0;
}

void screen_suspend(void)
{
	if(callbacks && callbacks->set_suspended)
		callbacks->set_suspended(1);
}

void screen_resume(void)
{
	screen_disp();

	if(callbacks && callbacks->set_suspended)
		callbacks->set_suspended(0);

}

int cursor_get_addr(void)
{
    return cursor_addr;
}

int cursor_set_addr(int baddr)
{
    int ret = cursor_addr;

    if(cursor_addr != baddr)
    {
        cursor_addr = baddr;

        if(callbacks && callbacks->move_cursor)
            callbacks->move_cursor(baddr/cCOLS, baddr%cCOLS);

		Trace("%s: baddr=%d cc=%04x",__FUNCTION__,baddr, ea_buf[baddr].cc);

    }

    return ret;
}

/* Status line stuff. */

static void set(OIA_FLAG id, Boolean on)
{
	if(callbacks && callbacks->set)
		callbacks->set(id,on);
}

void status_ctlr_done(void)
{
	lib3270_event_counter[COUNTER_ID_CTLR_DONE]++;
	set(OIA_FLAG_UNDERA,True);
}

/*
void
status_minus(void)
{
	status_changed(STATUS_CODE_MINUS);
}
*/

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

	status_changed(sts);

}

void status_resolving(Boolean on)
{
	if(callbacks && callbacks->cursor)
			callbacks->cursor(on ? CURSOR_MODE_LOCKED : CURSOR_MODE_NORMAL);

	status_changed(on ? STATUS_CODE_RESOLVING : STATUS_CODE_BLANK);
}

void status_connecting(Boolean on)
{
	if(callbacks && callbacks->cursor)
			callbacks->cursor(on ? CURSOR_MODE_LOCKED : CURSOR_MODE_NORMAL);

	status_changed(on ? STATUS_CODE_CONNECTING : STATUS_CODE_BLANK);
}

void
status_reset(void)
{
	if (kybdlock & KL_ENTER_INHIBIT)
		status_changed(STATUS_CODE_INHIBIT);
	else if (kybdlock & KL_DEFERRED_UNLOCK)
		status_changed(STATUS_CODE_X);
	else
		status_changed(STATUS_CODE_BLANK);

	if(screen_has_changes)
		screen_disp();

	if(callbacks && callbacks->reset)
	{
		Trace("%s calling reset",__FUNCTION__);
		callbacks->reset(kybdlock);
	}

}

LIB3270_EXPORT void status_reverse_mode(int on)
{
	set(OIA_FLAG_REVERSE,on);
}

static STATUS_CODE current_status_code = -1;

/**
 * Query the updated terminal status.
 *
 * @return status-code.
 *
 * @see STATUS_CODE
 */
LIB3270_EXPORT STATUS_CODE query_3270_terminal_status(void)
{
	return current_status_code;
}

LIB3270_EXPORT void status_changed(STATUS_CODE id)
{
	if(id == current_status_code)
		return;

	current_status_code = id;

	if(callbacks && callbacks->status)
		callbacks->status(id);
}

/*
void
status_syswait(void)
{
	status_changed(STATUS_CODE_SYSWAIT);
}
*/

LIB3270_EXPORT void status_twait(void)
{
	set(OIA_FLAG_UNDERA,False);
	status_changed(STATUS_CODE_TWAIT);
}

LIB3270_EXPORT void status_typeahead(int on)
{
	set(OIA_FLAG_TYPEAHEAD,on);
}

/*
LIB3270_EXPORT void status_compose(int on, unsigned char c, enum keytype keytype)
{
	if(callbacks && callbacks->compose)
		callbacks->compose(on,c,keytype);
}
*/

void
status_lu(const char *lu)
{
	if(callbacks && callbacks->lu)
		callbacks->lu(lu);
}

static void
status_connect(int connected)
{
	STATUS_CODE id = STATUS_CODE_USER;

	if (connected) {

		set(OIA_FLAG_BOXSOLID,IN_3270 && !IN_SSCP);

		if (kybdlock & KL_AWAITING_FIRST)
			id = STATUS_CODE_AWAITING_FIRST;
		else
			id = STATUS_CODE_CONNECTED;

#if defined(HAVE_LIBSSL) /*[*/
		set(OIA_FLAG_SECURE,h3270.secure_connection);
#endif /*]*/

	} else {
		set(OIA_FLAG_BOXSOLID,False);
		set(OIA_FLAG_SECURE,False);

		id = STATUS_CODE_DISCONNECTED;
	}

	status_changed(id);

}

static void status_3270_mode(int ignored unused)
{
	Boolean oia_boxsolid = (IN_3270 && !IN_SSCP);
	if(oia_boxsolid)
		set(OIA_FLAG_UNDERA,True);
	set(OIA_FLAG_BOXSOLID,oia_boxsolid);

}

static void status_printer(int on)
{
	set(OIA_FLAG_PRINTER,on);
}

static SCRIPT_STATE script_state = SCRIPT_STATE_NONE;

LIB3270_EXPORT SCRIPT_STATE status_script(SCRIPT_STATE state)
{
	if(state != script_state && callbacks && callbacks->set_script)
		callbacks->set_script(state);
	return script_state = state;
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
relabel(int ignored unused)
{
#if defined(WC3270) /*[*/
	if (appres.title != CN)
	    	return;
#endif

	if (PCONNECTED)
	{
#if defined(WC3270) /*[*/
		if (profile_name != CN)
			screen_title(profile_name);
		else
#endif
			screen_title(reconnect_host);

	}
	else
	{
	    	screen_title(0);
	}
}

int query_counter(COUNTER_ID id)
{
	return lib3270_event_counter[id];
}

void screen_changed(int bstart, int bend)
{
	screen_has_changes = 1;
	lib3270_event_counter[COUNTER_ID_SCREEN_CHANGED]++;

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

LIB3270_EXPORT void show_3270_popup_dialog(H3270 *session, PW3270_DIALOG type, const char *title, const char *msg, const char *fmt, ...)
{
	if(!fmt)
		fmt = "";

	va_list arg_ptr;
	va_start(arg_ptr, fmt);

	if(callbacks && callbacks->popup_dialog)
		callbacks->popup_dialog(session,type,title,msg,fmt,arg_ptr);

	va_end(arg_ptr);

}

void Error(const char *fmt, ...)
{
	va_list arg_ptr;

	va_start(arg_ptr, fmt);

	if(callbacks && callbacks->Warning)
		callbacks->Warning(fmt,arg_ptr);

	va_end(arg_ptr);

}

#if defined(LIB3270)
void notify_toggle_changed(int ix, int value, int reason)
{
	if(callbacks && callbacks->toggle_changed)
		callbacks->toggle_changed(ix,value,reason,toggle_names[ix]);
}

LIB3270_EXPORT void update_toggle_actions(void)
{
	int f;

	if(callbacks && callbacks->toggle_changed)
	{
		for(f=0;f< N_TOGGLES;f++)
			callbacks->toggle_changed(f,appres.toggle[f].value,TT_UPDATE,toggle_names[f]);
	}
}

#endif

void Warning(const char *fmt, ...)
{
	va_list arg_ptr;

	va_start(arg_ptr, fmt);

	if(callbacks && callbacks->Warning)
		callbacks->Warning(fmt,arg_ptr);

	va_end(arg_ptr);

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
	va_list	args;

	va_start(args, fmt);
	if(callbacks && callbacks->Error)
		callbacks->Error(fmt,args);
	va_end(args);

}

LIB3270_EXPORT void popup_system_error(const char *title, const char *message, const char *system)
{
	if(callbacks && callbacks->SysError)
	{
		callbacks->SysError(title,message,system);
		return;
	}

	popup_an_error("%s\n%s\n%s",title,message,system);
}

LIB3270_EXPORT int set_device_buffer(struct ea *src, int el)
{

	if(el > (maxROWS * maxCOLS))
		return EINVAL;

	memcpy(ea_buf,src,sizeof(struct ea) * el);

	screen_disp();

	return 0;
}

LIB3270_EXPORT void show_3270_test_pattern(void)
{
	static const unsigned char text_pat[] =
	{
		// Latin-1 chars
		0x48, 0x68, 0x45, 0x51, 0x55, 0xce, 0xde, 0x46, 0xcf, 0x65, 0x71, 0x75, 0xee, 0xfe, 0x66, 0xef,
		0x90, 0x9a, 0x9b, 0xb5, 0x42, 0x52, 0x56, 0xcb, 0xdb, 0x62, 0x72, 0x76, 0xeb, 0xfb,

		// Regular ASCII chars
		0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86,
		0x87, 0x88, 0x89, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0xa2, 0xa3, 0xa4, 0xa5,
		0xa6, 0xa7, 0xa8, 0xa9, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xd1, 0xd2, 0xd3,
		0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0x7d, 0x5a,
		0x7c, 0x7b, 0x5b, 0x6c, 0x50, 0x5c, 0x4d, 0x5d, 0x60, 0x6d, 0x4e, 0x7e, 0x7d, 0x79, 0xad, 0xc0,
		0xa1, 0xb0, 0xd0, 0xbd, 0x6b, 0x4c, 0x4b, 0x6e, 0x5e, 0x7a, 0x61, 0x6f, 0x7f,

		// End marker
		0x00
	};

	static const unsigned char cg_pat[] =
	{
		0x8c, // CG 0xf7, less or equal "≤"
		0xae, // CG 0xd9, greater or equal "≥"
		0xbe, // CG 0x3e, not equal "≠"
		0xad, // "["
		0xbd, // "]"
		0xc6, // CG 0xa5, left tee
		0xd3, // CG 0xab, plus
		0xa2, // CG 0x92, horizontal line
		0xc7, // CG 0xa6, bottom tee
		0xd7, // CG 0xaf, top tee
		0xd6, // CG 0xae, right tee
		0xc5, // CG 0xa4, UL corner
		0xd5, // CG 0xad, UR corner
		0x85, // CG 0x184, vertical line
		0xc4, // CG 0xa3, LL corner
		0xd4, // CG 0xac, LR corner

		0x00

	};

	static const struct _pat
	{
		unsigned char cs;
		const unsigned char *cc;
	} pat[] =
	{
		{ CS_APL,	cg_pat		},
		{ 0,		text_pat	},
	};

	int row = 0;
	int max = (maxROWS * maxCOLS);
	int pos = 0;
	int f;

	Trace("%s begins",__FUNCTION__);
	for(f=0;f<max;f++)
	{
		if(!pat[row].cc[pos])
		{
			if(++row >= (sizeof(pat)/sizeof(struct _pat)) )
				row = 0;
			pos = 0;
		}
		ea_buf[f].cs = pat[row].cs;
		ea_buf[f].cc = pat[row].cc[pos++];
	}

	Trace("%s display",__FUNCTION__);

	screen_disp();

	Trace("%s ends",__FUNCTION__);
}

LIB3270_EXPORT struct ea * copy_device_buffer(int *el)
{
	int			sz		=  sizeof(struct ea) * (maxROWS * maxCOLS);
	struct ea	*ret	=  malloc(sz);
	memcpy(ret,ea_buf,sz);
	if(el)
		*el = (maxROWS * maxCOLS);
	return ret;
}

LIB3270_EXPORT HCONSOLE console_window_new(const char *title, const char *label)
{
	if(callbacks && callbacks->console_new )
		return callbacks->console_new(title,label);

	return NULL;
}

LIB3270_EXPORT void console_window_delete(HCONSOLE hwnd)
{
	if(callbacks && callbacks->console_delete )
		callbacks->console_delete(hwnd);
}

LIB3270_EXPORT int console_window_append(HCONSOLE hwnd, const char *fmt, ...)
{
	va_list args;

	if(callbacks && callbacks->console_append)
	{
		va_start(args, fmt);
		callbacks->console_append(hwnd, fmt, args);
		va_end(args);
	}

	return 0;
}

LIB3270_EXPORT char * console_window_wait_for_user_entry(HCONSOLE hwnd)
{
	if(callbacks && callbacks->console_entry )
		return callbacks->console_entry(hwnd);
	return NULL;
}

