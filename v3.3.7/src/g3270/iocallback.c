/*
 * Modifications and original code Copyright 1993, 1994, 1995, 1996,
 *    2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008 by Paul Mattes.
 * Original X11 Port Copyright 1990 by Jeff Sparkes.
 *   Permission to use, copy, modify, and distribute this software and its
 *   documentation for any purpose and without fee is hereby granted,
 *   provided that the above copyright notice appear in all copies and that
 *   both that copyright notice and this permission notice appear in
 *   supporting documentation.
 *
 * Copyright 1989 by Georgia Tech Research Corporation, Atlanta, GA 30332.
 *   All Rights Reserved.  GTRC hereby grants public use of this software.
 *   Derivative works based on this software must incorporate this copyright
 *   notice.
 *
 * c3270 and wc3270 are distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the file LICENSE for more details.
 */

/*
 *	iocallback.c
 *		I/O callbacks
 *		http://library.gnome.org/devel/glib/unstable/glib-The-Main-Event-Loop.html
 */

#if defined(_WIN32) /*[*/
#include <windows.h>
#include "winversc.h"
#include "windirsc.h"
#endif /*]*/

#include <stdio.h>
#include <glib.h>
#include "g3270.h"

static unsigned long	g3270_AddInput(int source, void (*fn)(void));
static void			g3270_RemoveInput(unsigned long id);

#if !defined(_WIN32) /*[*/
static unsigned long	g3270_AddOutput(int source, void (*fn)(void));
#endif

static unsigned long	g3270_AddExcept(int source, void (*fn)(void));
static unsigned long	g3270_AddTimeOut(unsigned long interval_ms, void (*proc)(void));
static void 			g3270_RemoveTimeOut(unsigned long timer);

static gboolean 		IO_prepare(GSource *source, gint *timeout);
static gboolean 		IO_check(GSource *source);
static gboolean 		IO_dispatch(GSource *source, GSourceFunc callback, gpointer user_data);
static void 			IO_finalize(GSource *source); /* Can be NULL */
static gboolean		IO_closure(gpointer data);

/*---[ Structs ]-------------------------------------------------------------------------------------------*/

typedef struct _IO_Source
{
	GSource gsrc;
	GPollFD	poll;
	int		source;
	void	(*fn)(void);
} IO_Source;

typedef struct _timer
{
	unsigned char remove;
	void (*fn)(void);
} TIMER;

const struct lib3270_io_callbacks g3270_io_callbacks =
{
	sizeof(struct lib3270_io_callbacks),

	g3270_AddTimeOut,
	g3270_RemoveTimeOut,

	g3270_AddInput,
	g3270_RemoveInput,

	g3270_AddExcept,

#if !defined(_WIN32) /*[*/
	g3270_AddOutput
#endif /*]*/

};

 static GSourceFuncs IOSources =
 {
        IO_prepare,
        IO_check,
        IO_dispatch,
        IO_finalize,
        IO_closure,
        NULL
 };

/*---[ Implement ]-----------------------------------------------------------------------------------------*/

static unsigned long g3270_AddInput(int source, void (*fn)(void))
{
	IO_Source *src = (IO_Source *) g_source_new(&IOSources,sizeof(IO_Source));

	src->source			= source;
	src->fn				= fn;
	src->poll.fd		= source;
	src->poll.events	= G_IO_IN | G_IO_HUP | G_IO_ERR;

	g_source_attach((GSource *) src,NULL);
	g_source_add_poll((GSource *) src,&src->poll);

	Trace("Input %08x added in %p",source,src);

	return (unsigned long) src;
}

static void g3270_RemoveInput(unsigned long id)
{
	Trace("Input %p Removed",(void *) id);
	g_source_destroy((GSource *) id);
}

#if !defined(_WIN32) /*[*/
static unsigned long g3270_AddOutput(int source, void (*fn)(void))
{
	#error not implemented
}
#endif /*]*/

static unsigned long g3270_AddExcept(int source, void (*fn)(void))
{
#if defined(_WIN32) /*[*/
	return 0;
#else /*][*/
	#error Not Implemented
#endif /*]*/
}

static gboolean do_timer(TIMER *t)
{
	if(t->remove)
		return FALSE;
	t->fn();
	return TRUE;
}

static unsigned long g3270_AddTimeOut(unsigned long interval, void (*proc)(void))
{
	TIMER *t = g_malloc(sizeof(TIMER));

	t->remove = 0;
	t->fn = proc;

	g_timeout_add_full(G_PRIORITY_DEFAULT, (guint) interval, (GSourceFunc) do_timer, t, g_free);

	Trace("Timer with %ld ms created with id %p",t);

	return (unsigned long) t;
}

static void g3270_RemoveTimeOut(unsigned long timer)
{
	Trace("Removing timer %p",((TIMER *) timer));
	((TIMER *) timer)->remove++;
}

static gboolean IO_prepare(GSource *source, gint *timeout)
{
	/*
 	 * Called before all the file descriptors are polled.
	 * If the source can determine that it is ready here
	 * (without waiting for the results of the poll() call)
	 * it should return TRUE.
	 *
	 * It can also return a timeout_ value which should be the maximum
	 * timeout (in milliseconds) which should be passed to the poll() call.
	 * The actual timeout used will be -1 if all sources returned -1,
	 * or it will be the minimum of all the timeout_ values
	 * returned which were >= 0.
	 *
	 */

	return 0;
}

static gboolean IO_check(GSource *source)
{
	/*
 	 * Called after all the file descriptors are polled.
 	 * The source should return TRUE if it is ready to be dispatched.
	 * Note that some time may have passed since the previous prepare
	 * function was called, so the source should be checked again here.
	 *
	 */
	if(WaitForSingleObject((HANDLE) ((IO_Source *) source)->source,0) == WAIT_OBJECT_0)
		return TRUE;

	return FALSE;
}

static gboolean IO_dispatch(GSource *source, GSourceFunc callback, gpointer user_data)
{
	/*
	 * Called to dispatch the event source,
	 * after it has returned TRUE in either its prepare or its check function.
	 * The dispatch function is passed in a callback function and data.
	 * The callback function may be NULL if the source was never connected
	 * to a callback using g_source_set_callback(). The dispatch function
	 * should call the callback function with user_data and whatever additional
	 * parameters are needed for this type of event source.
	 */
	((IO_Source *) source)->fn();
	return TRUE;
}

static void IO_finalize(GSource *source)
{

}

static gboolean IO_closure(gpointer data)
{
	return 0;
}


