
 #include "g3270.h"

/*---[ Prototipes ]-----------------------------------------------------------*/

/*---[ Globals ]--------------------------------------------------------------*/

 GtkWidget	*top_window		= 0;
 GtkWidget  *terminal		= 0;
 GtkWidget  *StatusBar		= 0;
 GtkWidget	*CursorPosition	= 0;

/*---[ Main program ]---------------------------------------------------------*/

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


 static gboolean delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
 {
    return FALSE;
 }

 static void destroy( GtkWidget *widget, gpointer   data )
 {
    gtk_main_quit();
 }

 static void CreateMainWindow(const char *cl_hostname)
 {
 	GtkWidget *vbox;

    top_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(top_window),  "delete_event", G_CALLBACK(delete_event), NULL);
    g_signal_connect (G_OBJECT(top_window), "destroy", G_CALLBACK (destroy), NULL);

    SetWindowTitle(0);

    vbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(top_window),vbox);

    // Create terminal window
	terminal = g3270_new(cl_hostname);
	gtk_box_pack_start(GTK_BOX(vbox),terminal,TRUE,TRUE,0);

    // Create status-bar
    StatusBar = gtk_hbox_new(FALSE,1);
	gtk_box_pack_end(GTK_BOX(vbox),StatusBar,FALSE,FALSE,0);

    CursorPosition = gtk_label_new("--/---");
	gtk_box_pack_end(GTK_BOX(StatusBar),CursorPosition,FALSE,FALSE,10);

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

    /* Parse 3270 command line */
    parse_3270_command_line(argc, (const char **) argv, &cl_hostname);

    if(!cl_hostname)
       cl_hostname = "3270.df.bb:8023";

    CreateMainWindow(cl_hostname);

    DBGMessage("Starting gtk main loop");

    gtk_widget_show_all(top_window);
    gtk_main();

    return 0;
 }



