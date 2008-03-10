
 #include "g3270.h"

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

/*---[ Statics ]--------------------------------------------------------------*/

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

/*---[ Globals ]--------------------------------------------------------------*/

#ifdef __G_KEY_FILE_H__
 GKeyFile	*main_configuration	= 0;
#endif

/*---[ Implement ]------------------------------------------------------------*/

 void action_save(GtkWidget *w, gpointer data)
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
 }

 void action_restore(GtkWidget *w, gpointer data)
 {
 	char 		filename[4096];
 	char 		*home	= getenv("HOME");

#ifdef __G_KEY_FILE_H__
	gint		*pos;
	gsize		sz;
	int			f;
#else
 	struct user_config config;
#endif


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
 }

