
 #include <stdio.h>
 #include <gtk/gtk.h>
 #include "terminal.h"

/*---[ Constants ]------------------------------------------------------------*/

 static const char *hello = "hello.txt";

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

 static void initialize(Terminal *t)
 {
    FILE *arq;

    if(hello)
    {
    	arq = fopen(hello,"r");
    	if(arq)
    	{
			char buffer[90];
			int ln = 0;

			while(fgets(buffer,90,arq))
			{
				char *ptr;
				for(ptr=buffer;*ptr && *ptr >= ' ';ptr++);
				*ptr = 0;
				t->Print(ln,0,0,"%s",buffer);
				ln++;
			}

    		fclose(arq);
    	}
    }
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

    initialize(t);

    gtk_widget_show(top);

    gtk_main();

    delete t;

    return 0;
 }

 static void chkParameters(int argc, char *argv[])
 {
	// TODO (perry#3#): Testar parametros da linha de comando.

 }
