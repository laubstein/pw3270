/*
 * "Software G3270, desenvolvido com base nos códigos fontes do WC3270  e  X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe.
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
 * Este programa está nomeado como iocallback.c e possui 327 linhas de código.
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

#if defined(_WIN32) /*[*/
	#include <windows.h>
	#include "winversc.h"
	#include "windirsc.h"
#else
	#include <poll.h>
	#include <malloc.h>
	#include <string.h>
#endif /*]*/

#include <stdio.h>
#include <glib.h>
#include "g3270.h"

#ifdef G_THREADS_ENABLED
	static int g3270_CallAndWait(int(*callback)(void *), void *parm);
#endif

static unsigned long	g3270_AddInput(int source, void (*fn)(void));
static void			g3270_RemoveSource(unsigned long id);

#if !defined(_WIN32) /*[*/
static unsigned long	g3270_AddOutput(int source, void (*fn)(void));
#endif

static unsigned long	g3270_AddExcept(int source, void (*fn)(void));
static unsigned long	g3270_AddTimeOut(unsigned long interval_ms, void (*proc)(void));
static void 			g3270_RemoveTimeOut(unsigned long timer);
static int				g3270_Sleep(int seconds);
static int 			g3270_RunPendingEvents(int wait);

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
	g3270_RemoveSource,

	g3270_AddExcept,

#if !defined(_WIN32) /*[*/
	g3270_AddOutput,
#endif /*]*/

#ifdef G_THREADS_ENABLED
	g3270_CallAndWait,
#else
	NULL,	// int (*CallAndWait)(int(*callback)(void *), void *parm);
#endif

	g3270_Sleep,
	g3270_RunPendingEvents,

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

static unsigned long AddSource(int source, gushort events, void (*fn)(void))
{
	IO_Source *src = (IO_Source *) g_source_new(&IOSources,sizeof(IO_Source));

	src->source			= source;
	src->fn				= fn;
	src->poll.fd		= source;
	src->poll.events	= events;

	g_source_attach((GSource *) src,NULL);
	g_source_add_poll((GSource *) src,&src->poll);

	return (unsigned long) src;
}

static unsigned long g3270_AddInput(int source, void (*fn)(void))
{
	Trace("Adding Input handle %08x",source);
	return AddSource(source,G_IO_IN|G_IO_HUP|G_IO_ERR,fn);
}

static void g3270_RemoveSource(unsigned long id)
{
	if(id)
	{
		Trace("Removing input %p",(void *) id);
		g_source_destroy((GSource *) id);
	}
}

#if !defined(_WIN32) /*[*/
static unsigned long g3270_AddOutput(int source, void (*fn)(void))
{
	Trace("Adding Output handle %08x",source);
	return AddSource(source,G_IO_OUT|G_IO_HUP|G_IO_ERR,fn);
}
#endif /*]*/

static unsigned long g3270_AddExcept(int source, void (*fn)(void))
{
#if defined(_WIN32) /*[*/
	return 0;
#else
	Trace("Adding Except handle %08x",source);
	return AddSource(source,G_IO_HUP|G_IO_ERR,fn);
#endif
}

static gboolean do_timer(TIMER *t)
{
	if(!t->remove)
		t->fn();
	return FALSE;
}

static unsigned long g3270_AddTimeOut(unsigned long interval, void (*proc)(void))
{
	TIMER *t = g_malloc(sizeof(TIMER));

	t->remove = 0;
	t->fn = proc;

	g_timeout_add_full(G_PRIORITY_DEFAULT, (guint) interval, (GSourceFunc) do_timer, t, g_free);

	Trace("Timeout with %ld ms created with id %p",interval,t);

	return (unsigned long) t;
}

static void g3270_RemoveTimeOut(unsigned long timer)
{
	// FIXME (perry#2#): It this really necessary? The timeout is removed as soon as it ticks.
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
#if defined(_WIN32) /*[*/

	if(WaitForSingleObject((HANDLE) ((IO_Source *) source)->source,0) == WAIT_OBJECT_0)
		return TRUE;

#else /*][*/

	struct pollfd fds;

	memset(&fds,0,sizeof(fds));

	fds.fd     = ((IO_Source *) source)->poll.fd;
    fds.events = ((IO_Source *) source)->poll.events;

	if(poll(&fds,1,0) > 0)
		return TRUE;

#endif /*]*/

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

#ifdef G_THREADS_ENABLED

struct bgParameter
{
	int			status;
	int			rc;
	int(*callback)(void *);
	void		*parm;

};

gpointer BgCall(struct bgParameter *p)
{
	p->rc = p->callback(p->parm);
	p->status = 2;
	return 0;
}

static gboolean BgCallTimer(struct bgParameter *p)
{
	if(p->status == 2)
	{
		p->status = 0;
		return FALSE;
	}

	// TODO (perry#9#): Update some indicator in screen to inform the user of the application's "busy" state.

	return TRUE;
}

static int g3270_CallAndWait(int(*callback)(void *), void *parm)
{
	struct bgParameter p = { 3, -1, callback, parm };
	GThread	*thread;

	Trace("Starting auxiliary thread for callback %p",callback);

    thread =  g_thread_create( (GThreadFunc) BgCall, &p, 0, NULL);

    if(!thread)
    {
    	g_error("Can't start background thread");
    	return -1;
    }

	g_timeout_add((guint) 100, (GSourceFunc) BgCallTimer, &p);

	while(p.status)
		gtk_main_iteration();

	Trace("Auxiliary thread for callback %p finished (rc=%d)",callback,p.rc);

    return p.rc;
}
#endif

static int g3270_Sleep(int seconds)
{
	time_t end = time(0) + seconds;

	while(time(0) < end)
		gtk_main_iteration();

	return 0;
}

static int g3270_RunPendingEvents(int wait)
{
	int rc = 0;
	while(gtk_events_pending())
	{
		rc = 1;
		gtk_main_iteration();
	}

	if(wait)
		gtk_main_iteration();

	return rc;
}
