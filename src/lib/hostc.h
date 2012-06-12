/*
 * Copyright 1995, 1996, 1999, 2000, 2001, 2002, 2003, 2005 by Paul Mattes.
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
 *	hostc.h
 *		Global declarations for host.c.
 */

#include <lib3270/api.h>

	struct host {
		char *name;
		char **parents;
		char *hostname;
		enum { PRIMARY, ALIAS, RECENT } entry_type;
		char *loginstring;
		time_t connect_time;
		struct host *prev, *next;
	};
	extern struct host *hosts;

/*
	extern void Connect_action(Widget w, XEvent *event, String *params, Cardinal *num_params);
	extern void Disconnect_action(Widget w, XEvent *event, String *params, Cardinal *num_params);
*/

	LIB3270_INTERNAL void st_changed(int tx, int mode);
	LIB3270_INTERNAL void hostfile_init(void);
	LIB3270_INTERNAL void host_connected(void);
	LIB3270_INTERNAL void host_in3270(enum cstate);


