
 #include "g3270.h"
 #include <glib.h>

 #define __USE_XOPEN
 #include <poll.h>

 #include "lib/hostc.h"

 #define SRC_SIZE 3

// #ifdef DEBUG
//    #define LOCK DBGMessage("Lock"); 	g_mutex_lock(mutex);
//    #define UNLOCK DBGMessage("Unlock"); g_mutex_unlock(mutex);
// #else
    #define LOCK 	g_mutex_lock(mutex);
    #define UNLOCK 	g_mutex_unlock(mutex);
// #endif

/*---[ Structs ]--------------------------------------------------------------*/

#pragma pack(1)

 typedef struct _srcdata
 {
    GSource sr;
 } SRCDATA;

 typedef struct _pooldata
 {
 	GPollFD				gfd;		// MUST BE THE FIRST ONE!!!

 	const INPUT_3270	*ip;
 	int					events;

 } POOLDATA;

 #pragma pack()

/*---[ Prototipes ]-----------------------------------------------------------*/

#if GTK == 2

  // http://developer.gnome.org/doc/API/2.0/glib/glib-The-Main-Event-Loop.html#GSourceFuncs
  static gboolean prepare_3270(GSource *source, gint *timeout);
  static gboolean check_3270(GSource *source);
  static gboolean dispatch_3270(GSource *source, GSourceFunc callback, gpointer    user_data);
  static void     finalize_3270(GSource *source); /* Can be NULL */

  static gboolean closure_3270(gpointer data);
//  static void     DummyMarshal_3270(void); /* Really is of type GClosureMarshal */

#else

 static gboolean prepare_3270(gpointer  source_data, GTimeVal *current_time, gint *timeout, gpointer  user_data);
 static gboolean check_3270(gpointer  source_data, GTimeVal *current_time, gpointer  user_data);
 static gboolean dispatch_3270(gpointer  source_data, GTimeVal *current_time, gpointer  user_data);
 static void     destroy_3270(gpointer data);

#endif

/*---[ Constants ]------------------------------------------------------------*/

 /* 3270 Event Sources */
 static GSourceFuncs Source_3270 =
 {
#if GTK == 2

	prepare_3270,
	check_3270,
	dispatch_3270,
	finalize_3270,
	closure_3270,
	NULL

#else

	prepare_3270,
	check_3270,
	dispatch_3270,
	destroy_3270

#endif
 };

/*---[ Globals ]--------------------------------------------------------------*/

 static GSource		*fd3270			= 0;
 static GMutex		*mutex			= 0;
 static int			szPoll			= 0;
 static POOLDATA	*gpool			= 0;
 struct pollfd		*fds			= 0;
 static Boolean 	inputs_changed	= FALSE;


/*---[ Implement ]------------------------------------------------------------*/

 int gsource_init(void)
 {
    /* Add 3270 as a new gtk event source */

    mutex = g_mutex_new();

#if GTK == 2

    fd3270 = g_source_new(&Source_3270,sizeof(SRCDATA));
    g_source_attach(fd3270,NULL);

#else

	#error And what about GTK version 1?

#endif

 	return 0;
 }

 void gsource_addfile(const INPUT_3270 *ip)
 {
    // http://developer.gnome.org/doc/API/glib/glib-the-main-event-loop.html#G-MAIN-ADD-POLL
    POOLDATA 	*fd = 0;
    int			sz;
    int			f;

    LOCK

    if(!gpool)
    {
    	szPoll = SRC_SIZE;

    	sz     = szPoll * sizeof(POOLDATA);
    	gpool  = g_malloc(sz);

    	if(gpool)
    	{
			memset(gpool,0,sz);
    	}
		else
		{
			Log("Error allocating %d bytes for the GpoolFD structures",sz);
			return;
		}
    }

    for(f = 0; f < szPoll && !fd; f++)
    {
    	if(!gpool[f].ip)
    	   fd = gpool+f;
    }

    if(!fd)
    {
    	DBGPrintf("No more space in the poll table, adding more %d entries",SRC_SIZE);

    	sz     = (szPoll+SRC_SIZE) * sizeof(POOLDATA);

    	gpool  = g_realloc(gpool,sz);
    	if(!gpool)
    	{
			Log("Error resizing GpoolFD structures to %d bytes",sz);
			return;
    	}

    	for(f=0;f < SRC_SIZE;f++)
    	{
    		memset(gpool+(szPoll+f),0,sizeof(POOLDATA));
    	}

		fd = gpool+szPoll;

	    szPoll += SRC_SIZE;

    }

#ifdef DEBUG	// Sanity check
    if(fd->ip)
    {
    	DBGMessage("ERROR!!! The new poll structure has data!!!");
    }
#endif

    memset(fd,0,sizeof(POOLDATA));
    fd->ip     		= ip;
    fd->gfd.fd		= ip->source;

    if(ip->condition & InputReadMask)
    {
      fd->gfd.events |= (G_IO_IN|G_IO_PRI);
      fd->events      = (POLLIN|POLLRDNORM|POLLRDBAND|POLLPRI);
    }

    if(ip->condition & InputExceptMask)
    {
	  fd->gfd.events |= (G_IO_HUP|G_IO_NVAL|G_IO_ERR);
	  fd->events      = (POLLERR|POLLHUP|POLLNVAL);
    }

    if(ip->condition & InputWriteMask)
    {
	  fd->gfd.events |= G_IO_OUT;
	  fd->events      = (POLLOUT|POLLWRBAND);
    }

    inputs_changed	= TRUE;

    g_source_add_poll(fd3270,&fd->gfd);

    /* reset the poll() vector to the right size */
    sz = szPoll * sizeof(struct pollfd);

    if(fds)
       fds = g_realloc(fds,sz);
	else
	   fds = g_malloc(sz);

	memset(fds,0,sz);

    UNLOCK

	DBGPrintf("Source %p added (GPool=%p, fd=%d, masc=%02x)",ip,fd,ip->source,ip->condition);
 }

 void gsource_removefile(const INPUT_3270 *ip)
 {
    POOLDATA 	*fd = 0;
    int			f;

    LOCK

    for(f = 0; f < szPoll && !fd; f++)
    {
    	if(gpool[f].ip == ip)
    	   fd = gpool+f;
    }

    if(fd)
    {
		inputs_changed	= TRUE;
		g_source_remove_poll(fd3270,&fd->gfd);
    	memset(fd,0,sizeof(POOLDATA));
    }
    else
    {
    	Log("Unexpected call to gsource_removefile(%p) (fd=%d, masc=%02x)",ip,ip->source,ip->condition);
    }

    UNLOCK

	DBGPrintf("Source %p removed (GPool=%p, fd=%d, masc=%02x)",ip,fd,ip->source,ip->condition);

 }


#if GTK == 2

  // http://developer.gnome.org/doc/API/2.0/glib/glib-The-Main-Event-Loop.html#GSourceFuncs

  static gboolean prepare_3270(GSource *source, gint *timeout)
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
	struct timeval 	tv;
	long			ln;

    if(Check3270Timeouts(&tv))
    {
       // NOTE (perry#7#): Is there any better way?
       ln  = (tv.tv_sec * 1000000L);
       ln += tv.tv_usec;

       if(ln > 0)
          *timeout = (ln/1000);

    }

  	return 0;
  }

  static gboolean check_3270(GSource *source)
  {
    /*
     * Called after all the file descriptors are polled.
     * The source should return TRUE if it is ready to be dispatched.
     * Note that some time may have passed since the previous prepare
     * function was called, so the source should be checked again here.
     *
     */
	int f;
	int qtd = 0;
	int rc  = FALSE;

    LOCK

    for(f = 0; f < szPoll; f++)
    {
    	if(gpool[f].gfd.fd)
    	{
    		fds[qtd].fd     = gpool[f].gfd.fd;
    		fds[qtd].events = gpool[f].events;
    		qtd++;
    	}
    }

    if(poll(fds,qtd,0) > 0)
       rc = TRUE;

    DBGPrintf("Pending events: %s",rc ? "Yes" : "No");

    UNLOCK
  	return rc;
  }

  static gboolean dispatch_3270(GSource *source, GSourceFunc callback, gpointer user_data)
  {
    /*
     * Called to dispatch the event source,
     * after it has returned TRUE in either its prepare or its check function.
     * The dispatch function is passed in a callback function and data.
     * The callback function may be NULL if the source was never connected
     * to a callback using g_source_set_callback(). The dispatch function
     * should call the callback function with user_data and whatever additional
     * parameters are needed for this type of event source.
     *
     */
    struct pollfd		fds;
    gboolean			rc		= FALSE;
	int					f;
	const INPUT_3270	*ip;

    // FIXME (perry#1#): Do it right (using the callback function).

//    CHKPoint();
    for(f = 0; f < szPoll; f++)
    {
    	if(gpool[f].gfd.fd)
    	{
    		ip		   = gpool[f].ip;
    		fds.fd     = gpool[f].gfd.fd;
    		fds.events = gpool[f].events;

            if(poll(&fds,1,0) > 0)
            {
		       DBGPrintf("Source %p has event (fd=%d, masc=%02x)",ip,ip->source,ip->condition);
               (*ip->proc)();
               rc   = TRUE;
            }
        }
    }

    DBGTracex(rc);

  	return rc;
  }

  static void finalize_3270(GSource *source)
  {
    LOCK


    UNLOCK
  }

  static gboolean closure_3270(gpointer data)
  {
    LOCK


    UNLOCK
  	return 0;
  }

#else

 static gboolean prepare_3270(gpointer  source_data, GTimeVal *current_time, gint *timeout, gpointer  user_data)
 {
  	CHKPoint();
 }

 static gboolean check_3270(gpointer  source_data, GTimeVal *current_time, gpointer  user_data)
 {
  	CHKPoint();
 }

 static gboolean dispatch_3270(gpointer  source_data, GTimeVal *current_time, gpointer  user_data)
 {
  	CHKPoint();
 }

 static void destroy_3270(gpointer data)
 {
  	CHKPoint();
 }

#endif

