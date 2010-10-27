/*
 * Copyright 1995, 1999, 2000 by Paul Mattes.
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
 *	xioc.h
 *		Global declarations for xio.c.
 */

LIB3270_INTERNAL void Quit_action(Widget w, XEvent *event, String *params, Cardinal *num_params) __attribute__ ((deprecated));

LIB3270_INTERNAL void x3270_exit(int n) __attribute__ ((deprecated));
LIB3270_INTERNAL void x_add_input(int net_sock);
LIB3270_INTERNAL void x_except_off(void);
LIB3270_INTERNAL void x_except_on(int net_sock);
LIB3270_INTERNAL void x_remove_input(void);

