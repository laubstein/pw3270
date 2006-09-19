
 #include <gtk/gtk.h>
 #include "terminal.h"

/*---[ Prototipes ]-----------------------------------------------------------*/

 static void chkParameters(int argc, char *argv[]);


/*---[ Implement ]------------------------------------------------------------*/

 static gboolean delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
 {
    return FALSE;
 }

 static void destroy( GtkWidget *widget, gpointer   data )
 {
    gtk_main_quit();
 }

 int main(int argc, char *argv[])
 {
    GtkWidget *top;
    Terminal  *t;

    printf(TARGET " (Build " BUILD ") Starting\n");
    fflush(stdout);

    gtk_init(&argc, &argv);
    chkParameters(argc,argv);

    top = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(top), "delete_event", G_CALLBACK(delete_event), NULL);
    g_signal_connect (G_OBJECT(top), "destroy", G_CALLBACK (destroy), NULL);

    t = new Terminal();

    t->SetContainer(GTK_CONTAINER(top));

    gtk_widget_show(top);

    gtk_main();

    delete t;

    return 0;
 }

 static void chkParameters(int argc, char *argv[])
 {
	// TODO (perry#3#): Testar parametros da linha de comando.

 }
