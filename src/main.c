
 #include <stdio.h>
 #include <gtk/gtk.h>

 #include "g3270.h"

/*---[ Prototipes ]-----------------------------------------------------------*/

 static void chkParameters(int argc, const char **argv);
 static void stsConnect(Boolean ignored);
 static void stsHalfConnect(Boolean ignored);
 static void stsExiting(Boolean ignored);


 static gboolean prepare_3270(gpointer  source_data, GTimeVal *current_time, gint *timeout, gpointer  user_data);
 static gboolean check_3270(gpointer  source_data, GTimeVal *current_time, gpointer  user_data);
 static gboolean dispatch_3270(gpointer  source_data, GTimeVal *current_time, gpointer  user_data);
 static GDestroyNotify destroy_3270;

/*---[ Constants ]------------------------------------------------------------*/

/* 3270 Event Sources */
static struct GSourceFuncs Source_3270;


/*---[ Implement ]------------------------------------------------------------*/
/*
 static gboolean delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
 {
    return FALSE;
 }

 static void destroy( GtkWidget *widget, gpointer   data )
 {
    gtk_main_quit();
 }
*/

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
 static gboolean prepare_3270(gpointer  source_data, GTimeVal *current_time, gint *timeout, gpointer  user_data)
 {
 }

 static gboolean check_3270(gpointer  source_data, GTimeVal *current_time, gpointer  user_data)
 {
 }

 static gboolean dispatch_3270(gpointer  source_data, GTimeVal *current_time, gpointer  user_data)
 {
 }

 static GDestroyNotify destroy_3270
 {
 }


 int main(int argc, const char **argv)
 {
 	const char *cl_hostname;
/*
    GtkWidget *top;
    Terminal  *t;
*/

    chkParameters(argc,argv);

    printf(TARGET " (Build " BUILD " for gtk " GTKVERSION ") Starting\n");
    fflush(stdout);

    set_3270_screen(&g3270_screen_callbacks);
    set_3270_keyboard(&g3270_keyboard_info);

/*
    gtk_init(&argc, &argv);
*/

    parse_3270_command_line(argc, argv, &cl_hostname);

    if(!cl_hostname)
       cl_hostname = "3270.df.bb:8023";

    g3270_log(TARGET, "Host: \"%s\"",cl_hostname);

    Initialize_3270();

    CHKPoint();
    register_3270_schange(ST_CONNECT,		stsConnect);
    register_3270_schange(ST_EXITING,		stsExiting);
    register_3270_schange(ST_HALF_CONNECT,	stsHalfConnect);
    register_3270_schange(ST_RESOLVING,		stsResolving);
    CHKPoint();

    Run_3270(cl_hostname);

/*
    top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(top), "delete_event", G_CALLBACK(delete_event), NULL);
    g_signal_connect (G_OBJECT(top), "destroy", G_CALLBACK (destroy), NULL);

    t = new Terminal();

    t->SetContainer(GTK_CONTAINER(top));

    gtk_widget_show(top);

    gtk_main();

    delete t;
*/

    return 0;
 }

 static void chkParameters(int argc, const char **argv)
 {
	// TODO (perry#3#): Testar parametros da linha de comando.

 }


