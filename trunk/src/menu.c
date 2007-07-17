
 #include "g3270.h"
 #include <dlfcn.h>
 #include "lib/togglesc.h"

/*---[ Parameter structure ]--------------------------------------------------*/

 typedef struct _parmdata
 {
 	GtkItemFactoryEntry entry;
 	unsigned char		toggle;
 	char 				parm[1024];
 } PARMDATA;

/*---[ Implement ]------------------------------------------------------------*/

 static void Accelerator(PARMDATA *data, const char *accelerator)
 {
    data->entry.accelerator = g_strdup(accelerator);
 }

 static void Action(PARMDATA *data, const char *name)
 {
    int f;

	/* Locate internal actions */
    for(f=0;f< action_callback_counter;f++)
    {
    	if(!strcmp(action_callbacks[f].name,name))
    	{
    		data->entry.callback = action_callbacks[f].callback;
    		return;
    	}
    }

	/* Locate extension actions */
    data->entry.callback = QueryActionCallback(name);
	if(data->entry.callback)
		return;

	/* Invalid action code, release data and log-it */
    if(data->entry.path)
    {
    	g_free(data->entry.path);
    	data->entry.path = 0;
    }

    Log("Invalid action code: \"%s\"",name);
 }

 static void Type(PARMDATA *data, const char *type)
 {
 	char key[1024];

 	if(strcmp(type,"Toggle"))
 	{
 		snprintf(key,1023,"<%s>",type);
 	}
 	else
 	{
 		data->toggle = 1;
 		strcpy(key,"<ToggleItem>");
 	}

    data->entry.item_type = g_strdup(key);

    DBGPrintf("Type: %s item_type: %s",type,key);

 }

 static void Parm(PARMDATA *data, const char *parm)
 {
 	strncpy(data->parm,parm,1023);
 }

 static void ReleaseString(gpointer string)
 {
 	if(string)
 	{
 		DBGPrintf("Releasing \"%s\"",(char *) string);
 		g_free(string);
 	}
 }

 static void InsertItem(GtkItemFactory *factory, PARMDATA *item, int *qtd)
 {
    if(item->entry.path)
	{
	   if(*item->parm)
	   {
	      item->entry.callback_action = (guint) g_strdup((char *) item->parm);
	   }
	   else
	   {
          item->entry.callback_action = 0;
	   }

	   if(!item->entry.callback && item->toggle)
			Action(item,"toggle");

	   DBGPrintf("%p %s %p %s",(void *) item->entry.callback_action,item->entry.path,item->entry.callback,item->toggle ? "Toggle" : "");

       gtk_item_factory_create_item(	factory,
										(GtkItemFactoryEntry *) &item->entry,
										0,
										1 );

       if(item->entry.callback_action)
       {
	      GObject *obj = (GObject *) gtk_item_factory_get_widget_by_action(factory,item->entry.callback_action);

	      if(!obj)
	      {
		     Log("Error locating widget for \"%s\"",item->entry.path);
	      }
	      else
	      {
	      	g_object_set_data_full(obj,"g3270.parameter",(gpointer) item->entry.callback_action,ReleaseString);
	      }

#ifdef __G_KEY_FILE_H__
		  if(item->toggle)
		  {
		  	 // Load toggle configuration
		     int toggle = ToggleByName(item->parm);
			 if(toggle >= 0 && main_configuration)
			 {
				if(g_key_file_has_key(main_configuration,"Toggles",item->parm,NULL))
				{
					gboolean flag = g_key_file_get_boolean(main_configuration,"Toggles",item->parm,NULL);

					DBGPrintf("%s: %d",item->parm,flag);

				  	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(obj),flag);
					set_toggle(toggle,flag,TT_INITIAL);
				}
				else if(toggled(toggle))
				{
					gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(obj),TRUE);
					set_toggle(toggle,1,TT_INITIAL);
			 		g_key_file_set_boolean(main_configuration,"Toggles",item->parm,TRUE);
				}
			 }

		  }
#endif
       }

       (*qtd)++;

       g_free(item->entry.path);
	}
    memset(item,0,sizeof(PARMDATA));
 }

 GtkWidget *LoadMenu(GtkWidget *window)
 {
#ifdef DATADIR
    static const char		*filename = DATADIR "/menu.conf";
#else
    static const char		*filename = "./menu.conf";
#endif

	 static const struct _cmd
 	{
 		const char *key;
 		void (*exec)(PARMDATA *, const char *);
 	} cmd[] =
 	{
 		{ "Accelerator",	Accelerator },
 		{ "Action",			Action		},
 		{ "Type",			Type		},
 		{ "Script",			Parm		},
 		{ "Command",		Parm		},
 		{ "Option",			Parm		}
 	};

 	unsigned char		buffer[1024];

 	FILE 				*arq	= fopen(filename,"r");
    int  				qtd		= 0;
    unsigned char		*ptr;
    unsigned char		*ln;
    int					f;
    PARMDATA			item;

    // http://www.gtk.org/tutorial1.2/gtk_tut-13.html
    GtkAccelGroup		*accel_group	= gtk_accel_group_new();

    // http://developer.gnome.org/doc/API/2.0/gtk/GtkItemFactory.html
    GtkItemFactory		*factory		= gtk_item_factory_new(GTK_TYPE_MENU_BAR, "<main>", accel_group);

    DBGMessage(filename);

    memset(&item,0,sizeof(item));

    while(fgets((char *) buffer,1023,arq))
    {
   	    for(ln=buffer;*ln && *ln <= ' ';ln++);

    	for(ptr=ln;*ptr && *ptr >= ' ';ptr++);
    	*ptr = 0;

    	if(*buffer == '/')
    	{
		   ptr = buffer;
		   if(!strncmp((char *) ptr,"/menu",5))
			   ptr += 5;

           InsertItem(factory, &item, &qtd);
		   item.entry.path = g_strdup((char *) ptr);
    	}
    	else if(item.entry.path && *ln && *ln != '#')
    	{
		   ptr = (unsigned char *) strchr((char *) ln,'=');
		   if(ptr)
		   {
		      *ptr++ = 0;
		      for(f=0;f< (sizeof(cmd)/sizeof(struct _cmd));f++)
		      {
			     if(!strcmp(cmd[f].key,(char *) ln))
			        cmd[f].exec(&item,(char *) ptr);
		      }
		   }
    	}
    }

	if(item.entry.path)
       InsertItem(factory, &item, &qtd);

    fclose(arq);

	if(qtd)
	{
       /* Attach the new accelerator group to the window. */
       gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);

	   /* Finally, return the actual menu bar created by the item factory. */
       return gtk_item_factory_get_widget(factory, "<main>");
	}

	return 0;

 }
