
 #include "g3270.h"
 #include "lib/hostc.h"

/*---[ Prototipes ]-----------------------------------------------------------*/


/*---[ Main menu ]------------------------------------------------------------*/


 // TODO (perry#1#): Load it from configuration file.
 static GtkItemFactoryEntry menu_items[] =
 {
 	{ "/_Arquivo",					NULL,					NULL,						0,		"<Branch>"		},
 	{ "/Arquivo/_Imprimir Tela",	"<control>P",			action_print,				0,		NULL			},
 	{ "/Arquivo/Imprimir Seleção",	"<control><shift>P",	action_print_selection,		0,		NULL			},
 	{ "/Arquivo/Imprimir Cópia",	NULL,					action_print_copy,			0,		NULL			},
 	{ "/Arquivo/Sair",          	"<control>X",			action_exit,				0,		NULL			},

 	{ "/_Editar",					NULL,					NULL,						0,		"<Branch>"		},
 	{ "/Editar/Copiar",				"<control>C",			action_copy,				0,		NULL			},
 	{ "/Editar/Copiar anexando",	"<control><shift>C",	action_append,				0,		NULL			},
 	{ "/Editar/Colar",				"<control>V",			action_paste,				0,		NULL			},
	{ "/Editar/sep1",     			NULL,         			NULL,						0,		"<Separator>"	},
	{ "/Editar/Limpar campos",		NULL,					action_clear,				0,		NULL			},
	{ "/Editar/sep2",     			NULL,         			NULL,						0,		"<Separator>"	},
	{ "/Editar/Selecionar tudo",	"<control>A",			action_select_all,			0,		NULL			},
	{ "/Editar/Desmarcar",			"<control>D",			action_remove_selection,	0,		NULL			},

 	{ "/_Opções",					NULL,					NULL,						0,		"<Branch>"		},
 	{ "/Opções/Cross Hair",			"<ALT>x",				action_crosshair,			0,		NULL			},

 	{ "/Comunicação/Conectar",		NULL,					action_connect,				0,		NULL			},
 	{ "/Comunicação/Desconectar",	NULL,					action_disconnect,			0,		NULL			},

 };

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

 static GtkWidget *CreateMainMenu(GtkWidget  *window)
 {
   // http://www.gtk.org/tutorial1.2/gtk_tut-13.html
   GtkItemFactory	*item_factory;
   GtkAccelGroup	*accel_group;
   gint 			nmenu_items		= sizeof (menu_items) / sizeof (menu_items[0]);

   accel_group = gtk_accel_group_new();

   /* This function initializes the item factory.
      Param 1: The type of menu - can be GTK_TYPE_MENU_BAR, GTK_TYPE_MENU,
               or GTK_TYPE_OPTION_MENU.
      Param 2: The path of the menu.
      Param 3: A pointer to a gtk_accel_group.  The item factory sets up
               the accelerator table while generating menus.
   */

   item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);

   /* This function generates the menu items. Pass the item factory,
      the number of items in the array, the array itself, and any
      callback data for the the menu items. */
   gtk_item_factory_create_items(item_factory, nmenu_items, menu_items, NULL);

   /* Attach the new accelerator group to the window. */
   gtk_window_add_accel_group(GTK_WINDOW (window), accel_group);

   /* Finally, return the actual menu bar created by the item factory. */
   return gtk_item_factory_get_widget(item_factory, "<main>");
 }

 static void CreateMainWindow(const char *cl_hostname)
 {
 	GtkWidget *vbox;

    top_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	g_signal_connect(G_OBJECT(top_window),	"delete_event", G_CALLBACK(delete_event),	NULL);
    g_signal_connect(G_OBJECT(top_window),	"destroy", 		G_CALLBACK(destroy),		NULL);
    g_signal_connect(G_OBJECT(top_window),	"map-event",	G_CALLBACK(map_event),		NULL);

    SetWindowTitle(0);

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

#if GTK == 1
	gtk_window_set_wmclass(GTK_WINDOW(top_window),"toplevel",TARGET);
#else
    gtk_window_set_role(GTK_WINDOW(top_window), TARGET "_topwindow");
#endif

    gtk_widget_show(top_menu);
    gtk_widget_show(terminal);
    gtk_widget_show(vbox);

 }

 int main(int argc, char **argv)
 {
/*
#ifdef DEBUG
	int		f;
    gchar	str[2];
    gchar   *string;

    CHKPoint();

    str[1] = 0;
    for(f=1;f<0xFF;f++)
    {
    	*str   = f; // Ebc2ASC(f);
		string = g_convert(str, 1, "EBCDIC-BR", "UTF-8", NULL, NULL, NULL);

		if(string)
		   fprintf(stderr,"0x%02x\n",(unsigned char) *string);
		else
		   fprintf(stderr,"0x%02x\n",(unsigned char) ' ');

        g_free(string);
    }


    return -1;
#endif
*/

    printf(TARGET " Build " BUILD " for gtk " GTKVERSION "\n");
    fflush(stdout);


    /* Populate callback tables */
    set_3270_screen(&g3270_screen_callbacks);
    set_3270_keyboard(&g3270_keyboard_info);

    g_thread_init(NULL);
    gtk_init(&argc, &argv);

    /* Parse 3270 command line */
    parse_3270_command_line(argc, (const char **) argv, &cl_hostname);

    if(!cl_hostname)
       cl_hostname = getenv("HOST3270");

    CreateMainWindow(cl_hostname);

    DBGMessage("Starting gtk main loop");

    gtk_widget_show(top_window);
    gtk_main();

    return 0;
 }

 void SetWindowTitle(const char *msg)
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

       if(msg)
	      strncat(title,msg,511);

       gtk_window_set_title(GTK_WINDOW(top_window),title);

    }
 }



