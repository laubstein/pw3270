
 #include "g3270.h"
 #include <gdk/gdkkeysyms.h>
 #include "lib/hostc.h"
 #include "lib/actionsc.h"
 #include "lib/kybdc.h"

/*---[ Structures ]-----------------------------------------------------------*/

 #pragma pack(1)

 struct user_config
 {
 	unsigned short	sz;
    gint 			width;
    gint			height;

 };

 #pragma pack()

/*---[ Main menu ]------------------------------------------------------------*/


/*---[ Globals ]--------------------------------------------------------------*/

 GtkWidget	*top_window		= 0;
 GtkWidget  *terminal		= 0;
 GtkWidget  *top_menu		= 0;
 GThread    *MainThread     = 0;

#if defined(DATADIR) && GTK == 2
 GdkPixbuf	*icon			= 0;
#endif

/*---[ Main program ]---------------------------------------------------------*/

 void action_exit(GtkWidget *w, gpointer data)
 {
 	char	filename[4096];
 	char	*home	= getenv("HOME");
 	FILE	*arq;

 	struct user_config config;

    DBGPrintf("Widget: %p Data: %d",w,(int) data);

 	action_disconnect(0,0);
 	Log("Exiting");

 	// Save window size and position
 	memset(&config,0,sizeof(config));
 	config.sz = sizeof(config);

    gtk_window_get_size(GTK_WINDOW(top_window),&config.width,&config.height);

    DBGPrintf("Tamanho da janela: %dx%d",config.width, config.height);

    snprintf(filename,4095,"%s/.%s.saved",home ? home : ".", TARGET);
    arq = fopen(filename,"w");
    if(arq)
    {
	   fwrite (&config, sizeof(config), 1, arq);
	   fclose(arq);
    }

 	gtk_main_quit();
 }

 static gboolean delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
 {
 	action_exit(widget,0);
    return FALSE;
 }

 gboolean map_event(GtkWidget *widget, GdkEvent  *event, gpointer   user_data)
 {
 	DBGTracex(widget->window);

    return 0;
 }

 static void destroy( GtkWidget *widget, gpointer   data )
 {
    gtk_main_quit();
 }

 static int GetFunctionKey(GdkEventKey *event)
 {
 	int rc = (event->keyval - GDK_F1)+1;

 	if(event->state & GDK_SHIFT_MASK)
 	   rc += 12;

	DBGPrintf("F%d %s",rc,event->type == GDK_KEY_PRESS ? "Pressed" : "Released");

	return rc;
 }

 static gboolean key_press_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {
 	// http://developer.gnome.org/doc/API/2.0/gtk/GtkWidget.html#GtkWidget-key-press-event
 	char ks[6];

    if(IS_FUNCTION_KEY(event))
    {
    	snprintf(ks,5,"%d",GetFunctionKey(event));
		action_internal(PF_action, IA_DEFAULT, ks, CN);
    	return TRUE;
    }

    return FALSE;
 }

 static gboolean key_release_event(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
 {

    if(IS_FUNCTION_KEY(event))
    {
    	GetFunctionKey(event);
    	return TRUE;
    }

 	return 0;
 }

 static void CreateMainWindow(const char *cl_hostname)
 {
 	GtkWidget	*vbox;
	GtkWidget   *toolbar;
 	char 		filename[4096];
 	char 		*home	= getenv("HOME");
 	FILE 		*arq;

 	struct user_config config;

    top_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

#if GTK == 1
	gtk_window_set_wmclass(GTK_WINDOW(top_window),"toplevel",TARGET);
	#error Is it working?
#else
    gtk_window_set_role(GTK_WINDOW(top_window), TARGET "0" );
#endif

	g_signal_connect(G_OBJECT(top_window),	"delete_event", 		G_CALLBACK(delete_event),			NULL);
    g_signal_connect(G_OBJECT(top_window),	"destroy", 				G_CALLBACK(destroy),				NULL);
    g_signal_connect(G_OBJECT(top_window),	"map-event",			G_CALLBACK(map_event),				NULL);
    g_signal_connect(G_OBJECT(top_window),	"key-press-event",		G_CALLBACK(key_press_event),	0);
    g_signal_connect(G_OBJECT(top_window), 	"key-release-event",	G_CALLBACK(key_release_event),	0);

    UpdateWindowTitle();

    vbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(top_window),vbox);

	// Load menu bar
    top_menu = LoadMenu(top_window);
    if(top_menu)
       gtk_box_pack_start(GTK_BOX(vbox), top_menu, FALSE, TRUE, 0);

	// Load toolbar
	toolbar = LoadToolbar(top_window);
    if(toolbar)
       gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);
	
    // Create terminal window
	terminal = g3270_new(cl_hostname);
	gtk_box_pack_start(GTK_BOX(vbox),terminal,TRUE,TRUE,0);

#if defined(DATADIR) && GTK == 2
    icon = gdk_pixbuf_new_from_file(DATADIR "/icon.jpg", NULL);
    if(icon)
    	gtk_window_set_icon(GTK_WINDOW(top_window),icon);
#endif

    // Set size and position
    snprintf(filename,4095,"%s/.%s.saved",home ? home : ".", TARGET);
    arq = fopen(filename,"r");
    if(arq)
    {
    	if(fread(&config,sizeof(config),1,arq) == 1 && (config.sz == sizeof(config)))
    	{
		   DBGPrintf("Tamanho da janela: %dx%d",config.width, config.height);
           gtk_window_resize(GTK_WINDOW(top_window),config.width, config.height);
    	}
    	fclose(arq);
    }

	gtk_widget_grab_focus(terminal);
	gtk_widget_grab_default(terminal);
	gtk_window_set_position(GTK_WINDOW(top_window),GTK_WIN_POS_CENTER);

 }

 int main(int argc, char **argv)
 {

    printf(TARGET " Build " BUILD " for gtk " GTKVERSION "\n");
    fflush(stdout);

    Log(TARGET " Build " BUILD " for gtk " GTKVERSION "\n");

    /* Populate callback tables */
    set_3270_screen(&g3270_screen_callbacks);

	// FIXME (perry#9#): Looks like this is not necessary since the library doesn't process keyboard.
    set_3270_keyboard(&g3270_keyboard_info);

    g_thread_init(NULL);
    gdk_threads_init();
    gtk_init(&argc, &argv);

    MainThread = g_thread_self();
    DBGPrintf("Main thread: %p",MainThread);

    /* Parse 3270 command line */
    parse_3270_command_line(argc, (const char **) argv, &cl_hostname);

    if(!cl_hostname)
       cl_hostname = getenv("HOST3270_0");

    DBGMessage(cl_hostname);

    CreateMainWindow(cl_hostname);
	
	if(terminal)
	{
		
#ifdef EXTENSIONS
	   LoadExtensions(EXTENSIONS);
	   SetExtensionsChar("g3270ServerChanged",cl_hostname);
#endif	
		
       DBGMessage("Starting gtk main loop");
       gtk_widget_show_all(top_window);
       gtk_main();
		
#ifdef EXTENSIONS
	   UnloadExtensions();
#endif	
		
	}

    return 0;
 }

 void UpdateWindowTitle(void)
 {
    char title[512];
    const char *ptr = query_qualified_host();

    if(top_window)
    {
       strncpy(title,TARGET,511);
       strncat(title," ",511);

       if(ptr)
       {
          strncat(title,ptr,511);
          strncat(title," ",511);
       }
       else if(cl_hostname)
       {
          strncat(title,cl_hostname,511);
          strncat(title," ",511);
       }

       LockThreads();
       gtk_window_set_title(GTK_WINDOW(top_window),title);
       UnlockThreads();

    }
 }

 void gdk_lock(void)
 {
 	// Calls from mainthread are builto from the internal gtk_main_loop and
 	// they're already locked.
 	if(g_thread_self() == MainThread)
 	   return;

    gdk_threads_enter();
 }

 void gdk_unlock(void)
 {
 	// Calls from mainthread are builto from the internal gtk_main_loop and
 	// they're already locked.
 	if(g_thread_self() == MainThread)
 	   return;

    gdk_threads_leave();
 }
