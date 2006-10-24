
 #include <stdio.h>
 #include <gtk/gtk.h>
 #include "terminal.h"

 #include "g3270.h"

/*---[ Constants ]------------------------------------------------------------*/

/*---[ Prototipes ]-----------------------------------------------------------*/

extern "C" {

 static void chkParameters(int argc, const char **argv);
 static void stsConnect(Boolean ignored);
 static void stsHalfConnect(Boolean ignored);
 static void stsExiting(Boolean ignored);

}

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
 	DBGPrintf("Connect: %s",status ? "Yes" : "No");
 }

 static void stsHalfConnect(Boolean ignored)
 {
 	DBGMessage("HalfConnect");
 }

 static void stsExiting(Boolean ignored)
 {
 	DBGMessage("Exiting");
 }

 static void stsResolving(Boolean ignored)
 {
 	DBGMessage("Resolving");
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

    printf("Server: %s\n",cl_hostname);

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


