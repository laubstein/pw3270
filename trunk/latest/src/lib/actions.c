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
 * Este programa está nomeado como actions.c e possui 877 linhas de código.
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
 *	actions.c
 *		The X actions table and action debugging code.
 */

#include "globals.h"
#include "appres.h"

#include "actionsc.h"
#include "hostc.h"
// #include "keymapc.h"
#include "kybdc.h"
// #include "macrosc.h"
#include "popupsc.h"
#include "printc.h"
#include "resources.h"
// #include "selectc.h"
#include "togglesc.h"
#include "trace_dsc.h"
#include "utilc.h"
#include "xioc.h"

#if defined(X3270_FT) /*[*/
#include "ftc.h"
#endif /*]*/
#if defined(X3270_DISPLAY) /*[*/
#include "keypadc.h"
#include "menubarc.h"
#endif /*]*/
#if defined(X3270_DISPLAY) || defined(C3270) || defined(WC3270) /*[*/
#include "screenc.h"
#endif /*]*/

#if defined(X3270_DISPLAY) /*[*/
#include <X11/keysym.h>

#define MODMAP_SIZE	8
#define MAP_SIZE	13
#define MAX_MODS_PER	4
static struct {
        const char *name[MAX_MODS_PER];
        unsigned int mask;
	Boolean is_meta;
} skeymask[MAP_SIZE] = {
	{ { "Shift" }, ShiftMask, False },
	{ { (char *)NULL } /* Lock */, LockMask, False },
	{ { "Ctrl" }, ControlMask, False },
	{ { CN }, Mod1Mask, False },
	{ { CN }, Mod2Mask, False },
	{ { CN }, Mod3Mask, False },
	{ { CN }, Mod4Mask, False },
	{ { CN }, Mod5Mask, False },
	{ { "Button1" }, Button1Mask, False },
	{ { "Button2" }, Button2Mask, False },
	{ { "Button3" }, Button3Mask, False },
	{ { "Button4" }, Button4Mask, False },
	{ { "Button5" }, Button5Mask, False }
};
static Boolean know_mods = False;
#endif /*]*/

/* Actions that are aliases for other actions. */
/*
static char *aliased_actions[] = {
	"Close", "HardPrint", "Open", NULL
};
*/
enum iaction ia_cause;
const char *ia_name[] = {
	"String", "Paste", "Screen redraw", "Keypad", "Default", "Key",
	"Macro", "Script", "Peek", "Typeahead", "File transfer", "Command",
	"Keymap", "Idle"
};


/*
 * Return a name for an action.
 */
const char *
action_name(XtActionProc action)
{
	// TODO (perry#1#): Remove all calls to action_name; move all action processing to main program.
	return "Action";
}

#if defined(X3270_DISPLAY) /*[*/
/*
 * Search the modifier map to learn the modifier bits for Meta, Alt, Hyper and
 *  Super.
 */
static void
learn_modifiers(void)
{
	XModifierKeymap *mm;
	int i, j, k;
	static char *default_modname[] = {
	    CN, CN, "Ctrl",
	    "Mod1", "Mod2", "Mod3", "Mod4", "Mod5",
	    "Button1", "Button2", "Button3", "Button4", "Button5"
	};

	mm = XGetModifierMapping(display);

	for (i = 0; i < MODMAP_SIZE; i++) {
		for (j = 0; j < mm->max_keypermod; j++) {
			KeyCode kc;
			const char *name = CN;
			Boolean is_meta = False;

			kc = mm->modifiermap[(i * mm->max_keypermod) + j];
			if (!kc)
				continue;

			switch(XKeycodeToKeysym(display, kc, 0)) {
			    case XK_Meta_L:
			    case XK_Meta_R:
				name = "Meta";
				is_meta = True;
				break;
			    case XK_Alt_L:
			    case XK_Alt_R:
				name = "Alt";
				break;
			    case XK_Super_L:
			    case XK_Super_R:
				name = "Super";
				break;
			    case XK_Hyper_L:
			    case XK_Hyper_R:
				name = "Hyper";
				break;
			    default:
				break;
			}
			if (name == CN)
				continue;
			if (is_meta)
				skeymask[i].is_meta = True;

			for (k = 0; k < MAX_MODS_PER; k++) {
				if (skeymask[i].name[k] == CN)
					break;
				else if (!strcmp(skeymask[i].name[k], name))
					k = MAX_MODS_PER;
			}
			if (k >= MAX_MODS_PER)
				continue;
			skeymask[i].name[k] = name;
		}
	}
	for (i = 0; i < MODMAP_SIZE; i++) {
		if (skeymask[i].name[0] == CN) {
			skeymask[i].name[0] = default_modname[i];
		}
	}
	XFreeModifiermap(mm);
}

#if defined(X3270_TRACE) /*[*/
/*
 * Return the symbolic name for the modifier combination (i.e., "Meta" instead
 * of "Mod2".  Note that because it is possible to map multiple keysyms to the
 * same modifier bit, the answer may be ambiguous; we return the combinations
 * iteratively.
 */
static char *
key_symbolic_state(unsigned int state, int *iteration)
{
	static char rs[64];
	static int ix[MAP_SIZE];
	static int ix_ix[MAP_SIZE];
	static int n_ix = 0;
	static int leftover = 0;
	const char *comma = "";
	int i;

	if (!know_mods) {
		learn_modifiers();
		know_mods = True;
	}

	if (*iteration == 0) {
		/* First time, build the table. */
		n_ix = 0;
		for (i = 0; i < MAP_SIZE; i++) {
			if (skeymask[i].name[0] != CN &&
			    (state & skeymask[i].mask)) {
				ix[i] = 0;
				state &= ~skeymask[i].mask;
				ix_ix[n_ix++] = i;
			} else
				ix[i] = MAX_MODS_PER;
		}
		leftover = state;
	}

	/* Construct this result. */
	rs[0] = '\0';
	for (i = 0; i < n_ix;  i++) {
		(void) strcat(rs, comma);
		(void) strcat(rs, skeymask[ix_ix[i]].name[ix[ix_ix[i]]]);
		comma = " ";
	}
#if defined(VERBOSE_EVENTS) /*[*/
	if (leftover)
		(void) sprintf(strchr(rs, '\0'), "%s?%d", comma, state);
#endif /*]*/

	/*
	 * Iterate to the next.
	 * This involves treating each slot like an n-ary number, where n is
	 * the number of elements in the slot, iterating until the highest-
	 * ordered slot rolls back over to 0.
	 */
	if (n_ix) {
		i = n_ix - 1;
		ix[ix_ix[i]]++;
		while (i >= 0 &&
		       (ix[ix_ix[i]] >= MAX_MODS_PER ||
			skeymask[ix_ix[i]].name[ix[ix_ix[i]]] == CN)) {
			ix[ix_ix[i]] = 0;
			i = i - 1;
			if (i >= 0)
				ix[ix_ix[i]]++;
		}
		*iteration = i >= 0;
	} else
		*iteration = 0;

	return rs;
}
#endif /*]*/

/* Return whether or not an KeyPress event state includes the Meta key. */
Boolean
event_is_meta(int state)
{
	int i;

	/* Learn the modifier map. */
	if (!know_mods) {
		learn_modifiers();
		know_mods = True;
	}
	for (i = 0; i < MAP_SIZE; i++) {
		if (skeymask[i].name[0] != CN &&
		    skeymask[i].is_meta &&
		    (state & skeymask[i].mask)) {
			return True;
		}
	}
	return False;
}

#if defined(VERBOSE_EVENTS) /*[*/
static char *
key_state(unsigned int state)
{
	static char rs[64];
	const char *comma = "";
	static struct {
		const char *name;
		unsigned int mask;
	} keymask[] = {
		{ "Shift", ShiftMask },
		{ "Lock", LockMask },
		{ "Control", ControlMask },
		{ "Mod1", Mod1Mask },
		{ "Mod2", Mod2Mask },
		{ "Mod3", Mod3Mask },
		{ "Mod4", Mod4Mask },
		{ "Mod5", Mod5Mask },
		{ "Button1", Button1Mask },
		{ "Button2", Button2Mask },
		{ "Button3", Button3Mask },
		{ "Button4", Button4Mask },
		{ "Button5", Button5Mask },
		{ CN, 0 },
	};
	int i;

	rs[0] = '\0';
	for (i = 0; keymask[i].name; i++) {
		if (state & keymask[i].mask) {
			(void) strcat(rs, comma);
			(void) strcat(rs, keymask[i].name);
			comma = "|";
			state &= ~keymask[i].mask;
		}
	}
	if (!rs[0])
		(void) sprintf(rs, "%d", state);
	else if (state)
		(void) sprintf(strchr(rs, '\0'), "%s?%d", comma, state);
	return rs;
}
#endif /*]*/
#endif /*]*/

/*
 * Check the number of argument to an action, and possibly pop up a usage
 * message.
 *
 * Returns 0 if the argument count is correct, -1 otherwise.
 */

int
check_usage(XtActionProc action, Cardinal nargs, Cardinal nargs_min,
    Cardinal nargs_max)
{
	if (nargs >= nargs_min && nargs <= nargs_max)
		return 0;
	if (nargs_min == nargs_max)
		popup_an_error("Action requires %d argument%s",action, nargs_min, nargs_min == 1 ? "" : "s");
	else
		popup_an_error("Action requires %d or %d arguments",nargs_min, nargs_max);
//	cancel_if_idle_command();
	return -1;
}

/*
 * Display an action debug message
 */ /*
#if defined(X3270_TRACE)

#define KSBUF	256
void
action_debug(XtActionProc action, XEvent *event, String *params,
    Cardinal *num_params)
{
	Cardinal i;
	char pbuf[1024];
#if defined(X3270_DISPLAY)
	XKeyEvent *kevent;
	KeySym ks;
	XButtonEvent *bevent;
	XMotionEvent *mevent;
	XConfigureEvent *cevent;
	XClientMessageEvent *cmevent;
	XExposeEvent *exevent;
	const char *press = "Press";
	const char *direction = "Down";
	char dummystr[KSBUF+1];
	char *atom_name;
	int ambiguous = 0;
	int state;
	const char *symname = "";
	char snbuf[11];
#endif

	if (!toggled(EVENT_TRACE))
		return;
	if (event == (XEvent *)NULL) {
		trace_event(" %s", ia_name[(int)ia_cause]);
	}
#if defined(X3270_DISPLAY)
	else switch (event->type) {
	    case KeyRelease:
		press = "Release";
	    case KeyPress:
		kevent = (XKeyEvent *)event;
		(void) XLookupString(kevent, dummystr, KSBUF, &ks, NULL);
		state = kevent->state;
		//
		// If the keysym is a printable ASCII character, ignore the
		// Shift key.
		//
		if (ks != ' ' && !(ks & ~0xff) && isprint(ks))
			state &= ~ShiftMask;
		if (ks == NoSymbol)
			symname = "NoSymbol";
		else if ((symname = XKeysymToString(ks)) == CN) {
			(void) sprintf(snbuf, "0x%lx", (unsigned long)ks);
			symname = snbuf;
		}
		do {
			int was_ambiguous = ambiguous;

			trace_event("%s ':%s<Key%s>%s'",
				was_ambiguous? " or": "Event",
				key_symbolic_state(state, &ambiguous),
				press,
				symname);
		} while (ambiguous);
		//
		// If the keysym is an alphanumeric ASCII character, show the
		// case-insensitive alternative, sans the colon.
		//
		if (!(ks & ~0xff) && isalpha(ks)) {
			ambiguous = 0;
			do {
				int was_ambiguous = ambiguous;

				trace_event(" %s '%s<Key%s>%s'",
					was_ambiguous? "or":
					    "(case-insensitive:",
					key_symbolic_state(state, &ambiguous),
					press,
					symname);
			} while (ambiguous);
			trace_event(")");
		}
#if defined(VERBOSE_EVENTS)
		trace_event("\nKey%s [state %s, keycode %d, keysym "
			    "0x%lx \"%s\"]",
			    press, key_state(kevent->state),
			    kevent->keycode, ks,
			    symname);
#endif
		break;
	    case ButtonRelease:
		press = "Release";
		direction = "Up";
	    case ButtonPress:
		bevent = (XButtonEvent *)event;
		do {
			int was_ambiguous = ambiguous;

			trace_event("%s '%s<Btn%d%s>'",
				was_ambiguous? " or": "Event",
				key_symbolic_state(bevent->state, &ambiguous),
				bevent->button,
				direction);
		} while (ambiguous);
#if defined(VERBOSE_EVENTS)
		trace_event("\nButton%s [state %s, button %d]",
		    press, key_state(bevent->state),
		    bevent->button);
#endif
		break;
	    case MotionNotify:
		mevent = (XMotionEvent *)event;
		do {
			int was_ambiguous = ambiguous;

			trace_event("%s '%s<Motion>'",
				was_ambiguous? " or": "Event",
				key_symbolic_state(mevent->state, &ambiguous));
		} while (ambiguous);
#if defined(VERBOSE_EVENTS)
		trace_event("\nMotionNotify [state %s]",
			    key_state(mevent->state));
#endif
		break;
	    case EnterNotify:
		trace_event("EnterNotify");
		break;
	    case LeaveNotify:
		trace_event("LeaveNotify");
		break;
	    case FocusIn:
		trace_event("FocusIn");
		break;
	    case FocusOut:
		trace_event("FocusOut");
		break;
	    case KeymapNotify:
		trace_event("KeymapNotify");
		break;
	    case Expose:
		exevent = (XExposeEvent *)event;
		trace_event("Expose [%dx%d+%d+%d]",
		    exevent->width, exevent->height, exevent->x, exevent->y);
		break;
	    case PropertyNotify:
		trace_event("PropertyNotify");
		break;
	    case ClientMessage:
		cmevent = (XClientMessageEvent *)event;
		atom_name = XGetAtomName(display, (Atom)cmevent->data.l[0]);
		trace_event("ClientMessage [%s]",
		    (atom_name == CN) ? "(unknown)" : atom_name);
		break;
	    case ConfigureNotify:
		cevent = (XConfigureEvent *)event;
		trace_event("ConfigureNotify [%dx%d+%d+%d]",
		    cevent->width, cevent->height, cevent->x, cevent->y);
		break;
	    default:
		trace_event("Event %d", event->type);
		break;
	}
	if (keymap_trace != CN)
		trace_event(" via %s -> %s(", keymap_trace,
		    action_name(action));
	else
#endif
		trace_event(" -> Action %d(", (int) action);
	for (i = 0; i < *num_params; i++) {
		trace_event("%s\"%s\"",
		    i ? ", " : "",
		    scatv(params[i], pbuf, sizeof(pbuf)));
	}
	trace_event(")\n");

	trace_rollover_check();
}

#endif
*/

/*
 * Wrapper for calling an action internally.
 */
void
action_internal(XtActionProc action, enum iaction cause, const char *parm1,
    const char *parm2)
{
	Cardinal count = 0;
	String parms[2];

	/* Duplicate the parms, because XtActionProc doesn't grok 'const'. */
	if (parm1 != CN) {
		parms[0] = NewString(parm1);
		count++;
		if (parm2 != CN) {
			parms[1] = NewString(parm2);
			count++;
		}
	}

	ia_cause = cause;
	(*action)((Widget) NULL, (XEvent *) NULL,
	    count ? parms : (String *) NULL,
	    &count);

	/* Free the parm copies. */
	switch (count) {
	    case 2:
		Free(parms[1]);
		/* fall through... */
	    case 1:
		Free(parms[0]);
		break;
	    default:
		break;
	}
}


