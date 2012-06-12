/*
 * Copyright 1995, 1999, 2000, 2001, 2002, 2003, 2005, 2006 by Paul Mattes.
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
 *	macrosc.h
 *		Global declarations for macros.c.
 */

/* macro definition */
struct macro_def {
	char			*name;
	char			**parents;
	char			*action;
	struct macro_def	*next;
};
LIB3270_INTERNAL struct macro_def *macro_defs;
LIB3270_INTERNAL Boolean macro_output;

LIB3270_INTERNAL void abort_script(void);
LIB3270_INTERNAL void Abort_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void AnsiText_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void AsciiField_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void Ascii_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
#if defined(X3270_SCRIPT) /*[*/
LIB3270_INTERNAL void cancel_if_idle_command(void);
#else /*][*/
#define cancel_if_idle_command()
#endif /*]*/
LIB3270_INTERNAL void Bell_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void CloseScript_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void ContinueScript_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void EbcdicField_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void Ebcdic_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void Execute_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void execute_action_option(Widget w, XtPointer client_data,
    XtPointer call_data);
LIB3270_INTERNAL void Expect_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
#if defined(X3270_SCRIPT) && defined(X3270_PLUGIN) /*[*/
LIB3270_INTERNAL void plugin_aid(unsigned char aid);
#else /*][*/
#define plugin_aid(a)
#endif /*]*/
#if defined(X3270_SCRIPT) /*[*/
LIB3270_INTERNAL void Plugin_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
#endif /*]*/
LIB3270_INTERNAL void login_macro(char *s);
LIB3270_INTERNAL void macros_init(void);
LIB3270_INTERNAL void Macro_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void macro_command(struct macro_def *m);
LIB3270_INTERNAL void PauseScript_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void peer_script_init(void);
LIB3270_INTERNAL void ps_set(char *s, Boolean is_hex);
LIB3270_INTERNAL void Printer_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void push_command(char *);
LIB3270_INTERNAL void push_idle(char *);
LIB3270_INTERNAL void push_keymap_action(char *);
LIB3270_INTERNAL void push_macro(char *, Boolean);
LIB3270_INTERNAL void Query_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void ReadBuffer_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
LIB3270_INTERNAL void Script_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
#if defined(X3270_SCRIPT) /*[*/
LIB3270_INTERNAL void sms_accumulate_time(struct timeval *, struct timeval *);
#else /*][*/
#define sms_accumulate_time(a, b)
#endif /*]*/
LIB3270_INTERNAL Boolean sms_active(void);
LIB3270_INTERNAL void sms_connect_wait(void);
LIB3270_INTERNAL void sms_continue(void);
LIB3270_INTERNAL void sms_error(const char *msg);
LIB3270_INTERNAL void sms_host_output(void);
LIB3270_INTERNAL void sms_info(const char *fmt, ...) printflike(1, 2);
LIB3270_INTERNAL void sms_init(void);
LIB3270_INTERNAL Boolean sms_in_macro(void);
LIB3270_INTERNAL Boolean sms_redirect(void);
LIB3270_INTERNAL void sms_store(unsigned char c);
#if defined(X3270_SCRIPT) || defined(TCL3270) /*[*/
LIB3270_INTERNAL void Snap_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
#endif /*]*/
#if defined(TCL3270) /*[*/
LIB3270_INTERNAL void Status_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
#endif /*]*/
LIB3270_INTERNAL void Wait_action(Widget w, XEvent *event, String *params,
    Cardinal *num_params);
