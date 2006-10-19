/*
 * Copyright 2000, 2001, 2002, 2004, 2005 by Paul Mattes.
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
 *
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
#include "widec.h"
#include "xioc.h"
#include "lib3270.h"

/*---[ Publics ]--------------------------------------------------------------*/

const SCREEN_CALLBACK *screen_callbacks_3270	= 0;
Boolean 		 	   escaped 					= True;

/* Set callback structure */
int set_3270_screen(const SCREEN_CALLBACK *scr)
{
	screen_callbacks_3270 = scr;
	return 0;
}

/* Initialize the screen. */
void screen_init(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->screen_init)
       screen_callbacks_3270->screen_init();
}

/* Display what's in the buffer. */
void screen_disp(Boolean erasing)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->screen_disp)
       screen_callbacks_3270->screen_disp(erasing);
}

void screen_suspend(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->screen_suspend)
       screen_callbacks_3270->screen_suspend();
}

void screen_resume(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->screen_resume)
       screen_callbacks_3270->screen_resume();
}

void cursor_move(int baddr)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->cursor_move)
       screen_callbacks_3270->cursor_move(baddr);
}

void toggle_monocase(struct toggle *t, enum toggle_type tt)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->toggle_monocase)
       screen_callbacks_3270->toggle_monocase(t,tt);
}

void status_ctlr_done(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->status_ctlr_done)
       screen_callbacks_3270->status_ctlr_done();
}

void status_insert_mode(Boolean on)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->status_insert_mode)
       screen_callbacks_3270->status_insert_mode(on);
}

void status_minus(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->status_minus)
       screen_callbacks_3270->status_minus();
}

void status_oerr(int error_type)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->status_oerr)
       screen_callbacks_3270->status_oerr(error_type);
}

void status_reset(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->status_reset)
       screen_callbacks_3270->status_reset();
}

void status_reverse_mode(Boolean on)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->status_reverse_mode)
       screen_callbacks_3270->status_reverse_mode(on);
}

void status_syswait(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->status_syswait)
       screen_callbacks_3270->status_syswait();
}

void status_twait(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->status_twait)
       screen_callbacks_3270->status_twait();
}

void status_typeahead(Boolean on)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->status_typeahead)
       screen_callbacks_3270->status_typeahead(on);
}

void status_compose(Boolean on, unsigned char c, enum keytype keytype)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->status_compose)
       screen_callbacks_3270->status_compose(on,c,keytype);
}

void status_lu(const char *lu)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->status_lu)
       screen_callbacks_3270->status_lu(lu);
}

void Redraw_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->Redraw_action)
       screen_callbacks_3270->Redraw_action(w,event,params,num_params);
}

void ring_bell(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->ring_bell)
       screen_callbacks_3270->ring_bell();
}

void screen_flip(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->screen_flip)
       screen_callbacks_3270->screen_flip();
}

void screen_132(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->screen_width)
       screen_callbacks_3270->screen_width(132);
}

void screen_80(void)
{
	if(screen_callbacks_3270 && screen_callbacks_3270->screen_width)
       screen_callbacks_3270->screen_width(80);
}

