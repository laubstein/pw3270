
 #include "g3270.h"
 #include "lib/hostc.h"

/*---[ Prototipes ]-----------------------------------------------------------*/

 static void stsConnect(Boolean ignored);
 static void stsHalfConnect(Boolean ignored);
 static void stsExiting(Boolean ignored);

/*---[ Globals ]--------------------------------------------------------------*/

 const char	*cl_hostname	= 0;
 GtkWidget	*top_window		= 0;


/*---[ 3270 Event Status ]----------------------------------------------------*/

 void SetWindowTitle(const char *msg)
 {
 	char title[512];

    if(top_window)
    {
	   strncpy(title,TARGET,511);
	   strncat(title," ",511);

	   if(cl_hostname)
	   {
	      strncat(title,cl_hostname,511);
	      strncat(title," ",511);
	   }

       if(msg)
	      strncat(title,msg,511);

       gtk_window_set_title(GTK_WINDOW(top_window),title);

    }
}

 static void stsConnect(Boolean status)
 {
    g3270_log("lib3270", "%s", status ? "Connected" : "Disconnected");
    if(!status)
       SetWindowTitle(0);
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

    printf(TARGET " (Build " BUILD " for gtk " GTKVERSION ") Starting\n");
    fflush(stdout);

    /* Populate callback tables */
    set_3270_screen(&g3270_screen_callbacks);
    set_3270_keyboard(&g3270_keyboard_info);

    g_thread_init(NULL);
    gtk_init(&argc, &argv);

    /* Create gtk's stuff  */

    top_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(top_window), "delete_event", G_CALLBACK(delete_event), NULL);
    g_signal_connect (G_OBJECT(top_window), "destroy", G_CALLBACK (destroy), NULL);

    gsource_init();
    SetWindowTitle(0);

    /* Parse 3270 command line */

    parse_3270_command_line(argc, (const char **) argv, &cl_hostname);

    if(!cl_hostname)
       cl_hostname = "3270.df.bb:8023";

    Initialize_3270();

    register_3270_schange(ST_CONNECT,		stsConnect);
    register_3270_schange(ST_EXITING,		stsExiting);
    register_3270_schange(ST_HALF_CONNECT,	stsHalfConnect);
    register_3270_schange(ST_RESOLVING,		stsResolving);

    /* Start 3270 function */

    if(cl_hostname)
    {
       g3270_log(TARGET, "Connecting to \"%s\"",cl_hostname);
       host_connect(cl_hostname);
    }


//    Run_3270(cl_hostname);

    DBGMessage("Starting gtk main loop");

    gtk_widget_show(top_window);

    gtk_main();

    return 0;
 }



