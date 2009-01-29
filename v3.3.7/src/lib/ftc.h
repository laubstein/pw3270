/*
 * Modifications Copyright 1996, 1999, 2000, 2001, 2002, 2003 by Paul Mattes.
 * Copyright October 1995 by Dick Altenbern
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
 *	ftc.h
 *		Global declarations for ft.c.
 */

#if defined(X3270_FT) /*[*/

extern Boolean ascii_flag;
extern Boolean cr_flag;
extern unsigned long ft_length;
extern FILE *ft_local_file;
extern char *ft_local_filename;

extern Boolean ft_last_cr;
extern Boolean remap_flag;

extern void ft_aborting(void);
extern void ft_complete(const char *errmsg);
// extern void ft_init(void);
extern void ft_running(Boolean is_cut);
extern void ft_update_length(void);

/*
extern void PA_dialog_focus_action(Widget w, XEvent *event, String *parms,
    Cardinal *num_parms);
extern void PA_dialog_next_action(Widget w, XEvent *event, String *parms,
    Cardinal *num_parms);
extern void popup_ft(Widget w, XtPointer call_parms, XtPointer call_data);
extern void Transfer_action(Widget w, XEvent *event, String *parms,
    Cardinal *num_parms);
*/

#endif /*]*/
