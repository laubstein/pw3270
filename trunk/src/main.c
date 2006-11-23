
 #include "g3270.h"
 #include "lib/hostc.h"

/*---[ Prototipes ]-----------------------------------------------------------*/


/*---[ Main menu ]------------------------------------------------------------*/


/*---[ Globals ]--------------------------------------------------------------*/

 GtkWidget	*top_window		= 0;
 GtkWidget  *terminal		= 0;
 GtkWidget  *top_menu		= 0;

#if defined(DATADIR) && GTK == 2
 GdkPixbuf	*icon			= 0;
#endif

/*---[ Main program ]---------------------------------------------------------*/

 static gboolean delete_event( GtkWidget *widget, GdkEvent  *event, gpointer data )
 {
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

/*

 Why it's not working for some keys?

 static void LoadAcelerators(GtkWidget  *window)
 {
   // http://developer.gnome.org/doc/API/2.0/gtk/gtk-Keyboard-Accelerators.html

   static const struct _accelerator
   {
      const char *key;
      void (*action)(GtkWidget *w, gpointer data);
   } accelerator[] =
   {
		{ "Page_Up",			action_F7 		},
		{ "Page_Down",			action_F8 		},

     	{ "Print",				action_print	},
     	{ "3270_PrintScreen",	action_print	},

		{ "Tab",				action_Tab 		},
     	{ "ISO_Left_Tab",		action_BackTab	}

   };

   int				 f;

   GtkAccelGroup	*accel_group;
   GdkModifierType 	 mods;
   guint 			 key;

   // http://developer.gnome.org/doc/API/2.0/gobject/gobject-Closures.html
   GClosure			*closure;

   accel_group = gtk_accel_group_new();

   for(f=0;f< (sizeof(accelerator)/sizeof(struct _accelerator)); f++)
   {
	  DBGMessage(accelerator[f].key);
      gtk_accelerator_parse(accelerator[f].key, &key, &mods);
      if(key)
      {
         closure = g_cclosure_new_object_swap(G_CALLBACK(accelerator[f].action), G_OBJECT(window));
         gtk_accel_group_connect(accel_group,key,mods,GTK_ACCEL_LOCKED,closure);
      }
      else
      {
	     Log("Error parsing accelerator \"%s\"",accelerator[f].key);
      }
   }

   gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

 }
*/

 static GtkWidget *CreateMainMenu(GtkWidget  *window)
 {
   // http://www.gtk.org/tutorial1.2/gtk_tut-13.html
   GtkItemFactory	*item_factory;
   GtkAccelGroup	*accel_group;

   accel_group = gtk_accel_group_new();

   // http://developer.gnome.org/doc/API/2.0/gtk/GtkItemFactory.html
   item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);

   /* Load menu from configuration file */
#ifdef DATADIR
   LoadMenu(DATADIR "/menu.conf",item_factory);
#else
   LoadMenu("./menu.conf",item_factory);
#endif

   /* Attach the new accelerator group to the window. */
   gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

   /* Attach keyboard actions to the window */
//   LoadAcelerators(window);

   /* Finally, return the actual menu bar created by the item factory. */
   return gtk_item_factory_get_widget(item_factory, "<main>");
 }

 static void CreateMainWindow(const char *cl_hostname)
 {
 	GtkWidget *vbox;

    top_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

#if GTK == 1
	gtk_window_set_wmclass(GTK_WINDOW(top_window),"toplevel",TARGET);
	#error Is it working?
#else
    gtk_window_set_role(GTK_WINDOW(top_window), TARGET "0" );
#endif

	g_signal_connect(G_OBJECT(top_window),	"delete_event", G_CALLBACK(delete_event),	NULL);
    g_signal_connect(G_OBJECT(top_window),	"destroy", 		G_CALLBACK(destroy),		NULL);
    g_signal_connect(G_OBJECT(top_window),	"map-event",	G_CALLBACK(map_event),		NULL);

    UpdateWindowTitle();

    vbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(top_window),vbox);

    top_menu = CreateMainMenu(top_window);
    gtk_box_pack_start(GTK_BOX(vbox), top_menu, FALSE, TRUE, 0);

    // Create terminal window
	terminal = g3270_new(cl_hostname);
	gtk_box_pack_start(GTK_BOX(vbox),terminal,TRUE,TRUE,0);

#if defined(DATADIR) && GTK == 2
    icon = gdk_pixbuf_new_from_file(DATADIR "/icon.jpg", NULL);
    if(icon)
    	gtk_window_set_icon(GTK_WINDOW(top_window),icon);
#endif

	gtk_window_set_position(GTK_WINDOW(top_window),GTK_WIN_POS_CENTER);

 }

 int main(int argc, char **argv)
 {

    printf(TARGET " Build " BUILD " for gtk " GTKVERSION "\n");
    fflush(stdout);

    Log(TARGET " Build " BUILD " for gtk " GTKVERSION "\n");

    /* Populate callback tables */
    set_3270_screen(&g3270_screen_callbacks);
    set_3270_keyboard(&g3270_keyboard_info);

    g_thread_init(NULL);
    gdk_threads_init();
    gtk_init(&argc, &argv);

    /* Parse 3270 command line */
    parse_3270_command_line(argc, (const char **) argv, &cl_hostname);

    if(!cl_hostname)
       cl_hostname = getenv("HOST3270_0");

    DBGMessage(cl_hostname);

    CreateMainWindow(cl_hostname);

    DBGMessage("Starting gtk main loop");

    gtk_widget_show_all(top_window);
    gtk_main();

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



