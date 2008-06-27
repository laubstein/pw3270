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
 */


#include "c3270.h"

static unsigned long	c3270_AddInput(int source, void (*fn)(void));
static void			c3270_RemoveInput(unsigned long id);

#if !defined(_WIN32) /*[*/
static unsigned long	c3270_AddOutput(int source, void (*fn)(void));
#endif

static unsigned long	c3270_AddExcept(int source, void (*fn)(void));
static unsigned long	c3270_AddTimeOut(unsigned long interval_ms, void (*proc)(void));
static void 			c3270_RemoveTimeOut(unsigned long timer);

const struct lib3270_io_callbacks c3270_callbacks =
{
	sizeof(struct lib3270_io_callbacks),

	c3270_AddTimeOut,
	c3270_RemoveTimeOut,

	c3270_AddInput,
	c3270_RemoveInput,

	c3270_AddExcept,

#if !defined(_WIN32) /*[*/
	c3270_AddOutput
#endif /*]*/

};



#define InputReadMask	0x1
#define InputExceptMask	0x2
#define InputWriteMask	0x4

/* Input events. */
typedef struct input
{
	struct input *next;
	int source;
	int condition;
	void (*proc)(void);
} input_t;

static input_t *inputs = (input_t *)NULL;
static Boolean inputs_changed = False;

static unsigned long c3270_AddInput(int source, void (*fn)(void))
{
	input_t *ip;

	ip = (input_t *)Malloc(sizeof(input_t));
	ip->source = source;
	ip->condition = InputReadMask;
	ip->proc = fn;
	ip->next = inputs;
	inputs = ip;
	inputs_changed = True;
	return (unsigned long)ip;
}

static void c3270_RemoveInput(unsigned long id)
{
	input_t *ip;
	input_t *prev = (input_t *)NULL;

	for (ip = inputs; ip != (input_t *)NULL; ip = ip->next) {
		if (ip == (input_t *)id)
			break;
		prev = ip;
	}
	if (ip == (input_t *)NULL)
		return;
	if (prev != (input_t *)NULL)
		prev->next = ip->next;
	else
		inputs = ip->next;
	Free(ip);
	inputs_changed = True;
}

#if !defined(_WIN32) /*[*/
static unsigned long c3270_AddOutput(int source, void (*fn)(void))
{
	input_t *ip;

	ip = (input_t *)Malloc(sizeof(input_t));
	ip->source = source;
	ip->condition = InputWriteMask;
	ip->proc = fn;
	ip->next = inputs;
	inputs = ip;
	inputs_changed = True;
	return (unsigned long)ip;
}
#endif /*]*/

static unsigned long c3270_AddExcept(int source, void (*fn)(void))
{
#if defined(_WIN32) /*[*/
	return 0;
#else /*][*/
	input_t *ip;

	ip = (input_t *)Malloc(sizeof(input_t));
	ip->source = source;
	ip->condition = InputExceptMask;
	ip->proc = fn;
	ip->next = inputs;
	inputs = ip;
	inputs_changed = True;
	return (unsigned long)ip;
#endif /*]*/
}

typedef struct timeout {
	struct timeout *next;
#if defined(_WIN32) /*[*/
	unsigned long long ts;
#else /*][*/
	struct timeval tv;
#endif /*]*/
	void (*proc)(void);
	Boolean in_play;
} timeout_t;
#define TN	(timeout_t *)NULL
static timeout_t *timeouts = TN;

#if defined(_WIN32) /*[*/
static void ms_ts(unsigned long long *u)
{
	FILETIME t;

	/* Get the system time, in 100ns units. */
	GetSystemTimeAsFileTime(&t);
	memcpy(u, &t, sizeof(unsigned long long));

	/* Divide by 10 to get ms. */
	*u /= 10ULL;
}
#endif /*]*/

static unsigned long c3270_AddTimeOut(unsigned long interval_ms, void (*proc)(void))
{
	timeout_t *t_new;
	timeout_t *t;
	timeout_t *prev = TN;

	t_new = (timeout_t *)Malloc(sizeof(timeout_t));
	t_new->proc = proc;
	t_new->in_play = False;
#if defined(_WIN32) /*[*/
	ms_ts(&t_new->ts);
	t_new->ts += interval_ms;
#else /*][*/
	(void) gettimeofday(&t_new->tv, NULL);
	t_new->tv.tv_sec += interval_ms / 1000L;
	t_new->tv.tv_usec += (interval_ms % 1000L) * 1000L;
	if (t_new->tv.tv_usec > MILLION) {
		t_new->tv.tv_sec += t_new->tv.tv_usec / MILLION;
		t_new->tv.tv_usec %= MILLION;
	}
#endif /*]*/

	/* Find where to insert this item. */
	for (t = timeouts; t != TN; t = t->next) {
#if defined(_WIN32) /*[*/
		if (t->ts > t_new->ts)
#else /*][*/
		if (t->tv.tv_sec > t_new->tv.tv_sec ||
		    (t->tv.tv_sec == t_new->tv.tv_sec &&
		     t->tv.tv_usec > t_new->tv.tv_usec))
#endif /*]*/
			break;
		prev = t;
	}

	/* Insert it. */
	if (prev == TN) {	/* Front. */
		t_new->next = timeouts;
		timeouts = t_new;
	} else if (t == TN) {	/* Rear. */
		t_new->next = TN;
		prev->next = t_new;
	} else {				/* Middle. */
		t_new->next = t;
		prev->next = t_new;
	}

	return (unsigned long)t_new;
}

static void c3270_RemoveTimeOut(unsigned long timer)
{
	timeout_t *st = (timeout_t *)timer;
	timeout_t *t;
	timeout_t *prev = TN;

	if (st->in_play)
		return;
	for (t = timeouts; t != TN; t = t->next) {
		if (t == st) {
			if (prev != TN)
				prev->next = t->next;
			else
				timeouts = t->next;
			Free(t);
			return;
		}
		prev = t;
	}
}

#if defined(_WIN32) /*[*/
#define MAX_HA	256
#endif /*]*/

/* Event dispatcher. */
Boolean process_events(Boolean block)
{
#if defined(_WIN32) /*[*/
	HANDLE ha[MAX_HA];
	DWORD nha;
	DWORD tmo;
	DWORD ret;
	unsigned long long now;
	int i;
#else /*][*/
	fd_set rfds, wfds, xfds;
	int ns;
	struct timeval now, twait, *tp;
#endif /*]*/
	input_t *ip, *ip_next;
	struct timeout *t;
	Boolean any_events;
	Boolean processed_any = False;

	processed_any = False;
    retry:
	/* If we've processed any input, then don't block again. */
	if (processed_any)
		block = False;
	any_events = False;
#if defined(_WIN32) /*[*/
	nha = 0;
#else /*][*/
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&xfds);
#endif /*]*/
	for (ip = inputs; ip != (input_t *)NULL; ip = ip->next) {
		if ((unsigned long)ip->condition & InputReadMask) {
#if defined(_WIN32) /*[*/
			ha[nha++] = (HANDLE)ip->source;
#else /*][*/
			FD_SET(ip->source, &rfds);
#endif /*]*/
			any_events = True;
		}
#if !defined(_WIN32) /*[*/
		if ((unsigned long)ip->condition & InputWriteMask) {
			FD_SET(ip->source, &wfds);
			any_events = True;
		}
		if ((unsigned long)ip->condition & InputExceptMask) {
			FD_SET(ip->source, &xfds);
			any_events = True;
		}
#endif /*]*/
	}
	if (block) {
		if (timeouts != TN) {
#if defined(_WIN32) /*[*/
			ms_ts(&now);
			if (now > timeouts->ts)
				tmo = 0;
			else
				tmo = timeouts->ts - now;
#else /*][*/
			(void) gettimeofday(&now, (void *)NULL);
			twait.tv_sec = timeouts->tv.tv_sec - now.tv_sec;
			twait.tv_usec = timeouts->tv.tv_usec - now.tv_usec;
			if (twait.tv_usec < 0L) {
				twait.tv_sec--;
				twait.tv_usec += MILLION;
			}
			if (twait.tv_sec < 0L)
				twait.tv_sec = twait.tv_usec = 0L;
			tp = &twait;
#endif /*]*/
			any_events = True;
		} else {
#if defined(_WIN32) /*[*/
			tmo = INFINITE;
#else /*][*/
			tp = (struct timeval *)NULL;
#endif /*]*/
		}
	} else {
#if defined(_WIN32) /*[*/
		tmo = 1;
#else /*][*/
		twait.tv_sec = twait.tv_usec = 0L;
		tp = &twait;
#endif /*]*/
	}

	if (!any_events)
		return processed_any;
#if defined(_WIN32) /*[*/
	ret = WaitForMultipleObjects(nha, ha, FALSE, tmo);
	if (ret == WAIT_FAILED) {
#else /*][*/
	ns = select(FD_SETSIZE, &rfds, &wfds, &xfds, tp);
	if (ns < 0) {
		if (errno != EINTR)
			Warning("process_events: select() failed");
#endif /*]*/
		return processed_any;
	}
	inputs_changed = False;
#if defined(_WIN32) /*[*/
	for (i = 0, ip = inputs; ip != (input_t *)NULL; ip = ip_next, i++) {
#else /*][*/
	for (ip = inputs; ip != (input_t *)NULL; ip = ip_next) {
#endif /*]*/
		ip_next = ip->next;
		if (((unsigned long)ip->condition & InputReadMask) &&
#if defined(_WIN32) /*[*/
		    ret == WAIT_OBJECT_0 + i) {
#else /*][*/
		    FD_ISSET(ip->source, &rfds)) {
#endif /*]*/
			(*ip->proc)();
			processed_any = True;
			if (inputs_changed)
				goto retry;
		}
#if !defined(_WIN32) /*[*/
		if (((unsigned long)ip->condition & InputWriteMask) &&
		    FD_ISSET(ip->source, &wfds)) {
			(*ip->proc)();
			processed_any = True;
			if (inputs_changed)
				goto retry;
		}
		if (((unsigned long)ip->condition & InputExceptMask) &&
		    FD_ISSET(ip->source, &xfds)) {
			(*ip->proc)();
			processed_any = True;
			if (inputs_changed)
				goto retry;
		}
#endif /*]*/
	}

	/* See what's expired. */
	if (timeouts != TN) {
#if defined(_WIN32) /*[*/
		ms_ts(&now);
#else /*][*/
		(void) gettimeofday(&now, (void *)NULL);
#endif /*]*/
		while ((t = timeouts) != TN) {
#if defined(_WIN32) /*[*/
			if (t->ts <= now) {
#else /*][*/
			if (t->tv.tv_sec < now.tv_sec ||
			    (t->tv.tv_sec == now.tv_sec &&
			     t->tv.tv_usec < now.tv_usec)) {
#endif /*]*/
				timeouts = t->next;
				t->in_play = True;
				(*t->proc)();
				processed_any = True;
				Free(t);
			} else
				break;
		}
	}
	if (inputs_changed)
		goto retry;

	return processed_any;
}

