
 #include "g3270.h"
 #include "lib/hostc.h"

/*---[ Structs ]--------------------------------------------------------------*/

 typedef struct _srcdata
 {
    GSource sr;


 } SRCDATA;


/*---[ Globals ]--------------------------------------------------------------*/

 GSource *fd3270 = 0;

/*---[ Prototipes ]-----------------------------------------------------------*/

 static void stsConnect(Boolean ignored);
 static void stsHalfConnect(Boolean ignored);
 static void stsExiting(Boolean ignored);

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
 static void     destroy_3270(gpointer data)
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

/*---[ 3270 Event Status ]----------------------------------------------------*/

 static void stsConnect(Boolean status)
 {
    g3270_log("lib3270", "%s", status ? "Connected" : "Disconnected");
 }

 static void stsHalfConnect(Boolean ignored)
 {
 	DBGPrintf("HalfConnect: %s", ignored ? "Yes" : "No");
 }

 static void stsExiting(Boolean ignored)
 {
 	DBGPrintf("Exiting: %s", ignored ? "Yes" : "No");
 }

 static void stsResolving(Boolean ignored)
 {
 	DBGPrintf("Resolving: %s", ignored ? "Yes" : "No");
 }

/*---[ 3270 Event processing ]------------------------------------------------*/

#if GTK == 2

// http://developer.gnome.org/doc/API/2.0/glib/glib-The-Main-Event-Loop.html#GSourceFuncs

  static gboolean prepare_3270(GSource *source, gint *timeout)
  {
  	timeout = 0;
  	CHKPoint();
  	return 0;
  }

  static gboolean check_3270(GSource *source)
  {
  	CHKPoint();
  	return 0;
  }

  static gboolean dispatch_3270(GSource *source, GSourceFunc callback, gpointer    user_data)
  {
  	CHKPoint();
  	return 0;
  }

  static void finalize_3270(GSource *source)
  {
  	CHKPoint();
  }

  static gboolean closure_3270(gpointer data)
  {
  	CHKPoint();
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

/*---[ Main program ]---------------------------------------------------------*/

 static gboolean delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
 {
    return FALSE;
 }

 static void destroy( GtkWidget *widget, gpointer   data )
 {
    gtk_main_quit();
 }

 int main(int argc, char **argv)
 {
 	const char *cl_hostname;
    GtkWidget *top;

    printf(TARGET " (Build " BUILD " for gtk " GTKVERSION ") Starting\n");
    fflush(stdout);

    /* Populate callback tables */
    set_3270_screen(&g3270_screen_callbacks);
    set_3270_keyboard(&g3270_keyboard_info);

    gtk_init(&argc, &argv);

    parse_3270_command_line(argc, (const char **) argv, &cl_hostname);

    if(!cl_hostname)
       cl_hostname = "3270.df.bb:8023";

    Initialize_3270();

    CHKPoint();
    register_3270_schange(ST_CONNECT,		stsConnect);
    register_3270_schange(ST_EXITING,		stsExiting);
    register_3270_schange(ST_HALF_CONNECT,	stsHalfConnect);
    register_3270_schange(ST_RESOLVING,		stsResolving);
    CHKPoint();

    /* Add 3270 as a new gtk event source */

#if GTK == 2

    fd3270 = g_source_new(&Source_3270,sizeof(SRCDATA));
    g_source_attach(fd3270,NULL);

#else

	#error And what about GTK version 1?

#endif

    /* Create window and activate GTK */

    top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(top), "delete_event", G_CALLBACK(delete_event), NULL);
    g_signal_connect (G_OBJECT(top), "destroy", G_CALLBACK (destroy), NULL);

    /* Start 3270 function */
//    Run_3270(cl_hostname);

    if(cl_hostname)
    {
       g3270_log(TARGET, "Connecting to \"%s\"",cl_hostname);
       host_connect(cl_hostname);
    }


    gtk_widget_show(top);

    gtk_main();


    return 0;
 }



