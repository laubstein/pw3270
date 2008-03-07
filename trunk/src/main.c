
 #include "g3270.h"
 #include <gdk/gdkkeysyms.h>

 #include "lib/hostc.h"
 #include "lib/actionsc.h"
 #include "lib/kybdc.h"
 #include "lib/togglesc.h"

/*---[ Structures ]-----------------------------------------------------------*/

 #pragma pack(1)

 struct user_config
 {
 	unsigned short	sz;
    gint 			width;
    gint			height;

 };

 #pragma pack()

#ifdef __G_KEY_FILE_H__
	static const struct _WindowState
	{
		const char *name;
		GdkWindowState flag;
		void (*activate)(GtkWindow *);
	} WindowState[] =
	{
		{ "Maximized",	GDK_WINDOW_STATE_MAXIMIZED,		gtk_window_maximize		},
		{ "Iconified",	GDK_WINDOW_STATE_ICONIFIED,		gtk_window_iconify		},
		{ "Sticky",		GDK_WINDOW_STATE_STICKY,		gtk_window_stick		}
	};
#endif

/*---[ Constants ]------------------------------------------------------------*/

 const char *toggle_name[N_TOGGLES] =
 	{	"MONOCASE",
		"ALT_CURSOR",
		"CursorBlink",
		"SHOW_TIMING",
		"ShowCursorPosition",
		"DS_TRACE",
		"SCROLL_BAR",
		"LINE_WRAP",
		"BLANK_FILL",
		"SCREEN_TRACE",
		"EVENT_TRACE",
		"MarginedPaste",
		"RECTANGLE_SELECT",
		"CrossHairCursor",
		"VISIBLE_CONTROL",
		"AID_WAIT",
		"Reconnect",
		"FullScreen"
 	};

/*---[ Globals ]--------------------------------------------------------------*/

 GtkWidget	*top_window		= 0;
 GtkWidget  *terminal		= 0;
 GtkWidget  *top_menu		= 0;
 GThread    *MainThread     = 0;

#if defined(DATADIR) && GTK == 2
 GdkPixbuf	*icon			= 0;
#endif

#ifdef __G_KEY_FILE_H__
 GKeyFile	*main_configuration	= 0;
#endif

#ifdef USE_GNOME
 GnomeClient *client = 0;
#endif

/*---[ Main program ]---------------------------------------------------------*/

 void action_exit(GtkWidget *w, gpointer data)
 {
 	char	filename[4096];
 	char	*home	= getenv("HOME");
 	FILE	*arq;

#ifdef __G_KEY_FILE_H__
	int		pos[2];
	char	*ptr;
	char	*conf;
	int		f;

	GdkWindowState CurrentState;

#else
 	struct user_config config;
#endif

 	action_disconnect(0,0);
 	Log("Exiting");

#ifdef __G_KEY_FILE_H__

	DBGTracex(main_configuration);

	if(main_configuration)
	{
		snprintf(filename,4095,"%s/.%s.conf",home ? home : ".", PROJECT_NAME);

		if(top_window->window)
		{
			CurrentState = gdk_window_get_state(top_window->window);

		 	if( !(CurrentState & (GDK_WINDOW_STATE_FULLSCREEN|GDK_WINDOW_STATE_MAXIMIZED|GDK_WINDOW_STATE_ICONIFIED)) )
		 	{
				// Window isn't in fullscreen mode, save size
				DBGMessage("*** Saving window size");
				gtk_window_get_size(GTK_WINDOW(top_window),&pos[0],&pos[1]);
				g_key_file_set_integer_list(main_configuration,"MainWindow","size",pos,2);
		 	}

			for(f=0;f<(sizeof(WindowState)/sizeof(struct _WindowState));f++)
			{
				g_key_file_set_boolean(main_configuration,"MainWindow",WindowState[f].name, CurrentState & WindowState[f].flag);
			}
		}

		conf = ptr = g_key_file_to_data(main_configuration,NULL,NULL);
		if(ptr)
		{
			while(*ptr && isspace(*ptr))
				ptr++;
			arq = fopen(filename,"w");
			if(arq)
			{
				fprintf(arq,ptr);
				fclose(arq);
			}
			g_free(conf);
		}
		g_key_file_free(main_configuration);
		main_configuration = 0;
	}

#else
	if(top_window->window && !(gdk_window_get_state(top_window->window) & GDK_WINDOW_STATE_FULLSCREEN))
	{
		memset(&config,0,sizeof(config));
		config.sz = sizeof(config);

		gtk_window_get_size(GTK_WINDOW(top_window),&config.width,&config.height);

		DBGPrintf("Tamanho da janela: %dx%d",config.width, config.height);

		snprintf(filename,4095,"%s/.%s.saved",home ? home : ".", TARGET);
		arq = fopen(filename,"w");
		if(arq)
		{
		   fwrite (&config, sizeof(config), 1, arq);
		   SaveTerminalColors(arq);
		   fclose(arq);
		}
	}
#endif

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

 static gboolean window_state_event(GtkWidget *widget, GdkEventWindowState *event)
 {
 	return FALSE;
 }

 static void CreateMainWindow(const char *cl_hostname)
 {
 	GtkWidget	*vbox;
	GtkWidget   *toolbar;
 	char 		filename[4096];
 	char 		*home	= getenv("HOME");
 	FILE 		*arq	= 0;
#ifdef __G_KEY_FILE_H__
	gint		*pos;
	gsize		sz;
	int			f;
#else
 	struct user_config config;
#endif

    top_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	// Set default toogle values
	CHKPoint();
	set_toggle(MARGINED_PASTE,1,TT_INITIAL);
	set_toggle(CURSOR_POS,1,TT_INITIAL);

	DBGPrintf("toggle CURSOR_POS: %s",(int) toggled(CURSOR_POS) ? "Enabled" : "Disabled");


	// Load configuration
#ifdef __G_KEY_FILE_H__
	main_configuration = g_key_file_new();
	if(main_configuration)
	{
		// Load configuration
    	snprintf(filename,4095,"%s/.%s.conf",home ? home : ".", PROJECT_NAME);
    	g_key_file_load_from_file(main_configuration,filename,G_KEY_FILE_KEEP_TRANSLATIONS,NULL);

    	if(g_key_file_has_key(main_configuration,"MainWindow","size",NULL))
    	{
			pos = g_key_file_get_integer_list(main_configuration,"MainWindow","size",&sz,NULL);
			if(pos && sz == 2)
           		gtk_window_resize(GTK_WINDOW(top_window),pos[0],pos[1]);
    	}

		// Load window states
		for(f=0;f<(sizeof(WindowState)/sizeof(struct _WindowState));f++)
		{
			if(g_key_file_get_boolean(main_configuration,"MainWindow",WindowState[f].name,NULL))
				WindowState[f].activate(GTK_WINDOW(top_window));
		}


	}
#else
    snprintf(filename,4095,"%s/.%s.saved",home ? home : ".", TARGET);
    arq = fopen(filename,"r");
    if(arq)
    {
    	if(fread(&config,sizeof(config),1,arq) == 1 && (config.sz == sizeof(config)))
    	{
		   DBGPrintf("Tamanho da janela: %dx%d",config.width, config.height);
           gtk_window_resize(GTK_WINDOW(top_window),config.width, config.height);
    	}
    }
#endif

#if GTK == 1
	gtk_window_set_wmclass(GTK_WINDOW(top_window),"toplevel",PROJECT_NAME);
	#error Is it working?
#else
    gtk_window_set_role(GTK_WINDOW(top_window), PROJECT_NAME "0" );
#endif

	g_signal_connect(G_OBJECT(top_window),	"delete_event", 		G_CALLBACK(delete_event),			NULL);
    g_signal_connect(G_OBJECT(top_window),	"destroy", 				G_CALLBACK(destroy),				NULL);
    g_signal_connect(G_OBJECT(top_window),	"map-event",			G_CALLBACK(map_event),				NULL);
    g_signal_connect(G_OBJECT(top_window),	"key-press-event",		G_CALLBACK(key_press_event),		0);
    g_signal_connect(G_OBJECT(top_window), 	"key-release-event",	G_CALLBACK(key_release_event),		0);
    g_signal_connect(G_OBJECT(top_window), 	"window-state-event",	G_CALLBACK(window_state_event),		0);

    UpdateWindowTitle();

    vbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(top_window),vbox);

	// Load extensions
#ifdef EXTENSIONS
	LoadExtensions(EXTENSIONS);
#endif

	// Load menu bar
    top_menu = LoadMenu(top_window);
    if(top_menu)
       gtk_box_pack_start(GTK_BOX(vbox), top_menu, FALSE, TRUE, 0);

	// Load toolbar
	toolbar = LoadToolbar(top_window);
    if(toolbar)
       gtk_box_pack_start(GTK_BOX(vbox), toolbar, FALSE, TRUE, 0);

	// Preset toggles
	if(toggled(FULLSCREEN))
		gtk_window_fullscreen(GTK_WINDOW(top_window));
	else
		gtk_window_unfullscreen(GTK_WINDOW(top_window));


    /* Load colors */
	LoadTerminalColors(arq);

    // Create terminal window
	terminal = g3270_new(cl_hostname);
	gtk_box_pack_start(GTK_BOX(vbox),terminal,TRUE,TRUE,0);

#if defined(DATADIR) && GTK == 2
    icon = gdk_pixbuf_new_from_file(DATADIR "/icon.jpg", NULL);
    if(icon)
    	gtk_window_set_icon(GTK_WINDOW(top_window),icon);
#endif

	if(arq)
		fclose(arq);

	gtk_widget_grab_focus(terminal);
	gtk_widget_grab_default(terminal);
	gtk_window_set_position(GTK_WINDOW(top_window),GTK_WIN_POS_CENTER);

 }

#ifdef USE_GNOME
static gint save_session (GnomeClient *client, gint phase, GnomeSaveStyle save_style,
              gint is_shutdown, GnomeInteractStyle interact_style,
              gint is_fast, gpointer client_data)
{
  gchar** argv;
  guint argc;

  Log("Saving session for %s",(char *) client_data);

  /* allocate 0-filled, so it will be NULL-terminated */
  argv = g_malloc0(sizeof(gchar*)*4);
  argc = 0;

  argv[argc++] = client_data;

  if(cl_hostname)
  {
  	argv[argc++] = ((gchar *) cl_hostname);
  }

  gnome_client_set_clone_command(client, argc, argv);
  gnome_client_set_restart_command(client, argc, argv);

  return TRUE;
}

static void session_die(GnomeClient* client, gpointer client_data)
{
	Log("Exiting by gnome's request");
	action_exit(0,0);
}
#endif

 int main(int argc, char **argv)
 {

    printf(PROJECT_NAME " Build " BUILD "\n");
    fflush(stdout);

    Log(PROJECT_NAME " Build " BUILD );

    /* Populate callback tables */
    CHKPoint();
	set_3270_screen(&g3270_screen_callbacks);

	// FIXME (perry#9#): Looks like this is not necessary since the library doesn't process keyboard.
    CHKPoint();
    set_3270_keyboard(&g3270_keyboard_info);

    g_thread_init(NULL);
    gdk_threads_init();
    gtk_init(&argc, &argv);

    MainThread = g_thread_self();
    DBGPrintf("Main thread: %p",MainThread);

#ifdef USE_GNOME
	client = gnome_master_client();
	gtk_signal_connect(GTK_OBJECT (client), "save_yourself", GTK_SIGNAL_FUNC(save_session), argv[0]);
	gtk_signal_connect(GTK_OBJECT (client), "die", GTK_SIGNAL_FUNC(session_die), NULL);
#endif

    /* Parse 3270 command line */
    parse_3270_command_line(argc, (const char **) argv, &cl_hostname);

    if(!cl_hostname)
       cl_hostname = getenv("HOST3270_0");

    DBGMessage(cl_hostname);
    CreateMainWindow(cl_hostname);

	if(terminal)
	{
#ifdef EXTENSIONS
		SetExtensionsChar("g3270ServerChanged",cl_hostname);
#endif

		DBGMessage("Starting gtk main loop");
		gtk_widget_show_all(top_window);
		CHKPoint();
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
       strncpy(title,PROJECT_NAME,511);
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

