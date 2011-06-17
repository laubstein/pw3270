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
 * Este programa está nomeado como toggles.c e possui 253 linhas de código.
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
 *	toggles.c
 *		This module handles toggles.
 */

#include <errno.h>
#include <lib3270/config.h>
#include "globals.h"
#include "appres.h"

#include "ansic.h"
#include "actionsc.h"
#include "ctlrc.h"
#include "menubarc.h"
#include "popupsc.h"
#include "screenc.h"
#include "trace_dsc.h"
#include "togglesc.h"

#if defined(LIB3270)
	#include <lib3270/api.h>
#endif



#if defined(LIB3270)

static void no_callback(int value, enum toggle_type reason)
{
}

/* Register a callback to monitor toggle changes */
LIB3270_EXPORT int register_tchange(int ix, void (*callback)(int value, enum toggle_type reason))
{
	struct toggle *t;

	if(ix < 0 || ix >= N_TOGGLES)
		return EINVAL;

	t = &appres.toggle[ix];

	if(callback)
	{
		t->callback = callback;
		t->callback(t->value, (int) TT_INITIAL);
	}
	else
	{
		t->callback = no_callback;
	}

	return 0;
}

int Toggled(int ix)
{
	if(ix < 0 || ix >= N_TOGGLES)
		return 0;
	return (int) appres.toggle[ix].value;
}
#endif
/*
 * Generic toggle stuff
 */
static void
do_toggle_reason(int ix, enum toggle_type reason)
{
	struct toggle *t = &appres.toggle[ix];

	/*
	 * Change the value, call the internal update routine, and reset the
	 * menu label(s).
	 */
	toggle_toggle(t);
	t->upcall(t, reason);

#if defined(X3270_MENUS) /*[*/
	menubar_retoggle(t);
#endif /*]*/

#if defined(LIB3270)
	t->callback(t->value, (int) reason);
	notify_toggle_changed(ix, t->value, reason);
#endif

}

LIB3270_EXPORT int set_toggle(int ix, int value)
{
	Boolean v = ((Boolean) (value != 0)); // Convert int in Boolean

	struct toggle	*t;

	if(ix < 0 || ix >= N_TOGGLES)
		return EINVAL;

	t = &appres.toggle[ix];

	if(t->value == v)
		return 0;

	do_toggle_reason(ix, TT_INTERACTIVE);

	return 0;
}

LIB3270_EXPORT int do_toggle(int ix)
{
	if(ix < 0 || ix >= N_TOGGLES)
		return EINVAL;

	do_toggle_reason(ix, TT_INTERACTIVE);
	return 0;
}

/*
 * Called from system initialization code to handle initial toggle settings.
 */
void
initialize_toggles(void)
{
	int f;

#if defined(LIB3270)
	for(f=0;f<N_TOGGLES;f++)
	{
		appres.toggle[f].callback = no_callback;
		appres.toggle[f].upcall = toggle_nop;
	}
#endif

//#if defined(X3270_DISPLAY) || defined(C3270) /*[*/
// 	appres.toggle[MONOCASE].upcall =         toggle_monocase;
//#endif /*]*/

#if defined(X3270_DISPLAY) /*[*/
	appres.toggle[ALT_CURSOR].upcall =       toggle_altCursor;
	appres.toggle[CURSOR_BLINK].upcall =     toggle_cursorBlink;
	appres.toggle[SHOW_TIMING].upcall =      toggle_showTiming;
	appres.toggle[CURSOR_POS].upcall =       toggle_cursorPos;
	appres.toggle[SCROLL_BAR].upcall =       toggle_scrollBar;
	appres.toggle[CROSSHAIR].upcall =        toggle_crosshair;
	appres.toggle[VISIBLE_CONTROL].upcall =  toggle_visible_control;
#endif /*]*/
#if defined(X3270_TRACE) /*[*/
	appres.toggle[DS_TRACE].upcall =         toggle_dsTrace;
	appres.toggle[SCREEN_TRACE].upcall =     toggle_screenTrace;
	appres.toggle[EVENT_TRACE].upcall =      toggle_eventTrace;
#endif /*]*/
#if defined(X3270_ANSI) /*[*/
	appres.toggle[LINE_WRAP].upcall =        toggle_lineWrap;
#endif /*]*/

#if defined(X3270_TRACE) /*[*/
	if (toggled(DS_TRACE))
		appres.toggle[DS_TRACE].upcall(&appres.toggle[DS_TRACE],
		    TT_INITIAL);
	if (toggled(EVENT_TRACE))
		appres.toggle[EVENT_TRACE].upcall(&appres.toggle[EVENT_TRACE],
		    TT_INITIAL);
	if (toggled(SCREEN_TRACE))
		appres.toggle[SCREEN_TRACE].upcall(&appres.toggle[SCREEN_TRACE],
		    TT_INITIAL);
#endif /*]*/

#if defined(DEFAULT_TOGGLE_CURSOR_POS)
	appres.toggle[CURSOR_POS].value = True;
#endif /*]*/

#if defined(DEFAULT_TOGGLE_RECTANGLE_SELECT)
	appres.toggle[RECTANGLE_SELECT].value = True;
#endif

}

/*
 * Called from system exit code to handle toggles.
 */
void
shutdown_toggles(void)
{
#if defined(X3270_TRACE) /*[*/
	/* Clean up the data stream trace monitor window. */
	if (toggled(DS_TRACE)) {
		appres.toggle[DS_TRACE].value = False;
		toggle_dsTrace(&appres.toggle[DS_TRACE], TT_FINAL);
	}
	if (toggled(EVENT_TRACE)) {
		appres.toggle[EVENT_TRACE].value = False;
		toggle_dsTrace(&appres.toggle[EVENT_TRACE], TT_FINAL);
	}

	/* Clean up the screen trace file. */
	if (toggled(SCREEN_TRACE)) {
		appres.toggle[SCREEN_TRACE].value = False;
		toggle_screenTrace(&appres.toggle[SCREEN_TRACE], TT_FINAL);
	}
#endif /*]*/
}

LIB3270_EXPORT const char *get_toggle_name(int ix)
{
	if(ix < N_TOGGLES)
		return toggle_names[ix];
	return "";
}

LIB3270_EXPORT int get_toggle_by_name(const char *name)
{
	int f;

	for(f=0;f<N_TOGGLES;f++)
	{
		if(!strcmp(name,toggle_names[f]))
			return f;
	}

	return -1;
}
